/* $Id: strategy.h,v 1.1 2000/04/23 14:55:40 ktk Exp $ */

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
 * Defines, and prototypes for the strategy entry point and it's functions.
 * @version %I%
 * @context Unless otherwise noted, all interfaces are Ring-0, 16-bit,
 *  <stack context>.
 * @history
 *
 */
#ifndef STRATEGY_INCLUDED
#define STRATEGY_INCLUDED

#ifndef OS2_INCLUDED
#define INCL_NOPMAPI
#include <os2.h>
#endif

#pragma pack(1);

typedef struct {                           /* template for request header */
  BYTE bLength;                            /* request packet length */
  BYTE bUnit;                              /* unit code for block DD only */
  BYTE bCommand;                           /* command code */
  USHORT usStatus;                         /* return status */
  ULONG dwReserved;                        /* reserved bytes */
  ULONG ulQlink;                           /* queue linkage */
  union {                                  /* command-specific data */
    struct {
      BYTE b;
      PFN  ulDevHlp;                      /* dev help address */
      char __far *szArgs;                  /* argument pointer */
      BYTE bDrive;
    } init_in;
    struct {
      BYTE bUnits;
      USHORT usCodeEnd;                   // final code offset
      USHORT usDataEnd;                   // final data offset
      ULONG ul;
    } init_out;
    struct {
      BYTE bMedia;
      ULONG ulAddress;
      USHORT usCount;
      ULONG ulStartSector;
      USHORT usSysFileNum;
    } io;
    struct {
      BYTE bData;
    } peek;
    struct {
      BYTE bCategory;                     // category code
      BYTE bCode;                         // function code
      void __far *pvParm;                 // address of parameter buffer
      void __far *pvData;                 // address of data buffer
      USHORT usSysFileNum;                // system file number
      USHORT usPLength;                   // length of parameter buffer
      USHORT usDLength;                   // length of data buffer
    } ioctl;
    struct {
      USHORT usSysFileNum;                // system file number
    } open_close;
  } s;
} REQPACKET, __far *PREQPACKET;

#pragma pack();

/* Constants relating to the Strategy Routines
*/

#define RPDONE    0x0100         // return successful, must be set
#define RPBUSY    0x0200         // device is busy (or has no data to return)
#define RPDEV     0x4000         // user-defined error
#define RPERR     0x8000         // return error

// List of error codes, from chapter 8 of PDD reference
#define RPNOTREADY  0x0002
#define RPBADCMD    0x0003
#define RPGENFAIL   0x000c
#define RPDEVINUSE  0x0014
#define RPINITFAIL  0x0015

// list of Strategy commands from PDD reference
// Note this is only the list of commands audio device drivers care about
#define STRATEGY_INIT          0x00
#define STRATEGY_OPEN          0x0D
#define STRATEGY_CLOSE         0x0E
#define STRATEGY_GENIOCTL      0x10
#define STRATEGY_DEINSTALL     0x14
#define STRATEGY_INITCOMPLETE  0x1F

#endif
