/* $Id: maudio.cpp,v 1.1 2000/04/23 14:55:17 ktk Exp $ */

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
 *  MIDI audio hardware object implementation.
 * @version %I%
 * @context
 *  Unless otherwise noted, all interfaces are Ring-0, 16-bit, kernel stack.
 * @notes
 * @history
 */

#define INCL_NOPMAPI
#include <os2.h>
#include <os2medef.h>
#include <audio.h>
#include "..\midi\midi_idc.h"          // RTMIDI interfaces,
#include "maudio.hpp"

#pragma off (unreferenced)


void MIDIAUDIO::DevCaps(PAUDIO_CAPS pCaps)
{

   pCaps->ulSupport = SUPPORT_SUCCESS;
   pCaps->ulDataSubType = SUBTYPE_NONE;
   pCaps->ulResourceUnits = 1;
   pCaps->ulResourceClass = 1;
   pCaps->fCanRecord = FALSE;
   pCaps->ulFlags = INPUT   |          // Input select is supported
                    OUTPUT  |          // Output select is supported
                    MONITOR |          // Monitor is supported
                    VOLUME;            // Volume control is supported
}


MIDIAUDIO::MIDIAUDIO( ULONG devicetype, TIMER* pTimer ) :
   AUDIOHW( devicetype ),
   _pTimer ( pTimer )
{};


// Default routines for RTMIDI - always return failure codes
// if these haven't been redefined by the sub-classes.

virtual int MIDIAUDIO::writeByte(BYTE b)
{
   return 0;    // Failure.
}

virtual int MIDIAUDIO::readByte(void)
{
   return -1;   // Failure.
}

USHORT MIDIAUDIO::RTMIDI_OpenReceive(void)
{
   return MIDIERRA_NO_HARDWARE;
}

USHORT MIDIAUDIO::RTMIDI_OpenSend(void)
{
   return MIDIERRA_NO_HARDWARE;
}

USHORT MIDIAUDIO::RTMIDI_CloseReceive(void)
{
   return MIDIERRA_NO_HARDWARE;
}

USHORT MIDIAUDIO::RTMIDI_CloseSend(void)
{
   return MIDIERRA_NO_HARDWARE;
}
