/* $Id: waudio.cpp,v 1.2 2001/04/30 21:07:59 sandervl Exp $ */

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
#include <audio.h>

#include "waudio.hpp"


//
//
// the following 3-D array defines the subtypes for DATATYPE_WAVEFORM
// The array is 4x2x2 and is indexed using frequency index, bits per
// sample being 8 or 16 represented by 0 or 1 resp. and mono or stereo
// mode represented by 0 or 1 respectively. For eg. to find out the
// subtype for 22050Hz sampling frequency, using 16 bits per sample
// and stereo mode, we use aaulWave[FREQ22KHZ][BPS16][1].  This avoids
// inefficient nested if's and switches to find a matching value given
// the same.
//
ULONG aaulWave[NUMFREQS][BPSTYPES][MONOSTEREO] = {
      WAVE_FORMAT_1M08,    /* 11.025kHz, 8-bit  Mono   */
      WAVE_FORMAT_1S08,    /* 11.025kHz, 8-bit  Stereo */
      WAVE_FORMAT_1M16,    /* 11.025kHz, 16-bit Mono   */
      WAVE_FORMAT_1S16,    /* 11.025kHz, 16-bit Stereo */
      WAVE_FORMAT_2M08,    /* 22.05kHz , 8-bit  Mono   */
      WAVE_FORMAT_2S08,    /* 22.05kHz , 8-bit  Stereo */
      WAVE_FORMAT_2M16,    /* 22.05kHz , 16-bit Mono   */
      WAVE_FORMAT_2S16,    /* 22.05kHz , 16-bit Stereo */
      WAVE_FORMAT_4M08,    /* 44.1kHz  , 8-bit  Mono   */
      WAVE_FORMAT_4S08,    /* 44.1kHz  , 8-bit  Stereo */
      WAVE_FORMAT_4M16,    /* 44.1kHz  , 16-bit Mono   */
      WAVE_FORMAT_4S16,    /* 44.1kHz  , 16-bit Stereo */
      WAVE_FORMAT_8M08,    /*  8.0kHz  , 8-bit  Mono   */
      WAVE_FORMAT_8S08,    /*  8.0kHz  , 8-bit  Stereo */
      WAVE_FORMAT_8M16,    /*  8.0kHz  , 16-bit Mono   */
      WAVE_FORMAT_8S16     /*  8.0kHz  , 16-bit Stereo */
      };

//
// the following 2-D array defines the subtypes for DATATYPE_ALAW
// it is indexed by the sampling rate ordinal (from
// _usfind_matching_sample_rate) and the number of channels
ULONG aaulAlaw[NUMFREQS][MONOSTEREO] = {
      ALAW_8B11KM,      /* 8bit 11kHz mono*/
      ALAW_8B11KS,      /* 8bit 11kHz stereo*/
      ALAW_8B22KM,      /* 8bit 22kHz mono*/
      ALAW_8B22KS,      /* 8bit 22kHz stereo*/
      ALAW_8B44KM,      /* 8bit 44kHz mono*/
      ALAW_8B44KS,      /* 8bit 44kHz stereo*/
      ALAW_8B8KM ,      /* 8bit 8kHz mono*/
      ALAW_8B8KS        /* 8bit 8kHz stereo*/
      };
//
// the following 2-D array defines the subtypes for DATATYPE_MULAW
// it is indexed by the sampling rate ordinal (from
// _usfind_matching_sample_rate) and the number of channels
ULONG aaulMulaw[NUMFREQS][MONOSTEREO] = {
      MULAW_8B11KM,     /* 8bit 11kHz mono*/
      MULAW_8B11KS,     /* 8bit 11kHz stereo*/
      MULAW_8B22KM,     /* 8bit 22kHz mono*/
      MULAW_8B22KS,     /* 8bit 22kHz stereo*/
      MULAW_8B44KM,     /* 8bit 44kHz mono*/
      MULAW_8B44KS,     /* 8bit 44kHz stereo*/
      MULAW_8B8KM ,     /* 8bit 8kHz mono*/
      MULAW_8B8KS       /* 8bit 8kHz stereo*/
      };

//
// The aulSuppSampleRates array is used by member function
// _usfind_matching_sample_rate to determine the sampling rate
// ordinal
ULONG  aulSuppSampleRates[] = {11025, 22050, 44100, 8000};

// the following macro only takes positive arguments and returns the
// absolute difference between the two arguments
#define distance(x, y) ((x>y)? x-y : y-x)

/**@internal _usfind_matching_sample_rate
 * @param    PULONG pulSampleRate
 * @return
 * @notes
 * The following function takes a sampling rate as input and finds out
 * the closest sampling rate in 11KHz, 22KHz, 44KHz and 8KHz.  It retuns
 * the ordinal value of the matching sampling rate which should be used
 * to reference an array element by the frequency index.
 * This function is called by the DevCaps member function.
 *
 */
USHORT WAVEAUDIO::_usfind_matching_sample_rate(PULONG pulSampleRate)
{
ULONG  ulMinDistance = 0x7fffffff;
USHORT usMatching;
USHORT us;

   for ( us = 0; us < 4; us++ ) {
      ULONG ulDistance = distance( *pulSampleRate, aulSuppSampleRates[us] );
      if (ulDistance < ulMinDistance) {
         ulMinDistance = ulDistance;
         usMatching = us;
      }
   }
   *pulSampleRate = aulSuppSampleRates[usMatching];
   return usMatching;
}

/**@internal
 * @param
 * @return
 * @notes
 *
 */
void WAVEAUDIO::DevCaps(PAUDIO_CAPS pCaps)
{
USHORT usSampleRateIndex;
ULONG  ulSampleRate;


   // This device driver supports Playback or Record Operations
   // anything else makes no sence. Note: This a per stream operation so
   // even if this sevice can do full-duplex, it can not do PLAY_AND_RECORD

   if ( pCaps->ulOperation != OPERATION_PLAY &&
        pCaps->ulOperation != OPERATION_RECORD ) {
      pCaps->ulSupport = UNSUPPORTED_OPERATION;
      return;
   }

   // Stereo or Mono only
   if (pCaps->ulChannels != 1 && pCaps->ulChannels != 2) {
      pCaps->ulSupport = UNSUPPORTED_CHANNELS;
      return;
   }

   // supported bits per sample are 8 (for unsigned PCM, u-law or A-law )
   // and 16 (for 2's complement PCM)
   if (pCaps->ulBitsPerSample != 8 && pCaps->ulBitsPerSample != 16) {
      pCaps->ulSupport = UNSUPPORTED_BPS;
      return;
   }

   ulSampleRate = pCaps->ulSamplingRate; //save the sampling rate called with

   // find out the closest sampling rate (may or may not be the same )
   // from one of the following: 11025Hz, 22050Hz, 44100Hz and 8000Hz
   // _usfind_matching_sample_rate will update  pCaps->ulSamplingRate if there
   // is not an exact match.
   usSampleRateIndex = _usfind_matching_sample_rate(&pCaps->ulSamplingRate);

   // If _usfind_matching_sample_rate changed the sampling rate set
   // the best fit flag.
   if (ulSampleRate != pCaps->ulSamplingRate)
      pCaps->ulFlags |= BESTFIT_PROVIDED;

   // Determine the ulDataSubType and update any format specific flags
   // Note: All data types have more than one value.
   switch ( pCaps->ulDataType ) {
      case DATATYPE_WAVEFORM:
      case PCM:
         // determine subtype for PCM:
         pCaps->ulDataSubType = aaulWave[usSampleRateIndex]
                                       [(pCaps->ulBitsPerSample-8)/8]
                                       [pCaps->ulChannels-1];
         if (pCaps->ulBitsPerSample == 16)
            pCaps->ulFlags |= TWOS_COMPLEMENT;       // 2's complement data
         break;

#if 0
//SvL: Does the SB live hardware support this or is it just a limitation of the linux driver?
      case DATATYPE_ALAW:
      case DATATYPE_RIFF_ALAW:
      case A_LAW:
         // determine subtype for A_LAW
         pCaps->ulDataSubType = aaulAlaw[usSampleRateIndex][pCaps->ulChannels-1];
         break;

      case DATATYPE_MULAW:
      case DATATYPE_RIFF_MULAW:
      case MU_LAW:
         // determine subtype for MU_LAW
         pCaps->ulDataSubType = aaulMulaw[usSampleRateIndex][pCaps->ulChannels-1];
         break;
#endif
      default:
         pCaps->ulSupport = UNSUPPORTED_DATATYPE;
         return;
   } // end switch

   pCaps->ulFlags = FIXED        |    // Fixed length data
                    LEFT_ALIGNED |    // Left align bits on byte bndry
                    BIG_ENDIAN   |    // MSB's first (motorola format)
                    INPUT        |    // Input select is supported
                    OUTPUT       |    // Output select is supported
                    MONITOR      |    // Monitor is supported
                    VOLUME;           // Volume control is supported

   // Full Duplex Enabling Stuff here !!
   // The number of resource units is described in the MMPM2.INI
   // This can be thought of the number of active streams the
   // driver can manage at one time. We list this number as 2.
   // we tell MMPM here how many of these units THIS stream will consume.
   // The answer is so simple it's brilliant, (Thanks to Joe Nord at Crystal
   // Semi) If we are enabled to do full-duplex this stream will consume 1
   // unit, If we are not enabled to do full-duplex this stream will consume 2
   // (or all the available units)
   // Along with the resource units, we defined 2 resources classes,
   // one for playback and one for capture. We tell MMPM (in the MMPM2.INI)
   // that we can handle doing 1 playback and 1 capture stream or 1 capture and
   // one playback stream at the same time. (Valid Resource combos in the
   // MMPM2.INI) check if we are a playback or capture and set the correct
   // resource class (Playback = 1, Capture = 2)

   pCaps->ulResourceUnits = 1;

   if ( pCaps->ulOperation == OPERATION_PLAY)
      pCaps->ulResourceClass = 1;
   else
      pCaps->ulResourceClass = 2;
   pCaps->fCanRecord = 1;              //  Yes Virgina we can record
   pCaps->ulBlockAlign = 1;            //  Block alignment for this mode

   //return success
   pCaps->ulSupport = SUPPORT_SUCCESS;

}
/**@internal
 * @param
 * @return
 * @notes
 *
 */
void WAVEAUDIO::DevCaps(LPDAUDIO_CAPS lpCaps)
{
    lpCaps->dwFlags                        = DAUDIOCAPS_PRIMARYMONO | 
                                             DAUDIOCAPS_PRIMARYSTEREO | 
                                             DAUDIOCAPS_PRIMARY8BIT |
                                             DAUDIOCAPS_PRIMARY16BIT | 
                                             DAUDIOCAPS_CONTINUOUSRATE | 
                                             DAUDIOCAPS_CERTIFIED |
                                             DAUDIOCAPS_SECONDARYMONO | 
                                             DAUDIOCAPS_SECONDARYSTEREO | 
                                             DAUDIOCAPS_SECONDARY8BIT |
                                             DAUDIOCAPS_SECONDARY16BIT;

    lpCaps->dwMinSecondarySampleRate       = SBLIVECAPS_MINSAMPLERATE;
    lpCaps->dwMaxSecondarySampleRate       = SBLIVECAPS_MAXSAMPLERATE;
    lpCaps->dwPrimaryBuffers               = 1;
    lpCaps->dwMaxHwMixingAllBuffers        = SBLIVECAPS_MAXSTREAMS;
    lpCaps->dwMaxHwMixingStaticBuffers     = SBLIVECAPS_MAXSTREAMS;
    lpCaps->dwMaxHwMixingStreamingBuffers  = SBLIVECAPS_MAXSTREAMS;
    lpCaps->dwMaxHw3DAllBuffers            = 0;
    lpCaps->dwMaxHw3DStaticBuffers         = 0;
    lpCaps->dwMaxHw3DStreamingBuffers      = 0;
    lpCaps->dwFreeHw3DAllBuffers           = 0;
    lpCaps->dwFreeHw3DStaticBuffers        = 0;
    lpCaps->dwFreeHw3DStreamingBuffers     = 0;
    lpCaps->dwTotalHwMemBytes              = 0;
    lpCaps->dwFreeHwMemBytes               = 0;
    lpCaps->dwMaxContigFreeHwMemBytes      = 0;
    lpCaps->dwUnlockTransferRateHwBuffers  = 0;
    lpCaps->dwPlayCpuOverheadSwBuffers     = 0;
    lpCaps->dwReserved1                    = 0;
    lpCaps->dwReserved2                    = 0;
}

/**@internal
 * @param
 * @return
 * @notes
 *
 */
#pragma off (unreferenced)
virtual void WAVEAUDIO::ConfigDev(STREAM *stream, PWAVECONFIGINFO pConfigInfo)
#pragma on (unreferenced)
{
   ULONG ulCount, ulConsumeRate, ulBytesPerIRQ;

   // Set the clock select bits (_ucClockData)

   // Set up _ucFormatData and write usSilence for the WAVESTREAM
   switch (pConfigInfo->ulDataType) {
      case DATATYPE_WAVEFORM:
      case PCM:
         if (pConfigInfo->ulBitsPerSample == 16) {
            pConfigInfo->usSilence = 0x0000;
         }
         else {
            pConfigInfo->usSilence = 0x8080;
         }
         break;

      case DATATYPE_ALAW:
      case DATATYPE_RIFF_ALAW:
      case A_LAW:
         pConfigInfo->usSilence = 0x5555;
         break;

      case DATATYPE_MULAW:
      case DATATYPE_RIFF_MULAW:
      case MU_LAW:
         pConfigInfo->usSilence = 0x7F7F;
         break;
   } /* endswitch */

   // Set the Stereo bit if necessary
//   if (pConfigInfo->ulNumChannels == 2)
//         _ucFormatData |= STEREO_BIT;

   // calculate the count register value: _usCountData
   // according to the CS4232 Spec the Upper and Lower Base registers are
   // loaded with the number of samples - 1 to be transfered between interrupts
   // (for all data formats except ADPCM)
   // For All Devices :
   // We want to generate 32 interrupts per second but if this requires that
   // we copy more than 0x0800 bytes of data per irq. Then we recalculate
   // If more that 0x0800 bytes is transfered per interrupt then there
   // may be problems with apps that use DART and stream small buffers

   ulCount = pConfigInfo->ulSampleRate;
   ulCount >>= 5;
   ulBytesPerIRQ = ulCount;
   _usCountData = (USHORT)(ulCount -1);

   // Calculate the BytesPerIRQ
   // The BytesPerIRQ is the number of bytes consumed by this data format
   // for every interrupt generated by the codec.
   // This inforamtion is returned to the WAVESTREAM which uses it in
   // buffer management decisions....

   if (pConfigInfo->ulBitsPerSample == 16)
      ulBytesPerIRQ <<= 1;
   if (pConfigInfo->ulNumChannels == 2)
      ulBytesPerIRQ <<= 1;
   pConfigInfo->ulBytesPerIRQ = ulBytesPerIRQ;

   // Check that we are at or below 0x800 bytes per irq
   // if not reclaculate based on 0x800 bytes per irq

   if (ulBytesPerIRQ > 0x00000800) {
      ulCount = 0x00000800;
      pConfigInfo->ulBytesPerIRQ = 0x00000800;
      if (pConfigInfo->ulBitsPerSample == 16)
         ulCount >>= 1;
      if (pConfigInfo->ulNumChannels == 2)
         ulCount >>= 1;
      _usCountData = (USHORT)(ulCount -1);
   }

   // Calculate the PCMConsumeRate
   // The consume rate is the number of bytes consumed by this data format
   // per second. It calculated by taking the following equation:
   //          sampling rate * (BitsPerSample/8) * NumChannels
   // This info is returned to the WAVESTREAM and used to calculate stream time

   ulConsumeRate = pConfigInfo->ulSampleRate;
   if (pConfigInfo->ulBitsPerSample == 16)
      ulConsumeRate <<= 1;
   if (pConfigInfo->ulNumChannels == 2)
      ulConsumeRate <<= 1;
   pConfigInfo->ulPCMConsumeRate = ulConsumeRate;

}
/**@internal Pause
 * @param    None
 * @return   int 1
 * @notes
 * stub function pause is implemented as a stop by the stream
 */
#pragma off (unreferenced)
int WAVEAUDIO::Pause(STREAM *stream)
#pragma on (unreferenced)
{
   return 1;
}
/**@internal Resume
 * @param    None
 * @return   int 1
 * @notes
 * stub function resume is implemented as a start by the stream
 */
#pragma off (unreferenced)
int WAVEAUDIO::Resume(STREAM *stream)
#pragma on (unreferenced)
{
   return 1;
}

#pragma code_seg ("_inittext");
#pragma data_seg ("_initdata","endds");

/**@internal
 * @param
 * @return
 * @notes
 *
 */
void WAVEAUDIO::_vSetup()
{
}
