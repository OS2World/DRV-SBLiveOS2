/* $Id: header.h,v 1.1 2000/04/23 14:55:40 ktk Exp $ */

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
 * Defines for the Device Driver Header
 * @version %I%
 * @context Unless otherwise noted, all interfaces are Ring-0, 16-bit,
 *  <stack context>.
 * @history
 *
 */
#ifndef HEADER_INCLUDED
#define HEADER_INCLUDED

#define DA_CHAR         0x8000   // Character PDD
#define DA_IDCSET       0x4000   // IDC entry point set
#define DA_BLOCK        0x2000   // Block device driver
#define DA_SHARED       0x1000   // Shared device
#define DA_NEEDOPEN     0x800    // Open/Close required

#define DA_OS2DRVR      0x0080   // Standard OS/2 driver
#define DA_IOCTL2       0x0100   // Supports IOCTL2
#define DA_USESCAP      0x0180   // Uses Capabilities bits

#define DA_CLOCK        8        // Clock device
#define DA_NULL         4        // NULL device
#define DA_STDOUT       2        // STDOUT device
#define DA_STDIN        1        // STDIN device

#define DC_INITCPLT     0x10     // Supports Init Complete
#define DC_ADD          8        // ADD driver
#define DC_PARALLEL     4        // Supports parallel ports
#define DC_32BIT        2        // Supports 32-bit addressing
#define DC_IOCTL2       1        // Supports DosDevIOCtl2 and Shutdown (1C)

typedef void (__near *PFNENTRY) (void);

#pragma pack(1);

typedef struct {
   unsigned long ulNextDD;
   unsigned short usAttribs;
   PFNENTRY pfnStrategy;
   PFNENTRY pfnIDC;
   char abName[8];
   unsigned long ulReserved[2];
   unsigned long ulCaps;
} DEV_HEADER;

#pragma pack();

// pseudo-variable that points to device header
#define phdr ((DEV_HEADER *) 0)

#endif
