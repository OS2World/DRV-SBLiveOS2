/* $Id: strategy.c,v 1.1 2000/04/23 14:55:25 ktk Exp $ */

//******************************************************************************
// Strategy entry point
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
#define INCL_DOSINFOSEG
#include <os2.h>
}

#include <devhelp.h>
#include <devrp.h>
#include <devown.h>
#include "strategy.h"
#include <ossidc.h>

ULONG StratIOCtl(RP __far *_rp);
ULONG StratClose(RP __far *_rp);

// SvL: Needed in StratOpen
ULONG deviceOwner = DEV_NO_OWNER;
ULONG numOS2Opens = 0;
// SvL: End

ULONG StratOpen(RP __far*)
{
	if (numOS2Opens == 0) {
		deviceOwner = DEV_PDD_OWNER;
	}
	numOS2Opens++;
        return RPDONE;
}

#pragma off (unreferenced)
static ULONG StratWrite(RP __far* _rp)
#pragma on (unreferenced)
{
  return RPDONE | RPERR;
}

extern ULONG DiscardableInit(RPInit __far*);  

// External initialization entry-point
ULONG StratInit(RP __far* _rp)
{
  RPInit __far* rp = (RPInit __far*)_rp;
  return DiscardableInit(rp);
}


// External initialization entry-point
#pragma off (unreferenced)
ULONG StratInitComplete(RP __far* _rp)
#pragma on (unreferenced)
{
  return(RPDONE);
}

#pragma off (unreferenced)
ULONG StratShutdown(RP __far *_rp)
#pragma on (unreferenced)
{
 RPShutdown __far *rp = (RPShutdown __far *)_rp;

  if(rp->Function == 1) {//end of shutdown
  	OSS32_RemoveDriver(); 
  }
  return(RPDONE);
}

// Handle unsupported requests
static ULONG StratError(RP __far*)
{
  return RPERR_COMMAND | RPDONE;
}


// Strategy dispatch table
//
// This table is used by the strategy routine to dispatch strategy requests

typedef ULONG (*RPHandler)(RP __far* rp);
RPHandler StratDispatch[] =
{
  StratInit,                  // 00 (BC): Initialization
  StratError,                 // 01 (B ): Media check
  StratError,                 // 02 (B ): Build BIOS parameter block
  StratError,                 // 03 (  ): Unused
  StratError,                 // 04 (BC): Read
  StratError,                 // 05 ( C): Nondestructive read with no wait
  StratError,                 // 06 ( C): Input status
  StratError,                 // 07 ( C): Input flush
  StratWrite,                 // 08 (BC): Write
  StratError,                 // 09 (BC): Write verify
  StratError,                 // 0A ( C): Output status
  StratError,                 // 0B ( C): Output flush
  StratError,                 // 0C (  ): Unused
  StratOpen,                  // 0D (BC): Open
  StratClose,                 // 0E (BC): Close
  StratError,                 // 0F (B ): Removable media check
  StratIOCtl,                 // 10 (BC): IO Control
  StratError,                 // 11 (B ): Reset media
  StratError,                 // 12 (B ): Get logical unit
  StratError,                 // 13 (B ): Set logical unit
  StratError,                 // 14 ( C): Deinstall character device driver
  StratError,                 // 15 (  ): Unused
  StratError,                 // 16 (B ): Count partitionable fixed disks
  StratError,                 // 17 (B ): Get logical unit mapping of fixed disk
  StratError,                 // 18 (  ): Unused
  StratError,                 // 19 (  ): Unused
  StratError,                 // 1A (  ): Unused
  StratError,                 // 1B (  ): Unused
  StratShutdown,              // 1C (BC): Notify start or end of system shutdown
  StratError,                 // 1D (B ): Get driver capabilities
  StratError,                 // 1E (  ): Unused
  StratInitComplete           // 1F (BC): Notify end of initialization
};



// Strategy entry point
//
// The strategy entry point must be declared according to the STRATEGY
// calling convention, which fetches arguments from the correct registers.

ULONG Strategy(RP __far* rp);
#pragma aux (STRATEGY) Strategy "SBLIVE_STRATEGY";
ULONG Strategy(RP __far* rp)
{
  if (rp->Command < sizeof(StratDispatch)/sizeof(StratDispatch[0]))
       	return(StratDispatch[rp->Command](rp));
  else	return(RPERR_COMMAND | RPDONE);
}



