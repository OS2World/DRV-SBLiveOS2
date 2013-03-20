/* $Id: sizedefs.h,v 1.1 2000/04/23 14:55:20 ktk Exp $ */

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
 *  Definitions for the sizes of various arrays, max number of objects, etc.
 * @version %I%
 * @context
 *  Unless otherwise noted, all interfaces are Ring-0, 16-bit, kernel stack.
 * @notes
 * @history
 */

#ifndef SIZEDEFS_H
#define SIZEDEFS_H

const NumLogicalDevices = 5; // Number of PNP logical devices on adapter.

#define HEAP_SIZE         16384         // must be a multiple of 4
#define DEFAULT_HEAP      4096


#endif  // SIZEDEFS_H


