/* $Id: ossdefos2.h,v 1.1 2000/04/23 14:55:26 ktk Exp $ */

//******************************************************************************
// Header for type functions
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
#ifndef __OSSDEFOS2_H__
#define __OSSDEFOS2_H__

#define FAR48 __far
#ifndef FAR
#define FAR   __far
#endif

#ifndef TARGET_OS216

#ifdef DEBUG
#ifdef __cplusplus
extern "C" {
#endif
void _cdecl DPE(char *x, ...) ; /* not debugging: nothing */
void _cdecl DPD(int level, char *x, ...) ; /* not debugging: nothing */
#ifdef __cplusplus
}
#endif
#define dprintf(a)	DPE a
#define DebugInt3()	_asm int 3;
#else
#define dprintf(a)
#define DebugInt3()
#endif

#ifdef LINUX
#define NEAR __near

typedef unsigned long ULONG;
typedef unsigned long *PULONG;
typedef unsigned short USHORT;
typedef unsigned short *PUSHORT;
typedef signed long LONG;
typedef signed short SHORT;
typedef signed short *PSHORT;
typedef unsigned long BOOL;
typedef ULONG FARPTR16;
typedef char NEAR *LINEAR;
typedef unsigned long APIRET;
typedef void VOID;
typedef void *PVOID;
typedef signed int INT;
typedef unsigned char UCHAR;
typedef signed char CHAR;
typedef CHAR *PCHAR;
typedef __int64 QWORD;
typedef ULONG HFILE;

#define APIENTRY    _System

#endif

#endif //TARGET_OS216

#endif
