/* $Id: midistrm.cpp,v 1.2 2000/04/24 19:45:17 sandervl Exp $ */

/* SCCSID = %W% %E% */
/****************************************************************************
 *                                                                          *
 * Copyright (c) IBM Corporation 1994 - 1997.                               *
 *                                                                          *
 * The following IBM OS/2 source code is provided to you solely for the     *
 * the purpose of assisting you in your development of OS/2 device drivers. *
 * You may use this code in accordance with the IBM License Agreement       *
 * provided in the IBM Device Driver Source Kit for OS/2.                   *
 *                                                                          *
 ****************************************************************************/
/**@internal %W%
 * @notes
 *  MIDISTREAM class implementation.  The Midi Stream class is derived
 *  from the Stream class.
 * @version %I%
 * @context Unless otherwise noted, all interfaces are Ring-0, 16-bit,
 *  kernel stack.
 * @history
 *
 */
#define INCL_NOPMAPI
#define INCL_DOSERRORS            // for ERROR_INVALID_FUNCTION
#include <os2.h>
#include <os2me.h>
#include <audio.h>                // for #define MIDI

#include <include.h>
#include <devhelp.h>

#include "midistrm.hpp"
#include "event.hpp"
#include "maudio.hpp"

// An array to map integer 0..15 to the corresponding bit number in a USHORT.
USHORT MIDISTREAM::_usBitNumber[ NUM_MidiChannels ] =
  { 0x0001, 0x0002, 0x0004, 0x0008,
    0x0010, 0x0020, 0x0040, 0x0080,
    0x0100, 0x0200, 0x0400, 0x0800,
    0x1000, 0x2000, 0x4000, 0x8000 };


/**@internal CalcDelay
 * @param    None
 * @return   None
 * @notes
 *           600,000,000 microseconds/10 minutes
 *----------------------------------------------------------   ==   X microseconds/clock
 *                                    usCPQNnum
 *  (ulTempo beats/10 min) * ( 24 * ------------- clocks/beat )
 *                                    usCPQNden
 *
 *
 *      25,000,000 * usCPQNden
 *==  --------------------------
 *       ulTempo * usCPQNnum
 *
 * where
 * usCPQNden = ((usCPQN & 0x3F) + 1) * 3
 * usCPQNnum = 1                                    if bit 6 of usCPQN is set
 *
 *    or
 *
 * usCPQNden = 1
 * usCPQNnum = usCPQN + 1                           if bit 6 is not set
 */
void MIDISTREAM::CalcDelay(void)
{
   ULONG ul;

   if (usCPQN & 0x40) {          // bit 6 is set if it's a denominator
      ul = 25000000 * ((usCPQN & 0x3F) + 1);
      ulPerClock = ul / ulTempo;
      ulPerClock *= 3;
   } else {
      ul = ulTempo * (usCPQN+1);
      ulPerClock = 25000000 / ul;
   }
}


/**@external MIDISTRM::Process
 *  Consume MIDI bytes from the MMPM/2 stream buffers and send
 *  them off to the MIDI parser to be interpreted.
 * @param void
 * @return void
 * @notes Runs at Task time on a global context hook;  does not run
 *  on an interrupt level.  Interacts with the Timer object defined
 *  for this stream to obtain current time and to request next Stream
 *  time to be scheduled.
 */
void MIDISTREAM::Process( void )
{
   ULONG ulNewTime;                    // Time, in mSec, on entry.
   ULONG ulElapsedTime;                // Elapsed time, last tick to this one.

   // Update time variables.
   ulNewTime = ((MIDIAUDIO*) pahw)->getTimer()->ulGetTime();
   if ( ulNewTime > _ulLastProcess )
      ulElapsedTime = ulNewTime - _ulLastProcess;
   else
      ulElapsedTime = 0;

   _ulLastProcess = ulNewTime;
   ulCurrentTime = ulNewTime;

   if (qhInProcess.IsElements() == 0)   // no buffers to process?
      return;

   if (ulStreamState == STREAM_PAUSED)    // is the stream paused?
      return;

   ProcessEvents();

   lWait -= ulElapsedTime * 1000;

   PSTREAMBUFFER pstreambuff = (PSTREAMBUFFER) qhInProcess.Head();
   PSTREAMBUF pbuff = pstreambuff->pBuffptr;
   ULONG buffsz = pstreambuff->ulBuffsz;

   while (lWait <= 0 && qhInProcess.IsElements()) {
      parse(*(pbuff + (pstreambuff->ulBuffpos)++));

      if (pstreambuff->ulBuffpos >= buffsz) {
         qhDone.PushOnTail(qhInProcess.PopHead());
         pstreambuff = (PSTREAMBUFFER) qhInProcess.Head();
         pbuff = pstreambuff->pBuffptr;
         buffsz = pstreambuff->ulBuffsz;
      }
   }
   while (qhDone.IsElements())
      ReturnBuffer();

   // Determine next time to run.  If we submit a time that has already
   // passed, we'll get scheduled for the next tick.

   ULONG ulTimeNextRun = ulCurrentTime + (lWait / 1000);
   ((MIDIAUDIO*) pahw)->getTimer()->vSchedule( ulTimeNextRun );
}


// ### By getting Timer time, we're assuming that this stream owns the HW.
ULONG MIDISTREAM::GetCurrentTime(void)
{
   return ((MIDIAUDIO*) pahw)->getTimer()->ulGetTime() ;
}

// ### By setting Timer time, we're assuming that this stream owns the HW.
void  MIDISTREAM::SetCurrentTime(ULONG time)
{
   ((MIDIAUDIO*) pahw)->getTimer()->vSetTime( time );
   ulCurrentTime = time;
}

ULONG MIDISTREAM::Read(PSTREAMBUF, unsigned)
{
   return(ERROR_INVALID_FUNCTION);
}

ULONG  MIDISTREAM::StartStream(void)
{

   if (!pahw->Start(this))
      return ERROR_START_STREAM;
   state = S_Init;                        // Reset parser state.
   message.clear();                       // Clear current message.
   lWait = 0;	//SvL, reset this too
   ulStreamState = STREAM_STREAMING;
   return NO_ERROR;
}


/**@internal MIDISTREAM::_allNotesOff
 *  Shut off all notes that are currently playing.
 * @param None.
 * @return void
 * @notes This function walks the _notesOn array and shuts off any note
 *  that is flagged as being actively played.
 */
void MIDISTREAM::_allNotesOff( void )
{
   for ( USHORT noteNum=0; noteNum < NUM_MidiNotes; ++noteNum)
      if (_notesOn[noteNum])
         // This note number is playing on one or more channels.
         // Shut the note off on all channels on which it is playing.
         for ( USHORT mchan=0; mchan < NUM_MidiChannels; ++mchan)
            if (_notesOn[noteNum] & _usBitNumber[mchan]) {
               ((MIDIAUDIO*) pahw)->noteOff( mchan, noteNum, 0 );
               _notesOn[noteNum] &= ~(_usBitNumber[mchan]);
            }
}


ULONG  MIDISTREAM::StopStream(PCONTROL_PARM pControl)
{

   ulStreamState = STREAM_STOPPED;
   pahw->Stop(this);
   _allNotesOff();
   ReturnBuffers();
   pControl->ulTime = GetCurrentTime();
   return NO_ERROR;
}


ULONG  MIDISTREAM::PauseStream(PCONTROL_PARM pControl)
{

   if (ulStreamState == STREAM_PAUSED)    // is the stream paused?
      return ERROR_INVALID_SEQUENCE;
   pahw->Pause(this);
   _allNotesOff();
   pControl->ulTime = GetCurrentTime();
   ulStreamState = STREAM_PAUSED;
   return NO_ERROR;

}

ULONG  MIDISTREAM::ResumeStream(void)
{

   if (ulStreamState != STREAM_PAUSED)    // is the stream paused?
      return ERROR_INVALID_SEQUENCE;
   state = S_Init;                        // Reset parser state.
   message.clear();                       // Clear current message.
   pahw->Resume(this);
   ulStreamState = STREAM_STREAMING;
   return NO_ERROR;

}

MIDISTREAM::MIDISTREAM(ULONG streamtype, USHORT filesysnum):
   STREAM(streamtype, filesysnum)
{
   // Initialize tempo & scheduling information.
   ulTempo = 1200;
   usCPQN = 0;
   CalcDelay();
   lWait = 0;
   _ulLastProcess = 0;

   // Reset the parser.
   state = S_Init;
   message.clear();

   // Reset our tracking of which notes are currently on.
   for (int i=0; i<NUM_MidiNotes; ++i)
      _notesOn[ i ] = 0;
}

