/* $Id: queue.hpp,v 1.1 2000/04/23 14:55:19 ktk Exp $ */

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
#ifndef QUEUE_INCLUDED
#define QUEUE_INCLUDED

#ifndef OS2_INCLUDED
#define INCL_NOPMAPI
#include <os2.h>
#endif

class QUEUEELEMENT {

public:
    QUEUEELEMENT *pNext;   // next element on the queue
    QUEUEELEMENT(void):
    pNext(NULL)
    {};
};
typedef QUEUEELEMENT *PQUEUEELEMENT;

class QUEUEHEAD {
public:
    PQUEUEELEMENT Head(void);
    PQUEUEELEMENT Tail(void);
    void PushOnHead(PQUEUEELEMENT);
    void PushOnTail(PQUEUEELEMENT);
    PQUEUEELEMENT PopHead(void);
    PQUEUEELEMENT PopTail(void);
    USHORT DestroyElement(PQUEUEELEMENT);
    PQUEUEELEMENT PopElement(PQUEUEELEMENT);
    ULONG IsElements(void);
    QUEUEHEAD(void);
private:
    PQUEUEELEMENT pHead;
    PQUEUEELEMENT pTail;
};
typedef QUEUEHEAD *PQUEUEHEAD;
#endif
