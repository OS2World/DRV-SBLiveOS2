/* $Id: mpu401.cpp,v 1.3 2000/05/28 16:50:40 sandervl Exp $ */

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
 *  MPU_401 object implementation.
 * @version %I%
 * @context
 *  Unless otherwise noted, all interfaces are Ring-0, 16-bit, kernel stack.
 * @notes
 * @history
 */

extern "C" {               // 16-bit header files are not C++ aware
#define INCL_NOPMAPI
#define INCL_DOSMISC
#include <os2.h>
}
#include <os2medef.h>                  // DATATYPE_MIDI
#include <include.h>                   // cli, sti, inp, outp
#include <string.h>                    // strcpy(), strcat()
#include "sizedefs.h"                  //### NUM_DEVICES, MAX_MPU401
#include "..\midi\midi_idc.h"          // RTMIDI i/f
#include "iodelay.h"
#include "malloc.h"
#include "mpu401.hpp"                  // Object definition.
#include "timer.hpp"                   // Object definition.
#include "stream.hpp"                  // Prereq to includeing midistrm.h
#include "midistrm.hpp"                // Object definition.

#include "parse.h"
#include "ossidc.h"
#include <dbgos2.h>

#define TIMEOUT   60000


/* Constructor. */

MPU_401::MPU_401(TIMER* pTimer ) :
   MIDIAUDIO ( AUDIOHW_MPU401_PLAY, pTimer )
{
   static char szName[] = "SBLive RTMIDI #";  // Instance name for RTMIDI.  A number will be appended.
   static char szSuffix[] = "0";       // Printable char that is appended to szName.

   // RTMIDI (MIDI.SYS) related stuff
////   ++szSuffix[0];                      // Bump number in instance name.
   strcpy( szRTMIDI_Name, szName );    // Setup instance name.
   strcat( szRTMIDI_Name, szSuffix );  // Append ASCII number to instance name.
   ulRTMIDI_Caps = MIDICAPSA_INPUT | MIDICAPSA_OUTPUT;    // Set RTMIDI caps.

   midiOutStreamId = 0;
   midiInStreamId  = 0;
}

virtual void MPU_401::noteOff( BYTE mchan, BYTE note, BYTE velocity )
{
   writeByte( (BYTE) 0x80 | mchan );
   writeByte( note );
   writeByte( velocity );
}

virtual void MPU_401::noteOn( BYTE mchan, BYTE note, BYTE velocity )
{
   writeByte( (BYTE) 0x90 | mchan );
   writeByte( note );
   writeByte( velocity );
}

virtual void MPU_401::polyphonicPressure( BYTE mchan, BYTE note, BYTE value )
{
   writeByte( (BYTE) 0xA0 | mchan );
   writeByte( note );
   writeByte( value );
}

virtual void MPU_401::controlChange( BYTE mchan, BYTE control_number, BYTE value )
{
   writeByte( (BYTE) 0xB0 | mchan );
   writeByte( control_number );
   writeByte( value );
}

virtual void MPU_401::programChange( BYTE mchan, BYTE program_number )
{
   writeByte( (BYTE) 0xC0 | mchan );
   writeByte( program_number );
}

virtual void MPU_401::channelPressure( BYTE mchan, BYTE value )
{
   writeByte( (BYTE) 0xD0 | mchan );
   writeByte( value );
}

virtual void MPU_401::pitchBend( BYTE mchan, BYTE value_lsb, BYTE value_msb )
{
   writeByte( (BYTE) 0xE0 | mchan );
   writeByte( value_lsb );
   writeByte( value_msb );
}


int MPU_401::_iAllNotesOff(void)
{
   for (int iChannel=0; iChannel<16; iChannel++) {
      writeByte((BYTE) (0xB0 + iChannel));    // channel mode
      writeByte(123);                         // all notes off
      writeByte(0);
   }

   return TRUE;
}


int MPU_401::writeByte(BYTE b)
{
   if(!midiOutStreamId) {
	DebugInt3();
	return 0;
   }
   unsigned int i;

   for (i=0; i<TIMEOUT; i++) {
      	if(OSS16_WriteMidiByte(midiOutStreamId, b)) {
		return 1;
      	}
      	iodelay(1);
   }
   return 0;
}

int MPU_401::readByte(void)
{
   //TODO:
   return -1;
}

void MPU_401::processIrq(unsigned long streamid)
{
 char buffer[64];
 int  bufsize;

   MIDIAUDIO* pma = (MIDIAUDIO *) pAudioHWList->Head();
   while (pma) {
      if((pma->ulDeviceType == AUDIOHW_MPU401_CAPTURE) ||
        (pma->ulDeviceType == AUDIOHW_MPU401_PLAY)) {
		break;
      }
      pma = (MIDIAUDIO *) pma->pNext;
   }
   if(pma == NULL) {
	dprintf(("MPU_401::processIrq: mpu device found!!"));
	return;
   }

   while(TRUE) {
	bufsize = OSS16_ReadMidiBytes(streamid, &buffer[0], sizeof(buffer));
	for(int i=0;i<bufsize;i++) {
		pma->pfnSendByte(pma->ulRTMIDI_Handle, buffer[i]);
	}
	if(bufsize != sizeof(buffer)) {
		break;
	}
   }   
}

#pragma off (unreferenced)

int MPU_401::Reset(STREAM *stream)
{
    return 0;
}

int MPU_401::Start( STREAM *stream )
{
   BOOL rc;

   // Start timer on 4 mSec interval.
   rc = getTimer()->Start();
   return rc;
}


int MPU_401::Stop(STREAM *stream)
{
   getTimer()->Stop();
   _iAllNotesOff();
   return 0;
}


int MPU_401::Pause(STREAM *stream)
{
   getTimer()->Pause();
   _iAllNotesOff();
   return 0;
}


int MPU_401::Resume(STREAM *stream)
{
   getTimer()->Resume();
   return 0;
}

USHORT MPU_401::RTMIDI_OpenReceive(void)
{
   if(midiOutStreamId == 0) {
   	  midiOutStreamId = OSS16_OpenMidiStream(MIDI_RECEIVE);
   }
   return (midiOutStreamId) ? 0 : MIDIERRA_HW_FAILED;
}

USHORT MPU_401::RTMIDI_OpenSend(void)
{
   if(midiInStreamId == 0) {
   	  midiInStreamId = OSS16_OpenMidiStream(MIDI_SEND);
   }
   return (midiInStreamId) ? 0 : MIDIERRA_HW_FAILED;
}

USHORT MPU_401::RTMIDI_CloseReceive(void)
{
   if(midiOutStreamId) {
   	OSS16_CloseMidiStream(MIDI_RECEIVE, midiOutStreamId);
	midiOutStreamId = 0;
   }
   return 0;
}

USHORT MPU_401::RTMIDI_CloseSend(void)
{
   if(midiInStreamId) {
   	OSS16_CloseMidiStream(MIDI_SEND, midiInStreamId);
	midiInStreamId = 0;
   }
   return 0;
}

