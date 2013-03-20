/* $Id: end.h,v 1.1 2000/04/23 14:55:15 ktk Exp $ */

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
 *  Defines symbols which mark end of data regions.
 * @version %I%
 * @context
 *  Unless otherwise noted, all interfaces are Ring-0, 16-bit, kernel stack.
 *  Used for Init time only purposes to discard init data.
 * @notes
 * @history
 */

#ifndef END_INCLUDED
#define END_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

extern int end_of_data;
extern int end_of_heap;
extern int end_of_initdata;
extern int end_of_text;

// void end_of_text(void);

#ifdef __cplusplus
}
#endif

#endif
