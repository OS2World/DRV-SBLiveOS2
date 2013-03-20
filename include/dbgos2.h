/* $Id: dbgos2.h,v 1.2 2000/07/23 16:21:54 sandervl Exp $ */

//******************************************************************************
// Header for debug functions
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
#ifndef __COMMDBG_H__
#define __COMMDBG_H__

#ifdef __cplusplus
extern "C" {
#endif

void cdecl PrintfOut(char far *DbgStr , ...);
extern int dbglevel;

#ifdef __cplusplus
}
#endif

#ifdef DEBUG
#define dprintf(a)      PrintfOut a
#define dprintf2(a) if(dbglevel >= 2) PrintfOut a
#define dprintf3(a) if(dbglevel >= 3) PrintfOut a
#define dprintf4(a) if(dbglevel >= 4) PrintfOut a
#define DebugInt3()	_asm int 3
#else
#define dprintf(a)
#define dprintf2(a)
#define dprintf3(a)
#define dprintf4(a)
#define DebugInt3()	
#endif

#endif //__COMMDBG_H__
