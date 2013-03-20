/* $Id: ssm_idc.cpp,v 1.1 2000/04/23 14:55:20 ktk Exp $ */

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

#include "midistrm.hpp"
#include "event.hpp"
#include <include.h>


extern "C" ULONG __far __loadds __cdecl DDCMD_EntryPoint(PDDCMDCOMMON pCommon)
{
   ULONG ulReturnCode = NO_ERROR;
   PSTREAM pstream=FindStream(pCommon->hStream);

   switch (pCommon->ulFunction) {

      case DDCMD_SETUP: {
         if (!pstream)
            return ERROR_INVALID_STREAM;
         PDDCMDSETUP p = (PDDCMDSETUP) pCommon;
         SETUP_PARM __far *psp = (SETUP_PARM __far *) p->pSetupParm;
         pstream->SetCurrentTime(psp->ulStreamTime);

           // if there is a flags field in the SETUP_PARM
           // the tell MMPM it can send us 'RECURRING' events....
         if (p->ulSetupParmSize > sizeof(ULONG)) {
            psp->ulFlags = SETUP_RECURRING_EVENTS;
         }
         break;
      }
      case DDCMD_READ: {
         PDDCMDREADWRITE p=(PDDCMDREADWRITE) pCommon;


         if (!pstream)
            return ERROR_INVALID_STREAM;
         ulReturnCode =
            pstream->Read((PSTREAMBUF) p->pBuffer,(unsigned) p->ulBufferSize);
         if (ulReturnCode)
            return ulReturnCode;
         break;
      }
      case DDCMD_WRITE: {
         PDDCMDREADWRITE p=(PDDCMDREADWRITE) pCommon;

         if (!pstream)
            return ERROR_INVALID_STREAM;
         ulReturnCode =
            pstream->Write((PSTREAMBUF) p->pBuffer,(unsigned) p->ulBufferSize);
         if (ulReturnCode)
            return ulReturnCode;
         break;
      }
      case DDCMD_STATUS: {

         PDDCMDSTATUS p = (PDDCMDSTATUS) pCommon;
         PSTATUS_PARM p2 = (PSTATUS_PARM) p->pStatus;
         if (!pstream)
           return ERROR_INVALID_STREAM;
         p2->ulTime = pstream->GetCurrentTime();
         break;
      }
      case DDCMD_CONTROL: {
         PDDCMDCONTROL p = (PDDCMDCONTROL) pCommon;
         if (!pstream)
            return ERROR_INVALID_STREAM;

         switch (p->ulCmd) {
         case DDCMD_START:
            return pstream->StartStream();
         case DDCMD_STOP:
            p->ulParmSize=sizeof(ULONG);
            return pstream->StopStream((PCONTROL_PARM)p->pParm);
         case DDCMD_PAUSE:
            p->ulParmSize=sizeof(ULONG);
            return pstream->PauseStream((PCONTROL_PARM)p->pParm);
         case DDCMD_RESUME:
            return pstream->ResumeStream();
         case DDCMD_ENABLE_EVENT:
            return pstream->EnableEvent(p);
         case DDCMD_DISABLE_EVENT:
            return pstream->DisableEvent(p);
         case DDCMD_PAUSE_TIME:
            return pstream->PauseStreamTime();
         case DDCMD_RESUME_TIME:
            return pstream->ResumeStreamTime();
         default:
            return ERROR_INVALID_REQUEST;
         } /* endswitch */
      }
      case DDCMD_REG_STREAM: {
         if (pstream)
            return ERROR_HNDLR_REGISTERED;
         pstream = FindStream_fromFile(((PDDCMDREGISTER) pCommon)->ulSysFileNum);
         if (!pstream)
            return ERROR_STREAM_CREATION;
         ulReturnCode = pstream->Register((PDDCMDREGISTER) pCommon);
         if (ulReturnCode)
            return ERROR_STREAM_CREATION;
         break;
      }
      case DDCMD_DEREG_STREAM:
         if (!pstream)
            return ERROR_INVALID_STREAM;
         pstream->DeRegister();
         break;
      default:
         return ERROR_INVALID_FUNCTION;
   }

   return NO_ERROR;
}
