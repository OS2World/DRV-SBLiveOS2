/* $Id: stream.cpp,v 1.5 2001/05/09 17:44:24 sandervl Exp $ */

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
 * @notes
 * @version %I%
 * @context Unless otherwise noted, all interfaces are Ring-0, 16-bit,
 *  <stack context>.
 * @history
 *
 */
#define INCL_NOPMAPI
#define INCL_DOSERRORS            // for ERROR_INVALID_FUNCTION
#include <os2.h>
#include <os2me.h>
#include <audio.h>

#include <include.h>
#include <devhelp.h>

#include "stream.hpp"
#include "queue.hpp"
#include "event.hpp"
#include "strmbuff.hpp"
#include <ossidc.h>

PQUEUEHEAD pStreamList; // List head for Streams.  Initialized during DD initilialization.

void STREAM::ReturnBuffer(void)
{
   SHD_REPORTINT shdri; // structure used to return buffers to SHD
   PSTREAMBUFFER temp = (PSTREAMBUFFER)qhDone.PopHead();

   if (temp) 
   {
	shdri.ulFunction = SHD_REPORT_INT;

      	// if this is a write (playback) then set the streamtype and
      	// tell the stream handler that we played all of the buffer.
      	if (ulStreamType & STREAM_WRITE) {
         	shdri.ulFlag = SHD_WRITE_COMPLETE;
         	shdri.ulStatus = temp->ulBuffsz;
      	}
        // if this is a capture then tell the stream hamdler
        // how much data we wrote to the buffer
      	else {
         	shdri.ulFlag = SHD_READ_COMPLETE;
         	shdri.ulStatus = temp->ulBuffpos;
      	}
      	shdri.hStream = hstream;
      	shdri.pBuffer = temp->pBuffptr;
      	shdri.ulStreamTime = GetCurrentTime();
      	pfnSHD(&shdri);
      	delete temp;
   }
}

//
// ReturnBuffers(void)
// Return all buffers to MMPM.
//
void STREAM::ReturnBuffers(void)
{
   // move all buffers from the InProcess Queue to the Done Queue
   while (qhInProcess.IsElements()) {
      	qhDone.PushOnTail(qhInProcess.PopHead());
   }
   // Return all the buffers on the Done Queue
   while (qhDone.IsElements()) {
      	ReturnBuffer();
   }
}
//
// ProcessEvents
// called by the Process at interrupt time to see if there are
// any events that have timed out
//
void STREAM::ProcessEvents(void)
{
//SvL: BUGFIX: check all events
#if 1
   if (qhEvent.IsElements()) {
        PEVENT pnextevent = (PEVENT)qhEvent.Head();
      	ULONG  time = GetCurrentTime();
        while(pnextevent) {
	      	ULONG eventtime = pnextevent->GetEventTime();
	      	if (eventtime <= time)
	         	pnextevent->Report(time);
		pnextevent = (PEVENT)pnextevent->pNext;
	}
   }
#else
   if (qhEvent.IsElements()) {
      PEVENT pnextevent = (PEVENT)qhEvent.Head();
      ULONG time = GetCurrentTime();
      ULONG eventtime = pnextevent->GetEventTime();
      if (eventtime <= time)
         pnextevent->Report(time);
   }
#endif
}

ULONG STREAM::EnableEvent(PDDCMDCONTROL pControl)
{

   // see if the event already exists on the event queue
   // call FindEvent if we get back an address (event exists)
   // call the UpdateEvent member function and get the event info updated.
   // if Findevent returns NULL (no event on queue) then call the construct
   // a new event and put it on the tail of the event queue. then call
   // SetNextEvent to update the next event to time out....

   PEVENT pevent = FindEvent(pControl->hEvent, &qhEvent);
   if (pevent)
      pevent->UpdateEvent(this,pControl->hEvent,(PCONTROL_PARM)pControl->pParm);
   else {
      pevent= new EVENT(this,pControl->hEvent,(PCONTROL_PARM)pControl->pParm);
   }
   if (!pevent)
      return ERROR_TOO_MANY_EVENTS;

   SetNextEvent();
   return NO_ERROR;
}

ULONG STREAM::DisableEvent(PDDCMDCONTROL pControl)
{
   PEVENT pevent = FindEvent(pControl->hEvent, &qhEvent);
   if (!pevent)
      return ERROR_INVALID_EVENT;

    // destroying an event may change things that get referenced in the ISR
    // so we really need to cli/sti around the call to DestroyElement
   cli();
   qhEvent.DestroyElement((PQUEUEELEMENT)pevent);
   if (qhEvent.Head() != qhEvent.Tail())
      SetNextEvent();
   sti();
   return NO_ERROR;
}

ULONG STREAM::PauseStreamTime(void)
{
   fIncrementCounter = FALSE;
   return NO_ERROR;
}

ULONG STREAM::ResumeStreamTime(void)
{

   fIncrementCounter = TRUE;
   return NO_ERROR;
}



ULONG STREAM::Register(PDDCMDREGISTER p)
{
   hstream = p->hStream;
   pfnSHD = (PFN_SHD) p->pSHDEntryPoint;
   p->ulAddressType = ADDRESS_TYPE_LINEAR;
   p->mmtimePerUnit = 0;
   p->ulBytesPerUnit = 0;
   p->ulNumBufs = 0x00000010;
   if (ulStreamType & 0xFFFFFF60) // if this is a midi stream
      p->ulBufSize = 0x00000200;
   else
      p->ulBufSize = 0x00004000;
   return 0;
}


void STREAM::DeRegister(void)
{
   hstream = 0;
   pfnSHD = NULL;
}


#pragma off (unreferenced)
virtual ULONG STREAM::Write(PSTREAMBUF pbuf, ULONG uLength, BOOL fLooping)
#pragma on (unreferenced)
{
   qhInProcess.PushOnTail((PQUEUEELEMENT)new STREAMBUFFER(uLength, pbuf));
   return 0;
}

/**@internal SetNextEvent
 * @param    None
 * @return   None
 * @notes
 * the function walks the event list and finds the next event to timeout.
 * the event is moved to the head of the event queue.
 *
 */
void STREAM::SetNextEvent(void)
{

   // if there are no events or only one event on the
   // queue just return
   if ((qhEvent.Head()) == (qhEvent.Tail()))
       return;

   PQUEUEELEMENT pele1 = qhEvent.Head();
   PQUEUEELEMENT pele2 = NULL;
   ULONG ulTimeToBeat = -1;     // -1 equals 0xFFFFFFFF the maximum time

   while (pele1) {
      if (((PEVENT)pele1)->GetEventTime() <= ulTimeToBeat) {
         pele2 = pele1;
         ulTimeToBeat = ((PEVENT)pele1)->GetEventTime();
      }
      pele1 = pele1->pNext;
   }
   // pele2 should now contain the address of the next
   // event to time out.. if it is not already on
   // the head of the Event queue then put it there
   if (pele2 != qhEvent.Head()) {
      cli();
      qhEvent.PopElement(pele2);
      qhEvent.PushOnHead(pele2);
      sti();
   }
}
#define INVALID_HSTREAM  ((HSTREAM) 0)
#define INVALID_HFILE    ((ULONG)   0)

STREAM::STREAM(ULONG streamtype, USHORT filesysnum)
{
   // put this stream on the stream list
   pStreamList->PushOnTail(this);

   // get the pointer to the hardware object
   pahw = GetHardwareDevice(streamtype);
   ulStreamType = streamtype;

   hstream = INVALID_HSTREAM;     // We're putting a stream into the stream list, but
   ulSysFileNum = filesysnum;

   fIncrementCounter = TRUE;
   ulCurrentTime = 0;
   ulStreamState = STREAM_STOPPED;

   ulStreamId = 0;

   balance    = 50; //middle
   volume     = 80;
   inputgain  = 50;
   inputsrc   = MIX_RECSRC_LINE;
}

STREAM::~STREAM(void)
{
   if (ulStreamState == STREAM_STREAMING)
      pahw->Stop(this);
   // detstoy all the STREAMBUFFERs and EVENTs that this STREAM
   // may still have

   while (qhInProcess.IsElements()) {
      qhInProcess.DestroyElement(qhInProcess.Head());
   } /* endwhile */

   while (qhDone.IsElements()) {
      qhDone.DestroyElement(qhDone.Head());
   } /* endwhile */

   while (qhEvent.IsElements()) {
      qhEvent.DestroyElement(qhEvent.Head());
   } /* endwhile */

  pStreamList->PopElement(this);
}

#pragma off (unreferenced)
void STREAM::SetLooping(BOOL fLooping)
#pragma on (unreferenced)
{
   return;
}

#pragma off (unreferenced)
BOOL STREAM::SetProperty(int type, ULONG value, ULONG reserved)
#pragma on (unreferenced)
{
   switch(type) {
   case PROPERTY_VOLUME:
       volume = value;
       break;

   case PROPERTY_BALANCE:
       balance = value;
       break;

   case PROPERTY_MASTERVOL:
       if(mastervol != value) {
           mastervol = value;
           return OSS16_SetMasterVol(this, mastervol);
       }
       break;
   default:
       return FALSE;
   }
   return TRUE;
}

ULONG STREAM::GetProperty(int type) 
{
   switch(type) {
   case PROPERTY_FREQUENCY:
   case PROPERTY_LOOPING:
       break;

   case PROPERTY_VOLUME:
       return volume;

   case PROPERTY_BALANCE:
       return balance;

   case PROPERTY_MASTERVOL:
       return mastervol;
   }
   return -1;
}

ULONG STREAM::mastervol = MAKE_VOLUME_LR(100, 100);

PSTREAM FindActiveStream(ULONG StreamType)
{
   PSTREAM pStream = (PSTREAM) pStreamList->Head();

   while (pStream)
      if ((pStream->ulStreamType == StreamType) &&
          (pStream->ulStreamState == STREAM_STREAMING))
         return pStream;
      else
         pStream = (PSTREAM) pStream->pNext;

   return NULL;
}

PSTREAM FindActiveStream(ULONG StreamType, ULONG StreamId)
{
   PSTREAM pStream = (PSTREAM) pStreamList->Head();

   while (pStream)
      if ((pStream->ulStreamType == StreamType) &&
          (pStream->ulStreamState == STREAM_STREAMING) && (pStream->ulStreamId == StreamId))
         return pStream;
      else
         pStream = (PSTREAM) pStream->pNext;

   return NULL;
}

PSTREAM FindStream_fromFile(ULONG ulSysFileNum)
/* Map a system file handle to a Stream object.
 * ### FUTURE:  Make parm type a real type so we can use overloading,
 * ### and change name/signature back to "FindStream( fileHandle )".
 */
{
   PSTREAM pStream = (PSTREAM) pStreamList->Head();

   while (pStream)
      if (pStream->ulSysFileNum == ulSysFileNum)
         return pStream;
      else
         pStream = (PSTREAM) pStream->pNext;

   return NULL;
}

PSTREAM FindStream(HSTREAM hStream)
/* Map a stream handle to a Stream object. */
{
   PSTREAM pStream = (PSTREAM) pStreamList->Head();

   while (pStream)
      if (pStream->hstream == hStream)
         return pStream;
      else
         pStream = (PSTREAM) pStream->pNext;

   return NULL;
}
