/* $Id: init.cpp,v 1.8 2001/09/28 12:09:42 sandervl Exp $ */

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
 *  Top level device driver initialization.
 * @version %I%
 * @context
 *  Physical DD initialization context:  Ring3 with I/O privledge.
 *  Discarded after use.
 * @notes
 *  Creates resource manager object and uses that RM object to figure out
 *  whether audio hardware is present.  If present, RM provides IO resources
 *  to operate hardware.  This modules uses that information to create
 *  Audio HW objects and other global data structures.
 * @history
 */

#pragma code_seg ("_inittext");
////#pragma data_seg ("_initdata","endds");

extern "C" {               // 16-bit header files are not C++ aware
#define INCL_NOPMAPI
#define INCL_DOSMISC
#include <os2.h>
#include <audio.h>
#include <os2mixer.h>
}

#include <ctype.h>
#include <string.h>
#include <devhelp.h>
#include "strategy.h"                  // OS/2 DD strategy interface
#include "header.h"                    // Device driver header
#include "parse.h"                     // Parses DEVICE= command line
#include "malloc.h"                    // Heap memory used by this driver
#include "mpu401.hpp"                  // Object definition
#include "wavehw.hpp"                  // Object definition
#include "irq.hpp"                     // Object definition
#include "timer.hpp"                   // Object definition
#include "stream.hpp"                  // pStreamList
#include "rm.hpp"                      // Object definition
#include "end.h"                       // end_of_data, end_of_text()
#include "sizedefs.h"                  // HEAP_SIZE
#include "idc_vdd.h"                   // VDD interface and Data Structs
#include <include.h>                   // Pragmas and more.
#include <sbversion.h>
#include "commdbg.h"
#include <dbgos2.h>

#ifndef PCI_VENDOR_ID_CREATIVE
#define PCI_VENDOR_ID_CREATIVE 0x1102UL
#endif

#ifndef PCI_DEVICE_ID_CREATIVE_EMU10K1
#define PCI_DEVICE_ID_CREATIVE_EMU10K1 0x0002UL
#endif

#ifndef PCI_DEVICE_ID_CREATIVE_EMU10K1_JOYSTICK
#define PCI_DEVICE_ID_CREATIVE_EMU10K1_JOYSTICK 0x7002
#endif

#define PCI_ID 	   ((PCI_DEVICE_ID_CREATIVE_EMU10K1<<16UL)|PCI_VENDOR_ID_CREATIVE)
#define PCI_IDJOY  ((PCI_DEVICE_ID_CREATIVE_EMU10K1_JOYSTICK<<16UL)|PCI_VENDOR_ID_CREATIVE)

// Default MIDI timer interval, milliseconds.
static USHORT MIDI_TimerInterval = 10;

static char szSBLive[]    = "SoundBlaster Live! MMPM/2 Audio Driver v"SBLIVE_VERSION;
static char szCopyRight[] = "Copyright 2000-2001 Sander van Leeuwen (sandervl@xs4all.nl)";
static char NEWLINE[]     = "\r\n";
static char szSBLiveNotFound[] = "SB Live! hardware not detected!";
static char szSBLiveAllocResFailed1[] = "Another device driver was granted exclusive access to IRQ ";
static char szSBLiveAllocResFailed2[] = "Unable to allocate hardware resources! Aborting...";
static char szSBLiveConfig1[]   = "SB Live! configuration: IRQ ";
static char szSBLiveConfig2[]   = ", IO Port 0x";
static char szSBLiveConfigJoy[] = "SB Live! Joystick     : IO Port 0x";

//
// ASCII Z-String used to register for PDD-VDD IDC must
// Name is copied from header at runtime.
//
char  szPddName[9]  = {0};
char  digit[16]     = {0};

ResourceManager* pRM = 0;               // Resource manager object.

//
// Strategy Init
//
void StrategyInit(PREQPACKET prp)
{
   Device_Help = (PFN) prp->s.init_in.ulDevHlp;
   prp->s.init_out.usCodeEnd = 0;
   prp->s.init_out.usDataEnd = 0;
   prp->usStatus = RPDONE | RPERR | RPGENFAIL;

   /* Initialize Heap. */
   unsigned uInitSize;                // Actual size of Heap following initialization.
   uInitSize = HeapInit( HEAP_SIZE );
   if ((HEAP_SIZE-uInitSize) > 64) {  // Should get size of request, less some overhead.
      // ### pError->vLog( HeapInitFailure );       //### Error log 'pError' not yet created.
      return;
   }
   /* Heap initialized. */

   // Initialize global lists.
   pAudioHWList = new QUEUEHEAD;
   pStreamList = new QUEUEHEAD;
   pTimerList = new QUEUEHEAD;

   // Fetch command line parameters.
   if (!GetParms(prp->s.init_in.szArgs)) {
      return;
   }

   // Now that we've grabbed our cmd line options,
   // we can start processing stuff that depends on those switches.

   if (fInt3BeforeInit) DebugInt3();

   // If we got a /V (verbose) flag on the DEVICE= command, route messages
   // to the display.
   if (fVerbose) {
	USHORT result;

	DosWrite(1, (VOID FAR*)NEWLINE, sizeof(NEWLINE)-1, &result);
	DosWrite(1, (VOID FAR*)szSBLive, sizeof(szSBLive)-1, &result);
	DosWrite(1, (VOID FAR*)NEWLINE, sizeof(NEWLINE)-1, &result);

	DosWrite(1, (VOID FAR*)szCopyRight, sizeof(szCopyRight), &result);
	DosWrite(1, (VOID FAR*)NEWLINE, sizeof(NEWLINE)-1, &result);
	DosWrite(1, (VOID FAR*)NEWLINE, sizeof(NEWLINE)-1, &result);
   }

   if (szCL_DevName[0] != ' ')               // Was a valid device name specified?
      memcpy(phdr->abName,szCL_DevName,8);   // yes, copy it to the dev header

   pRM = new ResourceManager(PCI_ID);        // Create the RM object.
   if (! pRM) {
      return;
   }

   //SvL: Check if SB Live hardware has been detected by the resource manager
   if(pRM->getState() != rmDriverCreated || !pRM->bIsDevDetected(PCI_ID, SEARCH_ID_DEVICEID, TRUE)) 
   {
hardware_notfound:
	USHORT result;

	DosWrite(1, (VOID FAR*)szSBLiveNotFound, sizeof(szSBLiveNotFound)-1, &result);
	DosWrite(1, (VOID FAR*)NEWLINE, sizeof(NEWLINE)-1, &result);
	DosWrite(1, (VOID FAR*)NEWLINE, sizeof(NEWLINE)-1, &result);
	return;
   }
   LDev_Resources *pResources = pRM->pGetDevResources(PCI_ID, SEARCH_ID_DEVICEID, TRUE);
   if ((!pResources) || pResources->isEmpty()) {
   	goto hardware_notfound;
   }
   if (pRM->getState() == rmAllocFailed) {
	USHORT result;

	DosWrite(1, (VOID FAR*)szSBLiveAllocResFailed1, sizeof(szSBLiveAllocResFailed1)-1, &result);
	DecWordToASCII(digit, (USHORT)pResources->uIRQLevel[0], 0);
	DosWrite(1, (VOID FAR*)digit, strlen(digit), &result);
	DosWrite(1, (VOID FAR*)NEWLINE, sizeof(NEWLINE)-1, &result);
	DosWrite(1, (VOID FAR*)szSBLiveAllocResFailed2, sizeof(szSBLiveAllocResFailed2)-1, &result);
	DosWrite(1, (VOID FAR*)NEWLINE, sizeof(NEWLINE)-1, &result);
	DosWrite(1, (VOID FAR*)NEWLINE, sizeof(NEWLINE)-1, &result);
	return;
   }

   if (fVerbose) {
	USHORT result;

	DecWordToASCII(digit, (USHORT)pResources->uIRQLevel[0], 0);
	DosWrite(1, (VOID FAR*)szSBLiveConfig1, sizeof(szSBLiveConfig1)-1, &result);
	DosWrite(1, (VOID FAR*)digit, strlen(digit), &result);

	DosWrite(1, (VOID FAR*)szSBLiveConfig2, sizeof(szSBLiveConfig2)-1, &result);
	HexWordToASCII(digit, pResources->uIOBase[0], 0);
	DosWrite(1, (VOID FAR*)digit, strlen(digit), &result);
   }
   delete pResources;

   //Joystick detection
   if(pRM->bIsDevDetected(PCI_IDJOY, SEARCH_ID_DEVICEID, TRUE)) 
   {
       pResources = pRM->pGetDevResources(PCI_IDJOY, SEARCH_ID_DEVICEID, TRUE, TRUE);
       if((pResources) && !pResources->isEmpty() && fVerbose) {
           USHORT result;
  	   DosWrite(1, (VOID FAR*)NEWLINE, sizeof(NEWLINE)-1, &result);
	   DosWrite(1, (VOID FAR*)szSBLiveConfigJoy, sizeof(szSBLiveConfigJoy)-1, &result);
	   HexWordToASCII(digit, pResources->uIOBase[0], 0);
	   DosWrite(1, (VOID FAR*)digit, strlen(digit), &result);
       }
       delete pResources;
   }
   if (fVerbose) {
	USHORT result;
	DosWrite(1, (VOID FAR*)NEWLINE, sizeof(NEWLINE)-1, &result);
	DosWrite(1, (VOID FAR*)NEWLINE, sizeof(NEWLINE)-1, &result);
   }

   // Build the MPU401 object only if we got a good 2115 init.
   // First create a timer, then the HW object.
   TIMER* pMPUTimer =
       new TIMER(NULL, MIDI_TimerInterval, STREAM_MPU401_PLAY );
   if (pMPUTimer->eState() != TIMER_Disabled)
         new MPU_401(pMPUTimer);

   SetHardwareType(AUDIOHW_MPU401_PLAY, MIDI, OPERATION_PLAY, 0);
   SetHardwareType(AUDIOHW_MPU401_PLAY, DATATYPE_MIDI, OPERATION_PLAY, 0);

////   SetHardwareType(AUDIOHW_MPU401_PLAY,             0, OPERATION_PLAY, 0); //### Must be fixed.
   // DART on Warp3
   SetHardwareType(AUDIOHW_WAVE_PLAY, DATATYPE_NULL, OPERATION_PLAY, 0);
   SetHardwareType(AUDIOHW_WAVE_CAPTURE, DATATYPE_NULL, OPERATION_RECORD, 0);

   // Build the Wave Playback Hardware object
   new WAVEPLAY();
   new WAVEREC();

#if 0
   // fill in the ADAPTERINFO
   codec_info.ulNumPorts = NUMIORANGES;
   codec_info.Range[0].ulPort  =  pResourcesWSS->uIOBase[0];
   codec_info.Range[0].ulRange =  pResourcesWSS->uIOLength[0];
   codec_info.Range[1].ulPort  =  pResourcesWSS->uIOBase[1];
   codec_info.Range[1].ulRange =  pResourcesWSS->uIOLength[1];
   codec_info.Range[2].ulPort  =  pResourcesWSS->uIOBase[2];
   codec_info.Range[2].ulRange =  pResourcesWSS->uIOLength[2];
   codec_info.Range[3].ulPort  =  pResourcesICS->uIOBase[0];
   codec_info.Range[3].ulRange =  pResourcesICS->uIOLength[0];

   // set up the addressing to the codec data for the vdd
   pfcodec_info = (ADAPTERINFO __far *)&codec_info;
   DevHelp_VirtToLin (SELECTOROF(pfcodec_info), (ULONG)(OFFSETOF(pfcodec_info)),
                     (PLIN)&pLincodec);

   // copy the pdd name out of the header.
   for (int i = 0; i < sizeof(szPddName)-1 ; i++) {
      if (phdr->abName[i] <= ' ')
         break;
      szPddName[i] = phdr->abName[i];
   }
   // register the VDD IDC entry point..
   DevHelp_RegisterPDD ((NPSZ)szPddName, (PFN)IDCEntry_VDD);
#endif

   prp->usStatus = RPDONE;
   prp->s.init_out.usCodeEnd = (USHORT) &end_of_text;
   prp->s.init_out.usDataEnd = (USHORT) &end_of_heap;
}
