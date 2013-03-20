/* $Id: wavestrm.hpp,v 1.5 2001/04/30 21:08:00 sandervl Exp $ */

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
#ifndef WAVESTREAM_INCLUDED
#define WAVESTREAM_INCLUDED

#ifndef OS2_INCLUDED
#define INCL_NOPMAPI
#include <os2.h>
#include <os2medef.h>
#include <audio.h>
#endif

#include "stream.hpp"
#include "waudio.hpp"
#include "strmbuff.hpp"

class WAVESTREAM : public STREAM {

public:
   void Process(void);           // called every timer interrupt
   virtual ULONG  GetCurrentTime(void);
   virtual ULONG  GetCurrentPos(void);
   virtual ULONG  GetCurrentWritePos(void);
   virtual void   SetCurrentTime(ULONG time);
   virtual ULONG  Read(PSTREAMBUF, unsigned);
   virtual ULONG  Write(PSTREAMBUF, ULONG, BOOL fLooping = 0);
           ULONG  Write(PSTREAMBUFFER);
   virtual ULONG  StartStream(void);
   virtual ULONG  StopStream(PCONTROL_PARM);
   virtual ULONG  PauseStream(PCONTROL_PARM);
   virtual ULONG  ResumeStream(void);
   WAVESTREAM(ULONG streamtype, LPMCI_AUDIO_INIT pinit, USHORT filesysnum);
   virtual ~WAVESTREAM();

   virtual BOOL  SetProperty(int type, ULONG value, ULONG reserved = 0);
   virtual ULONG GetProperty(int type);

protected:

   ULONG   _ulAudioBufSz;         // size of the audio buffer
   WaveConfigInfo _configinfo;    // configuration info shared with the hardware
   ULONG   _ulBytesProcessed;     // number of bytes consumed or produces
   ULONG   _ulTimeBase;           // value in ms. MMPM sends for stream time
   ULONG   fragsize;
   USHORT  _vRealignBuffer(ULONG FAR *endpos, PSTREAMBUFFER pbuffer);
   void    _vRealignPausedBuffers(ULONG endpos = 0);
   virtual void    AddBuffers(BOOL fFirst);    	    // Initialize the audio buffer object
   ULONG   AddBuffer(ULONG space);             // write one buffer to the audio buffer
   BOOL    _vReadAudioBuf(void);    // read data from the audio buffer

   BOOL    fUnderrun;
};
typedef WAVESTREAM *PWAVESTREAM;

#endif
