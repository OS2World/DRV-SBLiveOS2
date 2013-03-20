/* $Id: rtmidi.cpp,v 1.1 2000/04/23 14:55:20 ktk Exp $ */

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
 *  RTMIDI object implementation.  IDC Entry points shared between RTMIDI
 *  (MIDI.SYS) and this driver are implemented here.
 * @version %I%
 * @context
 *  Unless otherwise noted, all interfaces are Ring-0, 16-bit, kernel stack.
 * @notes
 * @history
 */

#define INCL_NOPMAPI
#include <os2.h>
#include <devhelp.h>
#include <include.h>
#include "midi_idc.h"          // RTMIDI interfaces
#include "maudio.hpp"                  // MIDIAUDIO
#include "rtmidi.hpp"                  // Object definition.


/**@internal RTMIDI::_bIsRTMIDIDevice
 *  Opens the MIDI device for either send or receive
 * @param USHORT pMidiAudio - pointer to MIDI Audio hardware object of interest.
 * @param USHORT usMode - either MIDIMODE_OPEN_RECEIVE or MIDIMODE_OPEN_SEND
 * @notes
 *  This function initializes (opens) the MIDI device for either send or receive.
 */
static BOOL RTMIDI::_bIsRTMIDIDevice( MIDIAUDIO* pma )
{
   // FMSYNTH not included because the HW object has no ability to
   // parse an incoming byte stream into MIDI commands at this time.
   // This could be accomplished by packaging the existing MIDI
   // parser as an object, and associating it with the hardware instead
   // of organizing it with the stream.

   return
      (pma->ulDeviceType == AUDIOHW_MPU401_CAPTURE) ||
      (pma->ulDeviceType == AUDIOHW_MPU401_PLAY) ;
}


static VOID RTMIDI::vConnect( VOID )
{
   static MIDI_ATTACH_DD DDTable;   // It's got to be in DS, so make it static.
   MIDI_REGISTER reg;
   MIDIREG_TYPEA regA;
   USHORT usRC;

   if (DevHelp_AttachDD((NPSZ) "MIDI$   ", (NPBYTE) &DDTable)) {
      return;
   }

   // Call the RTMIDI driver using the function pointer provided by the
   // AttachDD.  The function call returns void.  We check for errors by
   // verifying validity of returned data:  All MIDI.SYS function entry
   // points are in the same segment & must all have the same selector.

   reg.pfnRegisterA = 0;
   reg.usSize = sizeof(reg);
   DDTable.pfn( &reg );
   if (SELECTOROF(reg.pfnRegisterA) != SELECTOROF(DDTable.pfn)) {
      return;
   }

   regA.usSize = sizeof(regA);

   regA.in.pfnOpen = RTMIDI::Open;
   regA.in.pfnClose = RTMIDI::Close;
   regA.in.pfnRecvString = RTMIDI::RecvString;      // Required
   regA.in.pfnRecvByte = RTMIDI::RecvByte;          // Required

   // This is also an MMPM driver, so we don't want to support the MMPM bridge
   regA.in.pfnIOCtl=NULL;

   // Now register each MIDI hardware object.
   // We assume that all MIDI hardware objects are setup at INIT time,
   // and assume szName and ulCapabilities fields are properly init'd.
   // We use the MIDI hw object address as its device ID.

   MIDIAUDIO* pma = (MIDIAUDIO *) pAudioHWList->Head();
   while (pma) {
      if ( _bIsRTMIDIDevice( pma )) {
         regA.in.pszInstanceName = pma->szRTMIDI_Name;
         regA.in.flCapabilities = pma->ulRTMIDI_Caps;
         regA.in.usDevId = (USHORT) pma;
               // The device ID is the pointer to the object

         usRC = reg.pfnRegisterA( &regA );
         if (!usRC) {
            pma->ulRTMIDI_Handle = regA.out.ulHandle;
            pma->pfnSendByte = regA.out.pfnSendByte;
            pma->pfnDeregister = regA.out.pfnDeregister;
         }
      }
      pma = (MIDIAUDIO *) pma->pNext;
   }

   return;
}


/**@external RTMIDI::Open
 *  Opens the MIDI device for either send or receive
 * @param USHORT pMidiAudio - pointer to MIDI Audio hardware object of interest.
 * @param USHORT usMode - either MIDIMODE_OPEN_RECEIVE or MIDIMODE_OPEN_SEND
 * @notes
 *  This function initializes (opens) the MIDI device for either send or receive.
 */
static USHORT __far __loadds __cdecl
RTMIDI::Open(USHORT pMidiAudio, USHORT usMode)
{
   MIDIAUDIO *pma = (MIDIAUDIO *) pMidiAudio;

   switch (usMode) {
      case MIDIMODE_OPEN_RECEIVE:
         return pma->RTMIDI_OpenReceive();
      case MIDIMODE_OPEN_SEND:
         return pma->RTMIDI_OpenSend();
   }

   // Invalid mode requested, so just return general failure
   return MIDIERRA_GEN_FAILURE;
}


/**@external RTMIDI::Close
 *  Closes the MIDI device for either send or receive
 * @param USHORT pMidiAudio - pointer to MIDI Audio hardware object of interest.
 * @param USHORT usMode - either MIDIMODE_OPEN_RECEIVE or MIDIMODE_OPEN_SEND
 * @notes
 *  This function closes the MIDI device.
 */
static USHORT __far __loadds __cdecl
RTMIDI::Close(USHORT pMidiAudio, USHORT usMode)
{
   MIDIAUDIO *pma = (MIDIAUDIO *) pMidiAudio;

   switch (usMode) {
      case MIDIMODE_OPEN_RECEIVE:
         return pma->RTMIDI_CloseReceive();
      case MIDIMODE_OPEN_SEND:
         return pma->RTMIDI_CloseSend();
   }

   // Invalid mode requested, so just return general failure
   return MIDIERRA_GEN_FAILURE;
}


/**@external RTMIDI::RecvByte
 *  Receives a single byte of MIDI data from the MIDI driver
 * @param USHORT pMidiAudio - pointer to MIDI Audio hardware object of interest.
 * @param BYTE b - the byte to write out to the MIDI hardware
 * @notes
 *  The MIDI driver calls this function to send a single byte of MIDI data.
 *  If this function returns error, the MIDI driver will probably close device.
 */
static USHORT __far __loadds __cdecl
RTMIDI::RecvByte(USHORT pMidiAudio, BYTE b)
{
   MIDIAUDIO *pma = (MIDIAUDIO *) pMidiAudio;

   return pma->writeByte(b) ? 0 : MIDIERRA_HW_FAILED;
}


/**@external RecvString
 *  Receives a string (not null-terminated) of MIDI data from the MIDI driver
 * @param USHORT pMidiAudio - pointer to MIDI Audio hardware object of interest.
 * @param BYTE* pb - far pointer to the string
 * @param USHORT usLength - length of that string
 * @notes
 *  The MIDI driver calls this function to send a string of MIDI data.  Typically,
 *  this will be used only for large chunks of bytes, such as a SysEx message.
 *  If this function returns error, the MIDI driver will probably close device.
 */
static USHORT __far __loadds __cdecl
RTMIDI::RecvString(USHORT pMidiAudio, BYTE __far *pb, USHORT usLength)
{
   MIDIAUDIO *pma = (MIDIAUDIO *) pMidiAudio;

   while (usLength--)
      if (!pma->writeByte(*pb++))
         return MIDIERRA_HW_FAILED;

   return 0;
}

