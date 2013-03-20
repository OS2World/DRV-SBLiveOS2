/* $Id: stacktoflat.h,v 1.1 2000/04/23 14:55:27 ktk Exp $ */

//******************************************************************************
// Header for stack to flat macros
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
#ifndef __STACKTOFLAT_H__
#define __STACKTOFLAT_H__

extern ULONG TKSSBase;
#pragma aux TKSSBase "TKSSBase"

extern ULONG stacksel;		//16 bits stack selector
#pragma aux stacksel "stacksel"

extern ULONG stackbase;		//32 bits stackbase
#pragma aux stackbase "stackbase"

extern ULONG GetTKSSBase();
#pragma aux GetTKSSBase "GetTKSSBase" \
  value [eax];

#ifdef KEE
#define __StackToFlat(addr)	(LINEAR)((unsigned long)(addr&0xffff) + stackbase)
#else
#define __StackToFlat(addr)	(LINEAR)((unsigned long)(addr&0xffff) + *(unsigned long *)TKSSBase)
#endif

#define MAKE_FAR48(a)	__Make48Pointer(a)

// Convert 16:16 pointer to 16:32
char __far *__Make48Pointer(unsigned long addr1616);
#pragma aux __Make48Pointer =          \
  "movzx edx, ax"   \
  "shr   eax, 16"   \
  "mov   fs, ax"    \
  parm [eax]        \
  value [fs edx];

#ifdef KEE
//Only valid for stack based pointer!!
#define __FlatToStack(addr32)	((stacksel << 16) | (((ULONG)addr32 - stackbase) & 0xffff))
#else
//Only valid for pointer previously constructed by __Make48Pointer!!
//(upper 16 bits of 32 bits offset must be 0)
FARPTR16 __Compress48Pointer(char FAR48 *addr1632);
#pragma aux __Compress48Pointer = \
  "mov   ax, gs"    \
  "shl   eax, 16"   \
  "mov   ax, dx"    \
  parm [gs edx]     \
  value [eax];
#endif

#define MAKE_FAR16(a)	__Compress48Pointer((char FAR48 *)a)

USHORT GETFARSEL(char FAR48 *addr1632);
#pragma aux GETFARSEL = \
  "mov   ax, gs"    \
  parm [gs edx]     \
  value [ax];

ULONG GETFAROFFSET(char FAR48 *addr1632);
#pragma aux GETFAROFFSET = \
  "mov   eax, edx"  \
  parm [gs edx]     \
  value [eax];

//SvL: Only works for DS & SS ptrs!
ULONG GETFLATPTR(char FAR48 *ptr);

#define FLATPTR(a)	GETFLATPTR((char FAR48 *)a)

#endif
