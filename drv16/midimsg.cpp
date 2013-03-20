/* $Id: midimsg.cpp,v 1.1 2000/04/23 14:55:17 ktk Exp $ */

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
 *  MIDIMSG object implementation.
 * @version %I%
 * @context
 *  Unless otherwise noted, all interfaces are Ring-0, 16-bit, kernel stack.
 * @notes
 *  For reference, this is the format of the IBM Sysex command.  Many more
 *  commands are defined by the IBM MMPM/2 command reference, but the subset
 *  noted here is all that is implemented.  Most of the other commands are
 *  historical for an IBM proprietary sound card, now withdrawn from market.
 *
 *  Sysex command format:  F0 00 00 3A cmd [ subCmd | data ] data *
 *
 *      cmd  subCmd    (cmdName)    Data
 *     -------------  -----------  ---------------------------------
 *        1            tc long     lsb msb (discard *)
 *        3    1       ppqn ctl    (discard 1) PPQN_byte (discard *)
 *        3    2       tempo       lsb msb
 *      7-127          tc short    (none)
 *
 *  'tc' = time compression
 *
 * @history
 */

#ifndef OS2_INCLUDED                   // Doesn't like being included twice.
extern "C" {                           // 16-bit header files are not C++ aware
   #define INCL_NOPMAPI
   #include <os2.h>
}
#endif                                 // end OS2_INCLUDED

#include <string.h>                    // _fmemcpy()
#include "maudio.hpp"                  // Object definition MIDIAUDIO
#include "midimsg.hpp"                 // Object definition


/**@external eMidiByteType
 *  Determine the type of the Midi status byte.
 * @param MIDIBYTE b
 * @return MIDIBYTETYPE, as enumerated in midimsg.hpp.
 * @notes Ref. .hpp file for definition of the enumerated values.
 */
MIDIBYTETYPE eMidiByteType( MIDIBYTE b )
{
   if (b < 0x80)
      return MBT_Data;
   else if (b < 0xF0)
      return MBT_ChannelStatus;
   else if (b == 0xF0)
      return MBT_Sysex;
   else if (b < 0xF8)
      return MBT_SystemCommon;
   else
      return MBT_SystemRT;
}


// Do we have a complete message?
BOOL MIDIMSG::isComplete( void )
{
   int expectedLength = _queryExpectedLength();
   return (expectedLength > 0) && (_length >= expectedLength) ;
}


// Is the message unsupported?
BOOL MIDIMSG::isUnsupported( void )
{
   return _queryExpectedLength() < 0;
}


// Concatenate 1 byte to end of message.
VOID MIDIMSG::addByte( MIDIBYTE b )
{
   if (_length < MAX_MidiMsgLen) {
      _msg[ _length ] = b;
      ++ _length;
   }
}


// Flush message to a MIDI hardware object using writeByte() method.
VOID MIDIMSG::flush( MIDIAUDIO* pmhw )
{
   for ( int i=0; i<_length; ++i) pmhw->writeByte( _msg[i] );
}


/**@external MIDIMSG::discardSignature
 *  Delete the IBM signature prefix from the front of an IBM Sysex msg,
 *  shift the remaining bytes to the front of the string.
 * @notes We assume that the IBM signature is there;  we don't check for it.
 */
VOID MIDIMSG::discardSignature( void )
{
   for (int i=0; i < _length; ++i) {
      _msg[i] = _msg[i + IBMSysexSigLen];
   }
   _length -= IBMSysexSigLen;
}


/**@internal MIDIMSG::_queryExpectedLength()
 *  Given a status byte, returns expected length of a message,
 *  (including the status byte).
 * @param None.  Examines _msg[].
 * @return int >0 - Expected length of complete message.
 * @return int 0 - Don't have enough bytes to determine length
 *  of complete message.
 * @return negative int MIDIMSG_Err_NotStatus when 1st byte in
 *  message isn't a status byte.
 * @return negative int MIDIMSG_Err_BadSysex when buffered message
 *  is recognized as an unsupported IBM sysex.
 * @notes A nonzero value for _expectedLength indicates we've already
 *  computed this.  We'll use it if we've got it, otherwise we'll compute
 *  it and save the value.
 */
int MIDIMSG::_queryExpectedLength( void )
{
   MIDIBYTE b;
   int iResult;

   // Check if already figured out the expected length.  Once the expected
   // message length is computed for a given message, it doesn't change.
   // This function is called for every byte received, can optimize here.
   if (_expectedLength) {
      return _expectedLength;
   }

   b = _msg[0];
   switch( eMidiByteType( b )) {
   case MBT_ChannelStatus:
      {
         int cmdIndex = (b >> 4) - 8;  // Transform 0x80..0xE0 to 0..6
         static UCHAR channelMsgLen[] = { 3, 3, 3, 3, 2, 2, 3 };
         iResult = channelMsgLen[ cmdIndex ];
      }
      break;

   case MBT_Sysex:
      iResult = _queryExpectedSysexLength();
      break;

   case MBT_SystemCommon:
      {
         int cmdIndex = (b & 7) - 1;  // Transform 0xF1..0xF7 to 0..6
         static UCHAR sysMsgLen[] = { 2, 3, 2, 1, 1, 1, 1 };
         iResult = sysMsgLen[ cmdIndex ];
      }
      break;

   case MBT_SystemRT:
      iResult = 1;                     // All 0xF8 - 0xFF are 1 byte.
      break;

   case MBT_Data:
   default:
      iResult = MIDIMSG_Err_NotStatus;
      break;
   }

   _expectedLength = iResult;
   return iResult;
}


/**@internal MIDIMSG::_queryExpectedSysexLength
 *  Returns expected length of an IBM sysex command (incl. signature prefix).
 * @param None.  Examines _msg[].
 * @return int Expected length of IBM sysex command.
 * @return 0 If command cannot be determined (some commands
 *  require several command bytes).
 * @return negative int MIDIMSG_Err_BadSysex If this is a Sysex command that
 *  we don't handle.
 * @notes The expected return length includes the length of the IBM Sysex
 *  signature prefix, but not the 0xF7 "EOX" byte.  The length returned
 *  counts only up to the number of bytes we need to interpret the command,
 *  which might not be all the bytes in the message definition.
 */
int MIDIMSG::_queryExpectedSysexLength( void )
{
   MIDIBYTE b1 = _msg[ IBMSysexSigLen ];     // First Sysex cmd byte.
   MIDIBYTE b2 = _msg[ IBMSysexSigLen+1 ];   // Second Sysex cmd byte.

   if (_length <= IBMSysexSigLen)
      return 0;

   switch( b1 ) {
   case 1:                             // Timing compression Long:  2 data bytes follow.
      return IBMSysexSigLen + 3;
   case 7:                             // Timing compression short:  No data bytes.
      return IBMSysexSigLen + 1;
   case 3:                             // Device driver control.
      if (_length <= IBMSysexSigLen+1) // Have 2nd cmd byte?  Return 0 if not.
         return 0;
      else if (b2 == 1)
         return IBMSysexSigLen + 4;
      else if (b2 == 2)
         return IBMSysexSigLen + 4;
      else
         return MIDIMSG_Err_BadSysex;
   default:
      return MIDIMSG_Err_BadSysex;
   }
}

