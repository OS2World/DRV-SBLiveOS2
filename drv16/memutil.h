/* $Id: memutil.h,v 1.1 2000/04/23 14:55:17 ktk Exp $ */

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
 *
 * @version %I%
 * @context Unless otherwise noted, all interfaces are Ring-0, 16-bit,
 *  <stack context>.
 * @history
 *
 */
#ifndef MEMUTIL_INCLUDED
#define MEMUTIL_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

void cdecl ddmemmov(PVOID pdest, PVOID psrc, USHORT count);
void cdecl ddmemfill(PVOID pdest, USHORT count, USHORT value);

#ifdef __cplusplus
}
#endif
#endif
