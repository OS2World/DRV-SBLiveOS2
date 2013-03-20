/* $Id: event.hpp,v 1.1 2000/04/23 14:55:15 ktk Exp $ */

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
 * Defines, class definations and prototypes for the EVENT class
 * @version %I%
 * @context Unless otherwise noted, all interfaces are Ring-0, 16-bit,
 *  <stack context>.
 * @history
 *
 */
#ifndef EVENT_INCLUDED
#define EVENT_INCLUDED

#ifndef OS2_INCLUDED
#define INCL_NOPMAPI
#include <os2.h>
#endif

#ifndef DDCMD_REG_STREAM         // shdd.h can't handle being included twice
#include <shdd.h>                // for PDDCMDREGISTER
#endif

#include "queue.hpp"

class EVENT : public QUEUEELEMENT {
   HEVENT he;                 // the event handle
   PSTREAM pstream;           // the stream for this event
   ULONG ulRepeatTime;        // for recurring events
   ULONG ulNextTime;          // the time this event should occur
   ULONG ulFlags;             // event flags single/recurring
   SHD_REPORTEVENT shdre;
public:
   void Report(ULONG time);
   HEVENT GetHandle(void);
   ULONG  GetEventTime(void);
   void UpdateEvent(PSTREAM, HEVENT, PCONTROL_PARM);
   EVENT(PSTREAM, HEVENT, PCONTROL_PARM);
};
typedef EVENT * PEVENT;

PEVENT FindEvent(HEVENT he, PQUEUEHEAD pQH);
#endif
