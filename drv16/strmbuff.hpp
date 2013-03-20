/* $Id: strmbuff.hpp,v 1.5 2001/04/30 21:07:59 sandervl Exp $ */

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
 * Defines, class definations and prototypes for
 * @version %I%
 * @context Unless otherwise noted, all interfaces are Ring-0, 16-bit,
 *  <stack context>.
 * @history
 *
 */
#ifndef STREAMBUFFER_INCLUDED
#define STREAMBUFFER_INCLUDED

#include "queue.hpp"

typedef BYTE __far *PSTREAMBUF;

class STREAMBUFFER : public QUEUEELEMENT{

public:
    PSTREAMBUF  pBuffptr;  // pointer to the stream buffer
    ULONG       ulBuffsz;  // size of stream buffer
    ULONG       ulBuffpos; // Current buffer position
    ULONG       ulDonepos; // position at which the buffer can be returned
    ULONG       lock[3];	//lock handle
    ULONG       linLock;
    BOOL        fDone;
    BOOL        looping;

    STREAMBUFFER(ULONG bufsize, PSTREAMBUF bufptr, BOOL fLooping = FALSE):
    pBuffptr(bufptr),
    ulBuffsz(bufsize),
    ulBuffpos(0),
    ulDonepos(0),
    looping(fLooping),
    fDone(0),
    linLock(0)
    {};

};
typedef STREAMBUFFER *PSTREAMBUFFER;

#endif
