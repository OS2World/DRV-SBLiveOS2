/* $Id: header.c,v 1.2 2001/03/23 17:14:13 sandervl Exp $ */

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
 * The Device Driver Header
 * @version %I%
 * @context Unless otherwise noted, all interfaces are Ring-0, 16-bit,
 *  <stack context>.
 * @history
 *
 */
#pragma code_seg ("_inittext");
#pragma data_seg ("_header","data");

#define INCL_NOPMAPI
#include <os2.h>

#include "header.h"

void __far StrategyHandler(void);
void __far StrategyHandler2(void);
ULONG __far __loadds __cdecl DDCMD_EntryPoint(void);

DEV_HEADER header[2] = {
   {  sizeof(DEV_HEADER),
      DA_CHAR | DA_IDCSET | DA_NEEDOPEN | DA_USESCAP,
      (PFNENTRY) StrategyHandler,
      (PFNENTRY) DDCMD_EntryPoint,
      {'S','B','L','I','V','E','1','$'},
      0,0,
      DC_INITCPLT | DC_IOCTL2 | DC_32BIT
   },
   {  -1,
      DA_CHAR | DA_IDCSET | DA_NEEDOPEN | DA_USESCAP,
      (PFNENTRY) StrategyHandler2,
      (PFNENTRY) 0,
      {'D','A','U','D','I','O','1','$'},
      0,0,
      DC_INITCPLT | DC_IOCTL2 | DC_32BIT
   }
};
