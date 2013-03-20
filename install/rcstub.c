/* $Id: rcstub.c,v 1.2 2001/09/28 12:11:20 sandervl Exp $ */

/*static char *SCCSID = "@(#)rcstub.c   13.1 93/07/15";*/
/************************ START OF SPECIFICATIONS ***************************/
/*                                                                          */
/* SOURCE FILE NAME:  RCSTUB.C                                              */
/*                                                                          */
/* DESCRIPTIVE NAME:  Stub 'C' file used to build the CARDINFO.RC file      */
/*                    into CARDINFO.DLL.                                    */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/* COPYRIGHT    Copyright (C) 1992 IBM Corporation                          */
/*                                                                          */
/* The following IBM OS/2 2.1 source code is provided to you solely for     */
/* the purpose of assisting you in your development of OS/2 2.x device      */
/* drivers. You may use this code in accordance with the IBM License        */
/* Agreement provided in the IBM Device Driver Source Kit for OS/2. This    */
/* Copyright statement may not be removed.                                  */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/* STATUS:  DDK #3 1993                                                     */
/*                                                                          */
/* ENTRY POINT: RCSTUB                                                      */
/*                                                                          */
/*************************** END OF SPECIFICATIONS **************************/

#define		INCL_NOPMAPI
#define		INCL_NOBASEAPI
#include	<os2.h>

// We need to make sure, that we have some code *AND* some data.
// Otherwise, MINSTALL will crash. For the same reason, turn off
// WLINK option "Eliminate" !


UCHAR	uDummy = 0;


VOID APIENTRY RCSTUB(VOID)
{
}


