/* $Id: malloc.h,v 1.1 2000/04/23 14:55:17 ktk Exp $ */

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
 *  Interfaces to the driver's built in memory management.
 * @version %I%
 * @context
 *  Unless otherwise noted, all interfaces are Ring-3 and Ring-0, 16-bit,
 *  kernel stack.
 * @notes
 * @history
 *  01-Jul-95  Timur Tabi   Creation
 */

#ifndef MALLOC_INCLUDED
#define MALLOC_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

// Standard malloc.h functions

void __near *malloc(unsigned);
void free(void __near *);
void __near *realloc(void __near *, unsigned);
unsigned _msize(void __near *);


// // Traditional 'C' library memset() -- don't need, intrinsic function !!!
// PVOID memset ( PVOID p, int c, USHORT len );

// Some extensions
unsigned _memfree(void);            // returns available space

// Specialized routines
unsigned HeapInit(unsigned);        // initializes the heap manager
void dumpheap(void);


#ifdef __cplusplus
}
#endif

#endif
