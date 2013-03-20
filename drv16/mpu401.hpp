/* $Id: mpu401.hpp,v 1.3 2000/05/28 16:50:40 sandervl Exp $ */

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
 *  MPU_401 object definition.
 * @version %I%
 * @context
 *  Unless otherwise noted, all interfaces are Ring-0, 16-bit, kernel stack.
 * @notes
 * @history
 */

#ifndef MPU401_INCLUDED
#define MPU401_INCLUDED

#ifndef OS2_INCLUDED
#define INCL_NOPMAPI
#include <os2.h>
#endif

#include "maudio.hpp"
#include "irq.hpp"

#define DSR 0x80
#define DRR 0x40

unsigned MPUcommand(USHORT, BYTE);
// sends byte to an I/O address.  Returns non-zero if error

class MPU_401 : public MIDIAUDIO {
   int fOpenedSend;              // TRUE if opened for RTMIDI Send (recording)
   int fOpenedRecv;              // TRUE if opened for RTMIDI Receive (playback)
   BYTE n;                       // This is the nth MPU-401
   int  _iAllNotesOff(void);      // function to turn all notes off
   ULONG midiInStreamId;
   ULONG midiOutStreamId;
public:
   // Constructor.
   MPU_401( TIMER* pTimer );

   static void processIrq(unsigned long streamid);

   // Standard MIDI channel commands.
   virtual void noteOff( BYTE mchan, BYTE note, BYTE velocity );
   virtual void noteOn( BYTE mchan, BYTE note, BYTE velocity );
   virtual void polyphonicPressure( BYTE mchan, BYTE note, BYTE value );
   virtual void controlChange( BYTE mchan, BYTE control_number, BYTE value );
   virtual void programChange( BYTE mchan, BYTE program_number );
   virtual void channelPressure( BYTE mchan, BYTE value );
   virtual void pitchBend( BYTE mchan, BYTE value_lsb, BYTE value_msb );

   // Direct byte level IO... used for handling System Common & Exclusive
   // commands, as well as for RTMIDI
   virtual int writeByte(BYTE b);
   virtual int readByte(void);

   // Stream control.
   virtual int Reset(STREAM *stream);
   virtual int Start(STREAM *stream);
   virtual int Stop(STREAM *stream);
   virtual int Pause(STREAM *stream);
   virtual int Resume(STREAM *stream);

   // Services provided to the RTMIDI interface.
   virtual USHORT RTMIDI_OpenReceive(void);
   virtual USHORT RTMIDI_OpenSend(void);
   virtual USHORT RTMIDI_CloseReceive(void);
   virtual USHORT RTMIDI_CloseSend(void);
};

#endif
