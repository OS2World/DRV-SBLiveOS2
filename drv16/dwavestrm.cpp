/* $Id: dwavestrm.cpp,v 1.5 2001/04/30 21:07:57 sandervl Exp $ */

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
#include <os2.h>
#include <os2me.h>
#include <audio.h>

#include <devhelp.h>
#include <include.h>

#include "dwavestrm.hpp"
#include "memutil.h"
#include <dbgos2.h>
#include "ioctl.h"
#include "malloc.h"
#include <ossidc.h>

void cdecl HookHandler(ULONG ulSysFileNum)
{
    PDWAVESTREAM pStream;
    PSTREAMBUFFER temp;
    int rc;

    pStream = (PDWAVESTREAM)FindStream_fromFile(ulSysFileNum);
    if(pStream == NULL) {
        dprintf(("HookHandler stream %lx not found!", (ULONG) ulSysFileNum));
        DebugInt3();
        return;
    }

    temp = (PSTREAMBUFFER)pStream->qhReturn.PopHead();
    while(temp) {
        if(pStream->hSem) {
            rc = DevHelp_PostEventSem(pStream->hSem);
            if(rc != 0) {
                dprintf(("DevHlp_PostEventSem returned %d", rc));
            }
        }
        DevHelp_VMFree((LIN)temp->pBuffptr);
        DevHelp_VMUnLock(temp->linLock);
        delete temp;

        temp = (PSTREAMBUFFER)pStream->qhReturn.PopHead();
    }
    return;
}


ULONG DWAVESTREAM::Write(PSTREAMBUF pbuf, ULONG uLength, BOOL fLooping)
{
    PSTREAMBUFFER pStreamBuf;
    LIN             linAddr;
    PULONG          pLock;
    ULONG           PageListCount;
    LIN             linLock;
    int             rc;

    pStreamBuf = new STREAMBUFFER(uLength, (PSTREAMBUF)0, fLooping);
    if(pStreamBuf == NULL) {
        DebugInt3();
        return 1;
    }

    pLock = &pStreamBuf->lock[0];

    rc = DevHelp_VirtToLin(SELECTOROF(pLock), OFFSETOF(pLock), &linLock);
    if(rc) {
        DebugInt3();
        delete pStreamBuf;
        return rc;
    }

    rc = DevHelp_VMLock(VMDHL_LONG | VMDHL_WRITE, (LIN)pbuf, uLength, -1L, linLock, (PULONG)&PageListCount);
    if(rc) {
        DebugInt3();
        delete pStreamBuf;
        return rc;
    }

    rc = DevHelp_VMProcessToGlobal(VMDHGP_WRITE, (LIN)pbuf, uLength, (PLIN)&linAddr);
    if(rc) {
        DebugInt3();
        DevHelp_VMUnLock(linLock);
        delete pStreamBuf;
        return rc;
    }
    pStreamBuf->pBuffptr = (PSTREAMBUF)linAddr;
    pStreamBuf->linLock  = linLock;

    return WAVESTREAM::Write((PSTREAMBUFFER)pStreamBuf);
}

void DWAVESTREAM::ReturnBuffer(void)
{
   PSTREAMBUFFER temp = (PSTREAMBUFFER)qhDone.PopHead();

   if(temp)
   {
        if(ulStreamState == STREAM_STREAMING) {//means we're called during an interrupt
            qhReturn.PushOnTail((PQUEUEELEMENT)temp);
            DevHelp_ArmCtxHook(ulSysFileNum, hCtxHook);
        }
        else {
            DevHelp_VMFree((LIN)temp->pBuffptr);
            DevHelp_VMUnLock(temp->linLock);
            delete temp;
        }
   }
}

ULONG DWAVESTREAM::Register(PDDCMDREGISTER pReg)
{
    hSem = pReg->hStream;

    if(DevHelp_OpenEventSem(hSem) != 0) {
        dprintf(("DevHlp_OpenEventSem %lx failed!", hSem));
        hSem = 0;
        return 1;
    }
    return WAVESTREAM::Register(pReg);
}

void DWAVESTREAM::DeRegister(void)
{
    if(DevHelp_CloseEventSem(hSem) != 0) {
        dprintf(("DevHlp_CloseEventSemaphore %lx failed!", hSem));
        return;
    }
    hSem = 0;
    WAVESTREAM::DeRegister();
}

//
//
ULONG DWAVESTREAM::StartStream(void)
{
   return WAVESTREAM::StartStream();
}


void DWAVESTREAM::AddBuffers(BOOL fFirst)
{
   PSTREAMBUFFER pTemp = (PSTREAMBUFFER)qhDone.Tail();
   ULONG space, Buff_left, byteswritten;

   if(!pTemp) pTemp = (PSTREAMBUFFER)qhInProcess.Tail();

   if(ulStreamType & STREAM_WRITE && pTemp && pTemp->looping) 
   {
        Buff_left = pTemp->ulBuffsz - pTemp->ulBuffpos;

	space = OSS16_StreamGetSpace(this);
        if(fFirst) {
             space = min(space, 4*fragsize);
        }
        else space = min(space, fragsize);

	if(space) {
            if(space >= Buff_left) {
                 byteswritten = AddBuffer(Buff_left);
                 if(byteswritten == Buff_left) {
                     pTemp->ulBuffpos = 0; //reset fill position
                     AddBuffer(space - Buff_left);
                 }
            }
            else AddBuffer(space);
	}	
        pTemp->ulDonepos = 0; //make sure it ::Process never thinks it's done
   }
   else WAVESTREAM::AddBuffers(fFirst);
}

BOOL DWAVESTREAM::SetProperty(int type, ULONG value, ULONG reserved)
{
   switch(type) {
   case PROPERTY_LOOPING:
   {
       cli();
       PSTREAMBUFFER pTemp = (PSTREAMBUFFER)qhInProcess.Head();

       if(!pTemp) pTemp = (PSTREAMBUFFER)qhDone.Head();

       if(pTemp) {
           pTemp->looping = (BOOL)value;
           if(pTemp->looping == FALSE) {
               //calculate current play position
               pTemp->ulDonepos = (_ulBytesProcessed % pTemp->ulBuffsz);
           }
       }
       sti();
   }
   case PROPERTY_FREQUENCY:
       break;

   default:
       return WAVESTREAM::SetProperty(type, value, reserved);

   }
   return TRUE;
}

ULONG DWAVESTREAM::GetProperty(int type) 
{
   switch(type) {
   case PROPERTY_LOOPING:
   {
       cli();
       PSTREAMBUFFER pTemp = (PSTREAMBUFFER)qhInProcess.Head();
       ULONG         ret = FALSE;

       if(!pTemp) pTemp = (PSTREAMBUFFER)qhDone.Head();

       if(pTemp) {
           ret = pTemp->looping;
       }
       sti();
       return ret;
   }

   default:
       return WAVESTREAM::GetProperty(type);
   }
}

DWAVESTREAM::DWAVESTREAM(ULONG streamtype, LPMCI_AUDIO_INIT pinit, USHORT filesysnum):
   WAVESTREAM(streamtype, pinit, filesysnum), fError(FALSE), hCtxHook(0), hSem(0)
{
    if(DevHelp_AllocateCtxHook((NPFN)HookHandlerAsm, &hCtxHook)) {
        DebugInt3();
        fError = TRUE;
    }
}

DWAVESTREAM::~DWAVESTREAM()
{
    if (ulStreamState == STREAM_STREAMING) {
         CONTROL_PARM cParm;
         StopStream(&cParm);
    }
    else ReturnBuffers();

    if(hSem) {
        APIRET rc = DevHelp_PostEventSem(hSem);
        if(rc != 0) {
            dprintf(("DevHlp_PostEventSem returned %d", rc));
        }
        if(DevHelp_CloseEventSem(hSem) != 0) {
            dprintf(("DevHlp_CloseEventSemaphore %lx failed!", hSem));
        }
    }
    if(hCtxHook) {
        DevHelp_FreeCtxHook(hCtxHook);
    }
}

