/* $Id: midipars.cpp,v 1.2 2000/04/24 19:45:17 sandervl Exp $ */

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
 *  Midi parser - parses and interprets incoming Midi stream.  Handles timing
 *  commands by adjusting MIDISTREAM::lwait variable.  Handles MIDI channel
 *  and system commands by calls to MIDI hardware object that this stream is
 *  associated with.
 * @version %I%
 * @context
 *  Unless otherwise noted, all interfaces are Ring-0, 16-bit, kernel stack.
 * @notes
 *  See midimsg.cpp file for description of the IBM Sysex command.
 * @history
 */

#ifndef OS2_INCLUDED                   // Doesn't like being included twice.
extern "C" {                           // 16-bit header files are not C++ aware
   #define INCL_NOPMAPI
   #include <os2.h>
}
#endif                                 // end OS2_INCLUDED

#include <string.h>                    // _fmemcpy()
#include "midistrm.hpp"                // Object definition MIDISTREAM
#include "midimsg.hpp"                 // Object definition


/**@internal parse
 *  Parse the next byte of the input stream.  If we form a complete message,
 *  then dispatch the message for interpretation.
 * @param MIDIBTE bInput - next byte in the input stream
 * @return void
 * @notes This function reads and updates many of the data members in the
 *  MIDISTREAM object.  A few of the most pertinent ones are listed next.
 * @notes reads/updates PARSER_STATE MIDISTREAM::state - current state of
 *  the parser.
 * @notes reads/updates MIDIMSG MIDISTREAM::message - current message that
 *  we're assembling.
 * @notes reads/updates MIDISTREAM::lwait variable, based on receipt of 0xF8
 *  timing pulses.
 */
void MIDISTREAM::parse( MIDIBYTE bInput )
{
   MIDIBYTETYPE eInputType;            // 'bInput' status byte type.

   //--- Algorithm starts here.  First check for 0xF8 status byte
   // (timing pulse).  If present, then process & return immediately.

   if (bInput == (MIDIBYTE) 0xF8) {    // Timing Clock, return.
      lWait += ulPerClock;
      return;
   }

   //--- Not a timing pulse, so continue parse of input stream based
   // on state and input.

   eInputType = eMidiByteType( bInput );
                                       // What class of bype did we get?
                                       // Returns member of MBT_* enumerated type.
   while (TRUE) {
      switch ( state ) {
      case S_Init:
         message.clear();
         message.addByte( bInput );
         switch ( eInputType ) {
         case MBT_ChannelStatus:
            state = S_ChannelMsg;
            break;
         case MBT_Sysex:
            state = S_SysexSignature;
            break;
         case MBT_SystemCommon:
            if (bInput != MidiByte_EOX) {    // We discard EOX's in Init state, such EOX's
                                             // mark end of Sysex's that we don't do.
               state = S_SystemCommon;
               if ( message.isComplete() ) {
                  dispatch( message );
                  state = S_Init;
               }
            }
            break;
         case MBT_SystemRT:               // Always 1 byte messgaes, handle immediately
            dispatch( message );          // with no state change.
            message.clear();
            break;
         case MBT_Data:                   // Discard input, no state change.
         default:
            break;
         }  // switch on midi byte type
         return;                          // end S_Init state changes.

      case S_ChannelMsg:
         // Currently parsing a channel message.  Anything other than a data
         // byte will reset us back into Init state.  If we have a complete
         // message, note that we save the status byte and then re-initialize
         // the message with the same status byte.  This implements the
         // "Running Status" feature of the MIDI definition.

         if ( eInputType != MBT_Data ) {
            state = S_Init;               // Continue from top of loop
            continue;                     // in S_Init state.
         }
         else {                           // Data byte, no state change.
            message.addByte( bInput );
            if ( message.isComplete() ) {
               MIDIBYTE runningStatus = message[0];
               dispatch( message );
               message.clear();
               message.addByte( runningStatus );
                                          // Remain in "ChannelMsg" state.
            }
            return;
         }

      case S_SysexSignature:
         if ( eInputType != MBT_Data ) {
            // We got a new status byte before receiving enough bytes to
            // perform a check for the IBM signature.

            // Flush the message buffer out to the hardware object.
            message.flush( (MIDIAUDIO*) pahw);

            // Return to S_Init state.  If an EOX, write it out and
            // start fresh with next byte;  otherwise let's loop back
            // and interpret this new status byte.
            state = S_Init;
            if ( bInput == MidiByte_EOX ) {
               ((MIDIAUDIO*) pahw)->writeByte( MidiByte_EOX );
               return;
            }
            else
               continue;               // Continue from top of loop with same input.
         }
         else {                        // Data byte, no state change.
            message.addByte( bInput );       // Add byte message.
            // If we've got the signature length, figure out if it's ours.
            if ( message.length() >= IBMSysexSigLen ) {
               if ( message.isIBMSignature() )
                  state = S_SysexIBM;
               else {
                  // Not IBM Sysex, flush buffered message and change state.
                  message.flush((MIDIAUDIO*) pahw);
                  state = S_SysexNotIBM;
               }
            }
            return;
         }

      case S_SysexIBM:
         // We enter this state after recognizing the IBM Sysex in the message.
         if ( eInputType != MBT_Data ) {  // Not a data byte?
            state = S_Init;               // Discard buffered msg, reset to Init state
            continue;                     // and continue from top of loop.
         }
         message.addByte( bInput );       // Add byte to message.
         if (message.isUnsupported()) {
                                          // We haven't implemented this IBM Sysex...
            state = S_Init;
            return;
         }
         else if (message.isComplete()) {
                                       // If we've got all the bytes...
            dispatch( message );       // Interpret the message
            state = S_Init;            // Back to Init state.
         }
         return;

      case S_SysexNotIBM:
         // Passthru bytes until we hit a status byte.
         if ( eInputType == MBT_Data ) {
            ((MIDIAUDIO*) pahw)->writeByte( bInput );
            return;                       // No state change.
         }
         else if ( bInput == MidiByte_EOX ) {
            ((MIDIAUDIO*) pahw)->writeByte( MidiByte_EOX );
            state = S_Init;
            return;                       // Sysex completed, start from S_Init when next byte arrives.
         }
         else {                           // Unexpected status byte.
            state = S_Init;               // Continue from top of loop
            continue;                     // in S_Init state.
         }

      }  // switch (state)
   }  // while TRUE
}


/**@internal dispatch
 *  Dispatch the MIDI message to the appropriate MIDI hw object.
 * @param MIDIMESSAGE message - a complete, well formed MIDI message.
 * @return void
 * @notes reads/updates MIDISTREAM::lwait variable, based on time
 *  compression commands.
 * @notes Non-IBM Sysex messages are not handled.  The parser handles
 *  these as soon as the non-IBM Sysex is recognized, by passing thru
 *  the bytes directly to the hardware object.
 */
void MIDISTREAM::dispatch( MIDIMSG& message )
{
   MIDIBYTE statusByte = message[0];
   MIDIBYTE cmdByte;                   // Used only for Sysex case.
   MIDIBYTETYPE statusType = eMidiByteType( statusByte );
   BYTE mchan;                         // MIDI channel for Channel cmds.

   switch( statusType ) {
   case MBT_ChannelStatus :
      mchan = statusByte & (BYTE) 0x0F;
      switch( statusByte & (BYTE) 0xF0 ) {

      case 0x80:                       // Note Off
         ((MIDIAUDIO*) pahw)->noteOff( mchan, message[1], message[2] );
         _notesOn[ (USHORT) message[1] ] &= ~(_usBitNumber[ mchan ]);
         break;

      case 0x90:                       // Note On
         ((MIDIAUDIO*) pahw)->noteOn( mchan, message[1], message[2] );
         _notesOn[ (USHORT) message[1] ] |= _usBitNumber[ mchan ];
         break;

      case 0xA0:                       // Polyphonic Key Pressure (Aftertouch)
         ((MIDIAUDIO*) pahw)->polyphonicPressure( mchan, message[1], message[2] );
         break;

      case 0xB0:                       // Control Change
         ((MIDIAUDIO *) pahw)->controlChange( mchan,  message[1], message[2] );
         break;

      case 0xC0:                       // Program Change
         ((MIDIAUDIO *) pahw)->programChange( mchan, message[1] );
         break;

      case 0xD0:                       // Channel Pressure (Aftertouch)
         ((MIDIAUDIO*) pahw)->channelPressure( mchan, message[1] );
         break;

      case 0xE0:                       // Pitch Bend
         ((MIDIAUDIO *) pahw)->pitchBend( mchan, message[1], message[2] );
         break;

      default:
         //### Should never get here, should log soft error.
         break;

      }  // End switch() for Channel messages.
      break;

   case MBT_SystemRT:                  // System Real Time:  all single byte messages.
      ((MIDIAUDIO*) pahw)->writeByte( statusByte );
      break;

   case MBT_SystemCommon:              // System Common
      message.flush((MIDIAUDIO*) pahw);
      break;

   case MBT_Sysex:  // Must be the IBM sysex
      //### Should check relative freq. of each cmd.  Currently coded to
      //### favor Timing Compression (short), then TC (long), then DD cmds

      message.discardSignature();      // Remove the IBM signature prefix.
      cmdByte = message[0];            // 1st cmd byte of the IBM sysex.

      if (cmdByte >= 7) {              // Timing Compression (Short) [ 7-127 ]
	 //SvL: += instead of = (BUGFIX)
         lWait += cmdByte * ulPerClock;
      }

      else if (cmdByte == 1) {         // Timing compression (Long) [ 1 ]
	 //SvL: += instead of = (BUGFIX)
         lWait += (((ULONG) (message[2] << 7)) + ((ULONG) message[1])) * ulPerClock;
      }

//      else if (cmdByte != 3)           // Sysex (ignored) [0 | 1 | 2 | 4 | 5 | 6]
//         pTrace->vLog( MIDISTREAM_SysXIgnored, (BYTE) message[1], (BYTE) message[2] );

      else if (message[1] == 1) {      // PPQN Control [ 3 ] [ 1 ] [time ctl] [PPQN value]
         usCPQN = message[3];
         CalcDelay();
         // ### Time control flags in message[2] are ignored
      }

      else if (message[1] == 2) {      // Tempo Control [ 3 ] [ 2 ] [ lsb ] [ msb ]
         ulTempo = (((WORD) message[3]) << 7) + message[2];
         CalcDelay();
         // ### Tempo fade in message[4] is ignored
      }
//      else                             // Sysex (ignored) [ 3 ] ^[ 1 | 2 ]

      break;
      // End IBM Sysex message processing.

   default:
      // ### Should never get here, should log soft error.
      break;
   }
}
