/* $Id: waveplay.cpp,v 1.1 2000/04/23 14:55:22 ktk Exp $ */

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
#include <os2medef.h>
#include <os2mixer.h>
#include <audio.h>
#include "wavehw.hpp"
#include "wavestrm.hpp"
#include "stream.hpp"
#include "irq.hpp"
#include "iodelay.h"
#include <include.h>

#include <ossidc.h>

#define MUTE_ON 0x0000028F
#define MUTE_OFF 0x00000000


/******************************************************************************/
/* WavePlay_Irq_Handler                                                       */
/* This is the ISR for the WAVEPLAY object. All ISRs must be Global in nature.*/
/* This routine will be called by the IRQ object when it receives an interrupt*/
/* From the OS/2 kernal and the WAVEPLAY object has previously registered this*/
/* handler, (by calling the IRQ's bAddHandler member function) and enabled    */
/* this handler. (by calling the IRQ's bEnableHandler member function)        */
/* Updates added for Defect 190876.                                           */
/*                                                                            */
/******************************************************************************/
BOOL __far __loadds __saveregs WavePlay_Irq_Handler(void)
{
   PWAVESTREAM pstream;

      // see if there is an active stream
      pstream = (PWAVESTREAM)FindActiveStream(STREAM_WAVE_PLAY);
      if (pstream) { // if there is an active stream
         pstream->Process();                  // call the active stream to
      }

      return (1);                           // return Happy to the IRQ
}

/******************************************************************************/
/* WAVEPLAY::Start(void)                                                      */
/*                                                                            */
/******************************************************************************/
virtual int WAVEPLAY::Start(STREAM *stream)
{
   return OSS16_StartStream(stream) != TRUE;
}

/******************************************************************************/
/* WAVEPLAY::Stop(void)                                                       */
/*                                                                            */
/******************************************************************************/
virtual int WAVEPLAY::Stop(STREAM *stream)
{
   return OSS16_StopStream(stream) != TRUE;
}

virtual void WAVEPLAY::ConfigDev(STREAM *stream, PWAVECONFIGINFO pConfigInfo)
{
 FORMAT_INFO formatInfo;

   WAVEAUDIO::ConfigDev(stream, pConfigInfo);

   formatInfo.ulSampleRate = pConfigInfo->ulSampleRate;
   formatInfo.ulBitsPerSample = pConfigInfo->ulBitsPerSample;
   formatInfo.ulNumChannels = pConfigInfo->ulNumChannels;
   formatInfo.ulDataType = pConfigInfo->ulDataType;
   OSS16_StreamSetFormat(stream, (ULONG)&formatInfo);
}

#pragma code_seg ("_inittext");
#pragma data_seg ("_initdata","endds");
/******************************************************************************/
/* WAVEPLAY::WAVEPLAY(USHORT PrimeDma, USHORT SecDma, USHORT Irq)             */
/* The constructor, basicly passes the resources info up the stream to the    */
/* "parent classes" and calles _vSetup which constructs all the resource      */
/* class objects we need to run.                                              */
/*                                                                            */
/******************************************************************************/

WAVEPLAY::WAVEPLAY():
   WAVEAUDIO(AUDIOHW_WAVE_PLAY)
{
   _vSetup();
}
