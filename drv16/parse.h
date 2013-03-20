/* $Id: parse.h,v 1.2 2001/09/28 12:09:43 sandervl Exp $ */

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
 *  "externs" for DEVICE= command line parameters.
 * @version %I%
 * @context
 *  Unless otherwise noted, all interfaces are Ring-3, 16-bit,
 *  Init-time kernel stack.
 * @notes
 * @history
 */

#ifndef PARSE_INCLUDED
#define PARSE_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#define SIZE_CONFIG_LINE   256

// NUM_DEVICES defines the dimension of the arrays that receive values from
// the DEVICE= command line.
#define NUM_DEVICES  7

// The base I/O ports specified on the command line.
extern USHORT ausCL_BaseIO[];          // parse.c

// The IRQ levels specified on the command line
extern USHORT ausCL_IRQ[];             // parse.c

// The DMA Channels specified on the command line
extern USHORT ausCL_DMA[];             // parse.c

// The device header name specified on the command line
extern char szCL_DevName[];            // parse.c

// The DSP Ucode file name specified on the command line
extern char szCL_DSPUcodeName[];       // parse.c

// The size of the heap, in bytes
extern USHORT usCL_HeapSize;           // parse.c

// TRUE if /P and /I parameters were specified on the command line
extern int fParamsSpecified;

// True if the /V parameter was specified
extern int fVerbose;

// True if the hardware initialization errors should be ignore (/O:QUIETINIT)
extern int fQuietInit;

// True if we should not use HW timers.
extern int fNoHWTimer;

// The value of the /R parameter
//### extern int iCL_Resolution=2;
      extern int iCL_Resolution;       //### parse.c

// (### No:)  declared in strategy.c
// ### Moved to parse.c to solve linkage convents.
// True if /O:LONGNAMES specified
extern int fLongNames;

// True when /J was specified.
extern int fFMforMIDI;

// True when /O:Int3BeforeInit was specified.
extern int fInt3BeforeInit;

int GetParms(char __far *pszCmdLine);

extern int fMicMute;
extern int fLineMute;
extern int fCDMute;
extern int fAuxMute;

#ifdef __cplusplus
}
#endif

#endif
