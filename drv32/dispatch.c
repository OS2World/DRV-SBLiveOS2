/* $Id: dispatch.c,v 1.1 2000/04/23 14:55:23 ktk Exp $ */

//******************************************************************************
// IOCtl & close strategy handlers
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

#include <devhelp.h>
#include <devtype.h>
#include <devrp.h>
#include "devown.h"

//******************************************************************************
// Dispatch IOCtl requests received from the Strategy routine
//******************************************************************************
ULONG StratIOCtl(RP __far* _rp)
{
  RPIOCtl __far* rp = (RPIOCtl __far*)_rp;

  return RPERR_COMMAND | RPDONE;
}
//******************************************************************************
// Dispatch Close requests received from the strategy routine
//******************************************************************************
ULONG StratClose(RP __far* _rp)
{
  RPOpenClose __far* rp = (RPOpenClose __far*)_rp;

  // only called if device successfully opened
  numOS2Opens--;

  if (numOS2Opens == 0) {
	deviceOwner = DEV_NO_OWNER;
  }
  return(RPDONE);
}
