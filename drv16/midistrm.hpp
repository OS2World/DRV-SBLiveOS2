/* $Id: midistrm.hpp,v 1.1 2000/04/23 14:55:18 ktk Exp $ */

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
 * Defines, class definations and prototypes for the MIDISTREAM class
 * @version %I%
 * @context Unless otherwise noted, all interfaces are Ring-0, 16-bit,
 *  kernel stack.
 * @history
 */
#ifndef MIDISTREAM_INCLUDED
#define MIDISTREAM_INCLUDED

#ifndef OS2_INCLUDED
#define INCL_NOPMAPI
#include <os2.h>
#endif

#ifndef OS2ME_INCLUDED
#include <os2me.h>                     // prereq to midistrm.hpp
#endif

#ifndef DDCMD_REG_STREAM         // shdd.h can't handle being included twice
#include <shdd.h>                // for PDDCMDREGISTER
#endif

#include "stream.hpp"                  // Object definition.
#include "midimsg.hpp"                 // Object definition.

// Number of notes in the MIDI definition.
const NUM_MidiNotes = 128;

// Number of channels in the MIDI definition.
const NUM_MidiChannels = 16;

// The MIDI parser is always in one of the following states.
enum PARSER_STATE
{
   S_Init,                             // Initial state.
   S_ChannelMsg,                       // Channel message in progress.
   S_SystemCommon,                     // System Common message in progress.
   S_SysexSignature,                   // System Exclusive signature (1st 4 bytes).
   S_SysexIBM,                         // IBM Sysex in progress.
   S_SysexNotIBM                       // Non-IBM Sysex in progress.
};


class MIDISTREAM : public STREAM {

public:
   MIDISTREAM( ULONG streamtype, USHORT filesysnum);
   virtual ~MIDISTREAM() {};
   void Process ( void );        // Called by Timer when it's time to do work.
                                 // Process() schedules its next invocation.
   virtual ULONG GetCurrentTime(void);
   virtual void  SetCurrentTime(ULONG time);
   virtual ULONG Read(PSTREAMBUF, unsigned);
   virtual ULONG StartStream(void);
   virtual ULONG StopStream(PCONTROL_PARM);
   virtual ULONG PauseStream(PCONTROL_PARM);
   virtual ULONG ResumeStream(void);

private:
// Members that deal with timing
   ULONG ulPerClock;             // microseconds per 0xF8 timing pulse
   ULONG _ulLastProcess;         // Time on clock when we last processed MIDI messages.
   long lWait;                   // microseconds to wait until it's time to
                                 // process the next message
   ULONG ulTempo;                // 1/10 beats per minute (beats per 10 minutes)
   USHORT usCPQN;                // last PPQN scalar received from MMPM; is
                                 // _not_ the current PPQN value.  used by
                                 // CalcDelay() to compute ulPerClock.
   void CalcDelay(void);         // calculates ulPerClock (per 0xF8 pulse)

// Members that deal with processing the stream data
   static USHORT _usBitNumber[]; // Used to map between integers & bit numbers.
   USHORT  _notesOn[ NUM_MidiNotes ];
                                 // 128 x 16 bit matrix, each bit represents one
                                 // note on one channel.  Records whether each note
                                 // is currently on or off.  Used for "all notes off".

   PARSER_STATE state;           // Current state of MIDI parser, as enumerated above.
   MIDIMSG message;              // Current message that we're assembling from the input stream.

   void _allNotesOff( void );     // Shut off all notes that are currently playing.
   void parse( MIDIBYTE bInput ); // Parse the next byte of the input stream.
   void dispatch( MIDIMSG& msg ); // Interpret the complete message.
};

#endif
