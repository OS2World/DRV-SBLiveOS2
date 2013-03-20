/* $Id: init.c,v 1.3 2001/04/14 17:03:35 sandervl Exp $ */

//******************************************************************************
// Init strategy handler
//
// Copyright 2000 Sander van Leeuwen (sandervl@xs4all.nl)
//
//     This program is free software; you can redistribute it and/or
//     modify it under the terms of the GNU General Public License as
//     published by the Free Software Foundation; either version 2 of
//     the License, or (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public
//     License along with this program; if not, write to the Free
//     Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
//     USA.
//
//******************************************************************************
extern "C" {               // 16-bit header files are not C++ aware
#define INCL_NOPMAPI
#define INCL_DOSMISC
#include <os2.h>
}
#include <string.h>

// Device support
#include <devhelp.h>
#include <devtype.h>
#include <devrp.h>
#include "devown.h"
#include <sbversion.h>
#include <dbgos2.h>
#ifdef KEE
#include <kee.h>
#endif

const char ERR_ERROR[]   = "ERROR: ";
const char ERR_LOCK[]    = "\r\nUnable to lock 32 bit data & code segments, exiting...\r\r\n";
#ifdef KEE
const char szSBLive[]    = "\r\n\r\nSoundBlaster Live! Audio Driver v"SBLIVE_VERSION" (32 Bits KEE)\r\n";
#else
const char szSBLive[]    = "\r\n\r\nSoundBlaster Live! Audio Driver v"SBLIVE_VERSION" (32 Bits)\r\n";
#endif
const char szCopyRight1[]= "Copyright 1999, 2000 Creative Labs, Inc.\r\n";
const char szCopyRight2[]= "Copyright 2000-2001 Sander van Leeuwen (OS/2 Port)\r\n\r\n";
const char szCodeStartEnd[] = "Code 0x%0x - 0x%0x\r\n\r\n";
 
typedef struct {
 USHORT MsgLength;
 WORD32 MsgPtr;
} MSG_TABLE;

extern "C" WORD32 MSG_TABLE32;

extern "C" int sprintf (char *buffer, const char *format, ...);

//Print messages with DosWrite when init is done or has failed (see startup.asm)
void DevSaveMessage(char __far *str, int length)
{
 MSG_TABLE __far *msg = (MSG_TABLE __far *)__Make48Pointer(MSG_TABLE32);
 char __far *str16 = (char __far *)__Make48Pointer(msg->MsgPtr);
 int i;

  for(i=0;i<length;i++) {
	str16[msg->MsgLength + i] = str[i];
  }
  str16[msg->MsgLength + length] = 0;
  msg->MsgLength += length;

  return;
}

//SvL: Lock our 32 bits data & code segments
int LockSegments(void) 
{
#ifdef KEE
 KEEVMLock lock;
#else
 char   lock[12];
 ULONG  PgCount;
#endif
 ULONG  segsize;

    /*
     * Locks DGROUP into physical memory
     */
    //NOTE: VMLock rounds base address down to nearest page
    //      So we MUST take this into account when calculating the
    //      size of our code/data
    segsize = OffsetFinalDS32 - ((OffsetBeginDS32) & ~0xFFF);
    if(segsize & 0xFFF) {
	segsize += PAGE_SIZE;
    }
    segsize &= ~0xFFF;
#ifdef KEE
    if(KernVMLock(VMDHL_LONG,
                  (PVOID)((OffsetBeginDS32) & ~0xFFF),
                  segsize,
                  &lock,
                  (KEEVMPageList*)-1,
                  0)) {
#else
    if(DevVMLock(VMDHL_LONG,
                   ((OffsetBeginDS32) & ~0xFFF),
                   segsize,
                   (LINEAR)-1,
                   __StackToFlat((ULONG)lock),
                   (LINEAR)__StackToFlat((ULONG)&PgCount))) {
#endif
	return(1);
    }
    /*
     * Locks CODE32 into physical memory
     */
    segsize = OffsetFinalCS32 - ((OffsetBeginCS32) & ~0xFFF);
    if(segsize & 0xFFF) {
	segsize += PAGE_SIZE;
    }
    segsize &= ~0xFFF;
#ifdef KEE
    if(KernVMLock(VMDHL_LONG,
                  (PVOID)((OffsetBeginCS32) & ~0xFFF),
                  segsize,
                  &lock,
                  (KEEVMPageList*)-1,
                  0)) {
#else
    if(DevVMLock(VMDHL_LONG,
                 ((OffsetBeginCS32) & ~0xFFF),
                 segsize,
                 (LINEAR)-1,
                 __StackToFlat((ULONG)lock),
                 (LINEAR)__StackToFlat((ULONG)&PgCount))) {
#endif
	return(1);
    }
    return 0;
}


// Write a string of specified length
static VOID WriteString(const char __far* str, int length)
{
  // Write the string
  DevSaveMessage((char __far *)str, length);
  return;
}

// Initialize device driver
WORD32 DiscardableInit(RPInit __far* rp)  
{
 char        debugmsg[64];
 char FAR48 *args;

  GetTKSSBase();
  if(LockSegments()) {
    	WriteString(ERR_ERROR, sizeof(ERR_ERROR)-1);
    	WriteString(ERR_LOCK, sizeof(ERR_LOCK)-1);
    	return RPDONE | RPERR;
  }

  rp->Out.FinalCS = 0;
  rp->Out.FinalDS = 0;

  //Do you init here:

  WriteString(szSBLive, sizeof(szSBLive)-1);
  WriteString(szCopyRight1, sizeof(szCopyRight1)-1);
  WriteString(szCopyRight2, sizeof(szCopyRight2)-1);

  args = __Make48Pointer(rp->In.Args);
  while(*args && *args == ' ') args++;
  while(*args && *args != ' ') args++;
  while(*args && *args == ' ') args++;
  while(*args && *args != '/') args++;
  if(*args) args++;

  if(*args == 'D' || *args == 'd') {
	sprintf(debugmsg, szCodeStartEnd, OffsetBeginCS32, OffsetFinalCS32);
	WriteString(debugmsg, strlen(debugmsg));
  }

  // Complete the installation
  rp->Out.FinalCS = _OffsetFinalCS16;
  rp->Out.FinalDS = _OffsetFinalDS16;

  // Confirm a successful installation
  return RPDONE;
}




