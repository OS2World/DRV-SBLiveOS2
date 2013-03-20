/* $Id: rtmidi.hpp,v 1.1 2000/04/23 14:55:20 ktk Exp $ */

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
 *  RTMIDI object definition.
 * @version %I%
 * @context
 *  Unless otherwise noted, all interfaces are Ring-0, 16-bit, kernel stack.
 * @notes
 * @history
 */

#ifndef RTMIDI_INCLUDED
#define RTMIDI_INCLUDED

#ifndef OS2_INCLUDED
#define INCL_NOPMAPI
#include <os2.h>
#endif
#include "..\midi\midi_idc.h"          // RTMIDI interfaces
#include "maudio.hpp"                  // MIDIAUDIO

class RTMIDI {

public:
   static VOID   vConnect ( VOID );
   static USHORT __far __loadds __cdecl Open(USHORT, USHORT);
   static USHORT __far __loadds __cdecl Close(USHORT, USHORT);
   static USHORT __far __loadds __cdecl RecvString(USHORT, BYTE __far *, USHORT);
   static USHORT __far __loadds __cdecl RecvByte(USHORT, BYTE);

private:
   static BOOL   _bIsRTMIDIDevice( MIDIAUDIO* pma );

};

#endif
