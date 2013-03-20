/* $Id: waudio.hpp,v 1.2 2001/05/09 17:44:25 sandervl Exp $ */

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
 * Defines, class definations and prototypes for
 * @version %I%
 * @context Unless otherwise noted, all interfaces are Ring-0, 16-bit,
 *  <stack context>.
 * @history
 *
 */
#ifndef WAUDIO_INCLUDED
#define WAUDIO_INCLUDED

#ifndef OS2_INCLUDED
#define INCL_NOPMAPI
#include <os2.h>
#include <os2medef.h>
#include <audio.h>
#endif

#include "audiohw.hpp"
#include "irq.hpp"

#include <daudio.h>

// defines used to define the PCM, MULAW, and ALAW tables used by
// virtual void DevCaps(PAUDIO_CAPS pCaps)
#define NUMFREQS   4
#define BPSTYPES   2
#define MONOSTEREO 2

#define DMA_BUFFER_SIZE (ULONG)0x4000

// FREQ_TABLE_TYPE class is specific to CS4232 it holds a particular sample
// rate and the calue that is required to be written into Clock select bits
// indexed register 8 bits 0-3) but this class can be easly update for other
// devices
class FREQ_TABLE_TYPE {
public:
   ULONG freq;
   UCHAR clock_select;
};
// The WaveConfigInfo Class
// Built the the WAVESTREAM class, at Audio Ioctl Init time, the WaveConfigInfo
// is used to suppply the WAVEAUDIO class with information about a particular
// instance and the WAVEAUDIO calss also returns information to the WAVESTREAM
// class it needs to calculate the stream time and keep the device quiet if
// the stream runs out of data.. All values supplyed by the WAVESTREAM are
// noted as Input and values returned by the WAVEAUDIO class are noted as
// Output.
class WaveConfigInfo {
public:
   ULONG  ulSampleRate;      // Samples Per Second Input
   ULONG  ulBitsPerSample;   // Number of Bits in a Sample Input
   ULONG  ulNumChannels;     // Number of Channels (mono or stereo) Input
   ULONG  ulDataType;        // type of data (PCM, MuLaw, ALaw etc) Input
   ULONG  ulPCMConsumeRate;  // number of bytes consumed/produced per sec Output
   ULONG  ulBytesPerIRQ;     // Number of bytes consumed/produced per IRQ Output
   USHORT usSilence;         // Value that produces silence Output
};
typedef WaveConfigInfo * PWAVECONFIGINFO;

class STREAM;

class WAVEAUDIO : public AUDIOHW {
public:
   virtual int Pause(STREAM *stream);    // Pause the operation
   virtual int Resume(STREAM *stream);   // Resume the operation

                               // Report the Device Capabilities to MMPM/2
                               // This member function is called from
                               // IoctlAudioInit() (IOCTL.CPP) and is the
                               // only MMPM/2 specific call made into the
                               // WAUDIO Class.
   virtual void DevCaps(PAUDIO_CAPS pCaps);
   virtual void DevCaps(LPDAUDIO_CAPS pCaps);
                               // configure the device for an operation
   virtual void ConfigDev(STREAM *stream, PWAVECONFIGINFO pConfigInfo);
 

protected:
WAVEAUDIO(ULONG devicetype) :
      AUDIOHW(devicetype)
      {};

   UCHAR  _ucClockData;   // The Clock Select Data on the CS4232 this data
                          // is written into bits 0-3 of the FS and Playback
                          // Data Format Reg (indexed reg 8)
   UCHAR  _ucFormatData;  // The Data Format bits on the CS4232 this corresponds
                          // to bits 4-7 indexed registers 8 (playback) and
                          // 28 (capture)
   USHORT _usCountData;   // The count register data (indexed registers 14 and
                          // 15 or 30 and 31)
   void   _vSetup(void);  // Common setup code called by both the WAVEPLAY and
                          // WAVEREC constructors

private:
   USHORT _usfind_matching_sample_rate(PULONG pulSampleRate);

};
typedef WAVEAUDIO *PWAVEAUDIO;

#endif
