/* $Id: midimsg.hpp,v 1.1 2000/04/23 14:55:18 ktk Exp $ */

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
 *  MIDIMSG class definition.  Defines a class which is used to assemble
 *  valid MIDI messages, and perform certain predicate queries on the MIDI
 *  message strings (such as "is the message complete?").
 * @version %I%
 * @context
 *  Unless otherwise noted, all interfaces are Ring-0, 16-bit, kernel stack.
 * @notes
 *  See .cpp file for description of the IBM Sysex command.
 * @history
 */

#ifndef MIDIMSG_INCLUDED
#define MIDIMSG_INCLUDED

#ifndef OS2_INCLUDED                   // Doesn't like being included twice.
extern "C" {                           // 16-bit header files are not C++ aware
   #define INCL_NOPMAPI
   #include <os2.h>
}
#endif                                 // end OS2_INCLUDED

#include <string.h>                    // _fmemset()
#include "maudio.hpp"                  // Object definition (MIDIAUDIO).


// One byte in a MIDI stream.  Must be unsigned.
typedef UCHAR MIDIBYTE;

// Types of MIDI status bytes.
enum MIDIBYTETYPE
{
   MBT_Data,                           // Not status.  Data byte (0x00 - 0x7F)
   MBT_ChannelStatus,                  // Channel status byte (0x80 - 0xEF)
   MBT_Sysex,                          // System Exclusive (0xF0)
   MBT_SystemCommon,                   // System Common (0xF1 - 0xF7)
   MBT_SystemRT                        // Real-time (0xF8 - 0xFF)
};

// Return the MIDIBYTETYPE (enumerated above) of a MIDI byte.
extern MIDIBYTETYPE eMidiByteType( MIDIBYTE b );

// Byte length of the IBM Sysex Signature (0xF000003A).
const IBMSysexSigLen = 4;

// Miscellaneous error codes, must be <0.
const int MIDIMSG_Err_NotStatus = -1;  // Got a data bytes when expected status.
const int MIDIMSG_Err_BadSysex = -2;   // Unsupported IBM sysex.

// 0xF7 EOX
const MIDIBYTE MidiByte_EOX = ((MIDIBYTE) 0xF7);

// We don't accumulate more than this number of bytes.  For vbl length
// messages (eg a non-IBM Sysex), we passthru the message byte by byte.
// Longest message at time of this writing is 4 byte IBM Sysex signature
// plus 4 bytes sysex command and data == 8 bytes.  We'll tack on 2 extra.
const MAX_MidiMsgLen = (IBMSysexSigLen + 4 + 2);

// We use this class to build up MIDI messages as we receive the bytes.
class MIDIMSG {
public:
   // Constructor.  Clear message.
   MIDIMSG( void ) { clear(); }

   // Clear message & set current length, expected length to 0.
   void clear( void ) { _fmemset((PVOID) this, 0, sizeof(MIDIMSG)); }

   // Query current message length.
   int length( void ) { return _length; }

   // Concatenate 1 byte to end of message.
   void addByte( MIDIBYTE b );

   // Do we have a complete message?
   BOOL isComplete( void );

   // Is the message unsupported?
   BOOL isUnsupported( void );

   // Does current message match the IBM Sysex signature?
   BOOL isIBMSignature() {
      return( (_length >= IBMSysexSigLen) && (*((ULONG*) _msg) == 0x3A0000F0) );
   }

   // Delete the IBM signature prefix from the front of an IBM Sysex msg.
   VOID discardSignature( void );

   // Return byte in the message buffer.  No range checking -- too
   // expensive a performance hit.
   inline const MIDIBYTE operator [] (int index) { return _msg[index]; }

   // Flush message to a MIDI hardware object using writeByte() method.
   VOID flush( MIDIAUDIO* pmhw );

private:
   // Private data.

   int   _length;                      // Number of bytes that we've accumulated
                                       // in the current message.
   int   _expectedLength;              // Expected length of messages based on
                                       // bytes accumulated;  0 if don't know,
                                       // <0 if unimplemented MIDI message (some
                                       // of the IBM sysex messages are not implemented.
   UCHAR _msg[ MAX_MidiMsgLen ];       // Message content.

   // Private methods.

   // Query expected length of message currently being assembled.
   int _queryExpectedLength( void );

   // Query expected length - helper function for IBM sysex messages.
   int _queryExpectedSysexLength( void );
};

#endif
