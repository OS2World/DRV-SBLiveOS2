/* $Id: strategy.cpp,v 1.2 2000/04/24 19:45:18 sandervl Exp $ */

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
 * The Strategy entry point lives here !!
 * @version %I%
 * @context Unless otherwise noted, all interfaces are Ring-0, 16-bit,
 *  kernel stack.
 * @history
 *
 */
extern "C" {               // 16-bit header files are not C++ aware
#define INCL_NOPMAPI
#define INCL_DOSINFOSEG
#include <os2.h>
#include <audio.h>
#include <os2medef.h>
}

#include <include.h>
#include <devhelp.h>
#include "strategy.h"
#include "rtmidi.hpp"                  // Object definition
#include "end.h"

// #include "ioctl.h"
#include "stream.hpp"
#include "idc_vdd.h"
#include <ossidc.h>
#include <dbgos2.h>
#include "ioctl.h"

#pragma off (unreferenced)

/*@external StrategyInitComplete
 * Perform initialization tasks that require Ring 0 context or IDC with other
 * drivers (InitComplete is called after all drivers are loaded).  We perform
 * RTMIDI registration here.
 */

void StrategyInitComplete(PREQPACKET prp)
{
   RTMIDI::vConnect();
   if(OSS16_AttachToPdd() == FALSE) {
        prp->usStatus = RPDONE | RPERR | RPGENFAIL;
	return;
   }
   MixerInit();
}

/* TROPEZ_StrategyOpen
 *
 * Purpose:  Worker rountine for the Strategy Open function.
 *
 * Notes:    The Strategy Open doesn't require allocation of any resources,
 *           and the function is treated as a NOP.  The first allocation of
 *           resources happens at Strategy IOCtl time, in the handler for
 *           the AudioInit IOCtl.
 *           Check to see if the VDD has claimed the hardware and return with
 *           the busy bit on the the request packett.
 *           Increment the InUseCount so we know that an OS/2 app has an
 *           instance open and we don't let the VDD take the hardware..
 *
 *
 *
 */
void StrategyOpen(PREQPACKET prp)
{
   if (usVDDHasHardware) {
      prp->usStatus = RPDONE | RPERR | RPBUSY;
   }
   else {
      ++usInUseCount;
   }
}

void StrategyClose(PREQPACKET prp)
{
   --usInUseCount;
   PSTREAM pstream = FindStream_fromFile( prp->s.open_close.usSysFileNum );
   if (pstream)
      delete pstream;
   else
      ;         //### Log unexpected condition.
}

void StrategyDeinstall(PREQPACKET prp)
{

   while (pAudioHWList->IsElements())
      pAudioHWList->DestroyElement(pAudioHWList->PopHead());
}

#pragma on (unreferenced)

void StrategyInit(PREQPACKET prp);
extern "C" void StrategyIoctl(PREQPACKET prp, USHORT LDev);

extern "C" void __far StrategyHandler(PREQPACKET prp);
#pragma aux StrategyHandler parm [es bx];

extern "C" void __far StrategyHandler(PREQPACKET prp)
{

   prp->usStatus = RPDONE;

   switch (prp->bCommand) {
      case STRATEGY_INIT:
         StrategyInit(prp);
         break;
      case STRATEGY_OPEN:
         StrategyOpen(prp);
         break;
      case STRATEGY_CLOSE:
         StrategyClose(prp);
         break;
      case STRATEGY_GENIOCTL:
         StrategyIoctl(prp, 0);
         break;
      case STRATEGY_DEINSTALL:
         StrategyDeinstall(prp);
         break;
      case STRATEGY_INITCOMPLETE:
         StrategyInitComplete(prp);
         break;
      default:
         prp->usStatus = RPDONE | RPERR | RPGENFAIL;
   }
}

extern "C" void __far StrategyHandler2(PREQPACKET prp);
#pragma aux StrategyHandler2 parm [es bx];

extern "C" void __far StrategyHandler2(PREQPACKET prp)
{

   prp->usStatus = RPDONE;

   switch (prp->bCommand) {
      case STRATEGY_INIT:
         prp->usStatus = RPDONE;
         prp->s.init_out.usCodeEnd = (USHORT) &end_of_text;
         prp->s.init_out.usDataEnd = (USHORT) &end_of_heap;
         break;
      case STRATEGY_OPEN:
         StrategyOpen(prp);
         break;
      case STRATEGY_CLOSE:
         StrategyClose(prp);
         break;
      case STRATEGY_GENIOCTL:
         StrategyIoctl(prp, 1);
         break;
      case STRATEGY_DEINSTALL:
         StrategyDeinstall(prp);
         break;
      case STRATEGY_INITCOMPLETE:
         prp->usStatus = RPDONE;
         break;
      default:
         prp->usStatus = RPDONE | RPERR | RPGENFAIL;
   }
}
