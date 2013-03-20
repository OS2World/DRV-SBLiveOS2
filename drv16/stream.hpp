/* $Id: stream.hpp,v 1.4 2001/04/30 21:07:59 sandervl Exp $ */

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
#ifndef STREAM_INCLUDED
#define STREAM_INCLUDED

#ifndef OS2_INCLUDED
#define INCL_NOPMAPI
#include <os2.h>
#endif
#include <os2me.h>
#include <shdd.h>

#include "strategy.h"
#include "audiohw.hpp"
#include "strmbuff.hpp"
#include <daudio.h>

extern PQUEUEHEAD pStreamList;   // List head for Streams.Defined in STREAM.CPP.

// stream types
#define STREAM_READ             AUDIOHW_READ_DEVICE
#define STREAM_WRITE            AUDIOHW_WRITE_DEVICE
#define STREAM_WAVE_CAPTURE     AUDIOHW_WAVE_CAPTURE
#define STREAM_WAVE_PLAY        AUDIOHW_WAVE_PLAY
#define STREAM_MPU401_CAPTURE   AUDIOHW_MPU401_CAPTURE
#define STREAM_MPU401_PLAY      AUDIOHW_MPU401_PLAY

// stream states
#define STREAM_STOPPED     0x00000000
#define STREAM_STREAMING   0x00000001
#define STREAM_PAUSED      0x00000002
#define STREAM_IDLE        0x00000004
#define STREAM_NOT_IDLE    0xFFFFFFFB

// input sources
#define INPUT_LINEIN		0
#define INPUT_MIC		1
#define INPUT_MIXER		2

typedef ULONG (__far __cdecl *PFN_SHD) (void __far *);

class STREAM;
class AUDIOHW;
class EVENT;

class STREAM : public QUEUEELEMENT {
public:
   HSTREAM hstream;
   PFN_SHD pfnSHD;
   ULONG ulSysFileNum;
   QUEUEHEAD      qhInProcess;
   QUEUEHEAD      qhDone;
   QUEUEHEAD      qhEvent;
   ULONG ulStreamType;       // the stream type see above
   ULONG ulStreamState;      // the current state to the stream see above
   ULONG ulStreamId;

   int fIncrementCounter;        // true if the current time should be incremented on every tick

   ULONG EnableEvent(PDDCMDCONTROL pControl);
   ULONG DisableEvent(PDDCMDCONTROL pControl);
   ULONG PauseStreamTime(void);
   ULONG ResumeStreamTime(void);
   virtual ULONG Register(PDDCMDREGISTER);
   virtual void  DeRegister(void);
   virtual ULONG Write(PSTREAMBUF, ULONG, BOOL fLooping = 0);
   virtual void  SetLooping(BOOL fLooping);
   virtual ULONG Read(PSTREAMBUF, unsigned) = 0;
   virtual ULONG GetCurrentTime(void) = 0;
   virtual ULONG GetCurrentPos(void) = 0;
   virtual ULONG GetCurrentWritePos(void) = 0;
   virtual void  SetCurrentTime(ULONG time) = 0;
   virtual ULONG StartStream(void) = 0;
   virtual ULONG StopStream(PCONTROL_PARM) = 0;
   virtual ULONG PauseStream(PCONTROL_PARM) = 0;
   virtual ULONG ResumeStream(void) = 0;

           BOOL  isActive()        { return ulStreamState == STREAM_STREAMING; };

   virtual BOOL  SetProperty(int type, ULONG value, ULONG reserved = 0);
   virtual ULONG GetProperty(int type);

   void SetNextEvent(void);
   STREAM(ULONG streamtype, USHORT filesysnum);
   virtual ~STREAM(void);

protected:
   ULONG ulCurrentTime;
   PAUDIOHW pahw;            // pointer to the hardware object for this stream
   virtual void ReturnBuffer(void);   // returns one buffer
   void ReturnBuffers(void);  // returns all buffers
   void ProcessEvents(void);

       ULONG balance;
       ULONG volume;
       ULONG inputsrc;
       ULONG inputgain;
static ULONG mastervol;
};
typedef STREAM *PSTREAM;


PSTREAM FindActiveStream(ULONG StreamType);
// Returns the pointer to the Active STREAM object with the given stream type

PSTREAM FindActiveStream(ULONG StreamType, ULONG streamid);
// Returns the pointer to the Active STREAM object with the given stream type

PSTREAM FindStream(HSTREAM hStream);
// Returns the pointer to the STREAM object with the given stream handle

PSTREAM FindStream_fromFile(ULONG ulSysFileNum);
// Returns the pointer to the STREAM object with the given system file handle

#endif
