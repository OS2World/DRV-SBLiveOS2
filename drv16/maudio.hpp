/* $Id: maudio.hpp,v 1.1 2000/04/23 14:55:17 ktk Exp $ */

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
 *  MIDI audio hardware object definition.
 * @version %I%
 * @context
 *  Unless otherwise noted, all interfaces are Ring-0, 16-bit, kernel stack.
 * @notes
 * @history
 */

#ifndef MIDIAUDIO_INCLUDED
#define MIDIAUDIO_INCLUDED

#ifndef OS2_INCLUDED
#define INCL_NOPMAPI
#include <os2.h>
#endif

#include "..\midi\midi_idc.h"          // RTMIDI interfaces, MIDI_NAME_LENGTH
#include "audiohw.hpp"                 // Object definition.
#include "timer.hpp"                   // Object definition.

class MIDIAUDIO : public AUDIOHW {
protected:
   MIDIAUDIO (ULONG devicetype, TIMER* pTimer );
public:
   // RT MIDI Data
   char szRTMIDI_Name[MIDI_NAME_LENGTH];
                                       // RTMIDI gets a pointer to this string.
   ULONG ulRTMIDI_Caps;                // RTMIDI defined capability bits.
   ULONG ulRTMIDI_Handle;              // Provided to us by RTMIDI (aka MIDI.SYS)

   // Entry points into RTMIDI (MIDI.SYS) provided to us by IDC interface.
   PFNMIDI_SENDBYTE pfnSendByte;
   PFNMIDI_SENDSTRING pfnSendString;
   PFNMIDI_DEREGISTER pfnDeregister;

   // Methods

   // Writes byte to data port.  Returns 0 if failure, 1 on success
   virtual int writeByte(BYTE);

   // Read data port.  Data read is in LSB with MSB==0 on good read.
   // Returns -1 on err.
   virtual int readByte(void);

   // Device caps for MMPM/2 IOCTL interface.
   virtual void DevCaps(PAUDIO_CAPS pCaps);

   // Start and stop hardware operating.
   virtual int Start(STREAM *stream) = 0;
   virtual int Stop(STREAM *stream) = 0;

   // Standard MIDI channel commands.
   virtual void noteOff( BYTE mchan, BYTE note, BYTE velocity ) = 0;
   virtual void noteOn( BYTE mchan, BYTE note, BYTE velocity ) = 0;
   virtual void polyphonicPressure( BYTE mchan, BYTE note, BYTE value ) = 0;
   virtual void controlChange( BYTE mchan, BYTE control_number, BYTE value ) = 0;
   virtual void programChange( BYTE mchan, BYTE program_number ) = 0;
   virtual void channelPressure( BYTE mchan, BYTE value ) = 0;
   virtual void pitchBend( BYTE mchan, BYTE value_lsb, BYTE value_msb ) = 0;

   // Services provided to the RTMIDI interface.
   virtual USHORT RTMIDI_OpenReceive(void);
   virtual USHORT RTMIDI_OpenSend(void);
   virtual USHORT RTMIDI_CloseReceive(void);
   virtual USHORT RTMIDI_CloseSend(void);

   // Timer association.
   inline TIMER* getTimer( void ) { return _pTimer; }

private:
   TIMER* _pTimer;                     // Timer associated with this hardware.

};

#endif
