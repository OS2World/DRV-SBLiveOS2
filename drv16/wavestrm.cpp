/* $Id: wavestrm.cpp,v 1.10 2001/09/09 15:30:51 sandervl Exp $ */

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
#define INCL_DOSERRORS            // for ERROR_INVALID_FUNCTION
#include <os2.h>
#include <os2me.h>
#include <audio.h>                // for #define MIDI
#include <include.h>

#include "wavestrm.hpp"
#include "audiohw.hpp"
#include "waudio.hpp"
#include "memutil.h"
#include <ossidc.h>
#include <dbgos2.h>
#include "ioctl.h"

#ifndef min
#define min(a,b) (a>b) ? b : a
#endif

//
//  _vRealignBuffer
//  called just after a wave stream pause on a playback.
//  Gets the end position of the stream when paused and a pointer to a
//  STREAMBUFFER. Basicly this function looks at the streambuffer and if
// there is any unplayed data in it it adjusts the bufpos counter.
// the donepos counter is ALWAYS set to zero. It will return 0 if all
// the data has been played and 1 if there is still some data left.
//
USHORT WAVESTREAM::_vRealignBuffer(ULONG FAR *bytesinc, PSTREAMBUFFER pbuffer)
{
   // if none of the data in this stream buffer has been consumed
   if (!*bytesinc) {
      	pbuffer->ulDonepos = 0;
      	pbuffer->ulBuffpos = 0;
      	return 1;
   }

   pbuffer->ulDonepos += *bytesinc;
   pbuffer->ulBuffpos  = pbuffer->ulDonepos;
   *bytesinc           = 0;
   if(pbuffer->ulDonepos >= pbuffer->ulBuffsz) {
	//calc position in next buffer
	*bytesinc = pbuffer->ulDonepos - pbuffer->ulBuffsz;
	return 0; //all of the buffer has been consumed
   }
   return 1;
}
//
// _vRealignPausedBuffers(void)
// when a stream is paused we need to "realign" the data in the audio buffer
// with reality. On playback, not all the data in the audio buffer has been
// consumed. Likewise on a capture, not all the good data in the audio buffer
// has been copied out. After receiving the DDCMDCONTROL Pause we will call
// this function to line the MMPM buffers back up.
// there are 2 cases here: first one is the case of a capture stream.
// for a capture stream we simply read any data that is still in the audio
// buffer into a MMPM buffer.
// for a playback stream things are not so straight forward.
// first check the STREAMBUFFER on pHead to see if any of it's data is in the
// audio buffer and not consumed, if yes back up the ulBuffpos in the
// STREAMBUFFER. Next check any STREAMBUFFERS on pdone starting with the last
// one. (the one on the tail of the queue) If necessary back up the ulBuffpos
// and put the STREAMBUFFER on the Head queue.
//
void WAVESTREAM::_vRealignPausedBuffers(ULONG endpos)
{
   PSTREAMBUFFER ptempbuff;

   switch (ulStreamType & STREAM_WRITE) {
   case STREAM_READ:
	//SvL: Don't get the lastest recording data as a read command
        //     would now restart recording (as it's stopped)
        //     Just return what we've got or push the buffer on the inprocess queue
	ptempbuff = (PSTREAMBUFFER)qhDone.Head();
	if(ptempbuff) {
		if(ptempbuff->ulBuffpos) {//if we recorded anything into this buffer, then return it now
			ReturnBuffer();
			return;
		}
		ptempbuff->ulBuffpos = 0;
		ptempbuff->ulDonepos = 0;
		qhInProcess.PushOnHead(qhDone.PopHead());
	}
      	break;

   case STREAM_WRITE:
   {
        PQUEUEHEAD pTempHead = new QUEUEHEAD;
        ULONG bytesinc;
        USHORT usRC;

	bytesinc = endpos - _ulBytesProcessed;
      	bytesinc &= 0xFFFFFFFC; //keep it on a dword boundary

      	// if there are bufferes on the done queue, pop them off the head and
      	// push them on the head of qhTempHead.  This will reorder them so
      	// that the more recently used ones will be in the front of the queue.
      	// Pass them all to _vRealignBuffer. If the rc from _vRealignBuffer is
      	// 0 then there is no unprocessed data in the buffer (it is ready to
      	// be returned) so put it on the Tail of the done queue.
      	// If the rc is 1 then put it on the head of the InProcess queue.

      	while (qhDone.IsElements()) {
         	pTempHead->PushOnTail(qhDone.PopHead());
      	} /* endwhile */

      	while (qhInProcess.IsElements()) {
         	pTempHead->PushOnTail(qhInProcess.PopHead());
      	} /* endwhile */

      	while(pTempHead->IsElements()) {
         	usRC = _vRealignBuffer(&bytesinc, (PSTREAMBUFFER)pTempHead->Head());
         	if (usRC) {
           		qhInProcess.PushOnTail(pTempHead->PopHead());
		}
         	else {
		    	qhDone.PushOnTail(pTempHead->PopHead());
		}
      	} /* endwhile */
	if(qhDone.IsElements())
		ReturnBuffer();

   	delete pTempHead; // free the memory this ain't no Java here !!
      	break;
   }
   default:
      	break;
   } /* endswitch */
}
//
// get ready to start streaming
// this requires the following:
// call Initbuffer in the audiobuffer object
// if this is a write stream call _vFillAudioBuf
//
#pragma off (unreferenced)
void WAVESTREAM::AddBuffers(BOOL fFirst)
#pragma on (unreferenced)
{
 ULONG space, byteswritten;

   if (ulStreamType & STREAM_WRITE) {
	if(!qhInProcess.Head() && !qhDone.Head()) {
		//underrun: stop playback
		dprintf(("underrun: stop playback"));
		pahw->Stop(this);
		fUnderrun = TRUE;
		return;
	}
	space = OSS16_StreamGetSpace(this);
	while(space) {
		byteswritten = AddBuffer(space);
		if(byteswritten == (ULONG)-1) break;
		space -= byteswritten;
	}
   }
}

//
// write one buffer to the audio buffer
// the caller of this function MUST make sure it ok to write the audio buffer..
// _AudioBufWrite will not check if there is room in the audio buffer of if
// there are buffers on pHead... BEWARE
//
ULONG WAVESTREAM::AddBuffer(ULONG space)
{
   PSTREAMBUFFER pTemp = (PSTREAMBUFFER)qhDone.Tail();
   ULONG pdataBuf;
   ULONG Buff_left, byteswritten;

   if(!pTemp || pTemp->ulBuffpos >= (pTemp->ulBuffsz & 0xFFFFFFFC)) {
	pTemp = (PSTREAMBUFFER)qhInProcess.Head();
   }
   if(!pTemp) {
	dprintf4(("AddBuffer: pTemp == NULL"));
	return (ULONG)-1;
   }

   // get the buffer pointer and amount of data remaining
   pdataBuf = (ULONG)pTemp->pBuffptr + pTemp->ulBuffpos;
   Buff_left = pTemp->ulBuffsz - pTemp->ulBuffpos;

   if(Buff_left) {
        // write the audio buffer
        Buff_left = min(Buff_left, space);
        byteswritten = OSS16_StreamAddBuffer(this, pdataBuf, Buff_left);
        if(byteswritten == 0) {
  	    return (ULONG)-1; //no more room
        }

        // update the buffer pos counter
        pTemp->ulBuffpos += byteswritten;
   } 
   else byteswritten = 0;

   if(pTemp == qhInProcess.Head()) {
	qhDone.PushOnTail(qhInProcess.PopHead());
   }
   dprintf4(("AddBuffer %lx size %d, bytes written %d", pdataBuf, (USHORT)Buff_left, (USHORT)byteswritten));
   return byteswritten;
}

// Read data from the audio Buffer.
// Called at interrupt time to get the good data from the audiobuffer object.
//
BOOL WAVESTREAM::_vReadAudioBuf(void)
{
   PSTREAMBUFFER pTemp = (PSTREAMBUFFER)qhInProcess.Head();
   ULONG pdataBuf;
   ULONG Buff_left, bytesread;

   if(!pTemp) return FALSE;

   // get the buffer pointer and amount of data remaining
   pdataBuf = (ULONG)pTemp->pBuffptr + pTemp->ulBuffpos;
   Buff_left = pTemp->ulBuffsz - pTemp->ulBuffpos;

   // write the audio buffer
   bytesread = OSS16_StreamAddBuffer(this, pdataBuf, Buff_left);
   if(bytesread == 0) {
	return FALSE; //no more data
   }

   dprintf4(("_vReadAudioBuf %lx size %d, bytes read %d", pdataBuf, Buff_left, bytesread));

   // update the buffer pos counter
   pTemp->ulBuffpos  += bytesread;
   _ulBytesProcessed += bytesread;

   if(pTemp->ulBuffpos == pTemp->ulBuffsz) {
      	qhDone.PushOnTail(qhInProcess.PopHead());
	ReturnBuffer();
   	dprintf4(("_vReadAudioBuf return buffer %lx size %ld, bytes read %ld", (ULONG)pTemp->pBuffptr, pTemp->ulBuffsz, bytesread));
   }

   return TRUE;
}
// called by the irq function in the hardware object when we get an interrupt
// first call _vUpdateProcessed() to update the dma amd audio buffer related
// stuff. Next if we have buffers on the primary queue try to read/write them
// to the audiobuffer. Look at the buffers on the done queue and see if they
// can be returned and finally process any events pending.
void WAVESTREAM::Process(void)
{
 PSTREAMBUFFER ptemp;
 ULONG         ulCurBytesProcessed = 0;
 ULONG         bytesinc;

   switch (ulStreamType & STREAM_WRITE) {
   case STREAM_WRITE:
   {
   	OSS16_StreamGetPos(this, &ulCurBytesProcessed);
   	if(ulCurBytesProcessed == 0) {
		//shouldn't happen
		DebugInt3();
		return;
	}
   	bytesinc           = ulCurBytesProcessed - _ulBytesProcessed;
	dprintf4(("Process: %lx %x", ulCurBytesProcessed, (USHORT)bytesinc));
	if(ulCurBytesProcessed < _ulBytesProcessed) {
       		dprintf(("WARNING: Process: Current pos %ld incr %d", ulCurBytesProcessed, (USHORT)bytesinc));
	}
   	_ulBytesProcessed  = ulCurBytesProcessed;

      	while(bytesinc) {
      	  if(qhDone.IsElements()) {  // if there are buffers that have been
                                  // completly written to the audio buffer
                                  // check the first one on the done queue
                                  // if it's data has been consumed by
                                  // the hardware return it
         	ptemp = (PSTREAMBUFFER)qhDone.Head();
		ptemp->ulDonepos += bytesinc;
		bytesinc          = 0;
		if(ptemp->ulDonepos >= ptemp->ulBuffsz) {
			//calc position in next buffer
			bytesinc = ptemp->ulDonepos - ptemp->ulBuffsz;
			dprintf3(("Process: Return buffer %lx size %d", ptemp->pBuffptr, ptemp->ulBuffsz));
		 	ReturnBuffer();
		}
      	  }
	  else	break; //shouldn't happen
      	}
	AddBuffers(FALSE);
	break;
   }
   case STREAM_READ:
	while(_vReadAudioBuf());
	break;
   default:
      break;
   } /* endswitch */

   ProcessEvents();
}

#pragma off (unreferenced)
ULONG WAVESTREAM::Write(PSTREAMBUF pbuf, ULONG uLength, BOOL fLooping)
#pragma on (unreferenced)
{
 PSTREAMBUFFER pStreamBuf = new STREAMBUFFER(uLength, pbuf);

   return Write(pStreamBuf);
}

ULONG WAVESTREAM::Write(PSTREAMBUFFER pStreamBuf)
{
   qhInProcess.PushOnTail((PQUEUEELEMENT)pStreamBuf);
   dprintf2(("WAVESTREAM::Write: Push on tail %lx %ld", ((PSTREAMBUFFER)qhInProcess.Tail())->pBuffptr, ((PSTREAMBUFFER)qhInProcess.Tail())->ulBuffsz));
   if(fUnderrun) {
	fUnderrun = FALSE;
   	OSS16_StreamReset(this);
	AddBuffers(TRUE);
	if(ulStreamType == STREAM_WAVE_PLAY)
   		OSS16_SetWaveOutVol(this, volume);
   }
   return 0;
}

ULONG WAVESTREAM::Read(PSTREAMBUF pbuf, unsigned uLength)
{
   qhInProcess.PushOnTail((PQUEUEELEMENT)new STREAMBUFFER(uLength, pbuf));
   dprintf2(("WAVESTREAM::Read: Push on tail %lx %d", ((PSTREAMBUFFER)qhInProcess.Head())->pBuffptr, ((PSTREAMBUFFER)qhInProcess.Head())->ulBuffsz));
   return 0;
}

// WAVESTREAM::GetCurrentTime(void)
// get current time will calculate the stream time in milliseconds based on
// NOW.... the open mpeg folks this is the greatest thing since my last
// pay raise!!
// but then again you know what those ring 3 programmers are like....
// the algorythum goes like this....
// bytes consumed / consume rate = seconds
// Note before calling BufferUpdate check to see if the stream is running.
// if it is not then call with flags of 0. This will prevent the audio buffer
// trying to get the latest consumption info from the hardware object. (dma or
// or pci as the case may be) Asking a hardware object that is not running for
// status information is just not a good idea.....
//

ULONG WAVESTREAM::GetCurrentTime()
{
   ULONG Seconds, MilliSeconds, Overflow, Processed;

   if (ulStreamState == STREAM_STREAMING)  // if the stream is active
   {
   	if (ulStreamType & STREAM_WRITE) {
		OSS16_StreamGetPos(this, &Processed);
	}
	else	Processed = _ulBytesProcessed;
   }
   else Processed = _ulBytesProcessed;

   // if we haven't processed anything then just return
   // _ulTimeBase
   if(Processed == 0)
      return(_ulTimeBase);

   Seconds = Processed / _configinfo.ulPCMConsumeRate;
   Overflow = Processed - (Seconds * _configinfo.ulPCMConsumeRate);
   MilliSeconds = (Overflow * 1000) / _configinfo.ulPCMConsumeRate;
   MilliSeconds += (Seconds * 1000);
   return(MilliSeconds + _ulTimeBase);
}

ULONG WAVESTREAM::GetCurrentPos(void)
{
   ULONG Processed;

   if (ulStreamState == STREAM_STREAMING)  // if the stream is active
   {
   	if (ulStreamType & STREAM_WRITE) {
		OSS16_StreamGetPos(this, &Processed);
	}
	else	Processed = _ulBytesProcessed;
   }
   else Processed = _ulBytesProcessed;

   return Processed;
}

ULONG WAVESTREAM::GetCurrentWritePos(void)
{
   ULONG writepos = 0;

   cli();
   PSTREAMBUFFER pTemp = (PSTREAMBUFFER)qhDone.Tail();

   if(!pTemp) {
       pTemp = (PSTREAMBUFFER)qhInProcess.Head();
   }
   if(pTemp) {
       writepos = pTemp->ulBuffpos;
   }
   sti();
   return writepos;
}

//
// SetCurrentTime
// MMPM will send in the "starting stream time" as they see it.
// "our stream time" will always start at 0, so we save "their" time and
// add it to the elapsed time we calculate when we need to return time.
//
void  WAVESTREAM::SetCurrentTime(ULONG time)
{
   _ulTimeBase = time;
}

//
//
ULONG WAVESTREAM::StartStream(void)
{
 PSTREAMBUFFER pTemp = (PSTREAMBUFFER)qhInProcess.Head();

   // configure the wave device
   ((PWAVEAUDIO)pahw)->ConfigDev(this, &_configinfo);

   if(ulStreamType == STREAM_WAVE_PLAY) {
   	fragsize = _configinfo.ulPCMConsumeRate/64; //start with 64 irqs/sec
   }
   else fragsize = _configinfo.ulPCMConsumeRate/32; //start with 32 irqs/sec (no need for more)

   //if the buffer is smaller than our predefined fragmentsize (*2), then correct it
   //I assume here that buffers sizes don't radically change (except the last one)
   //while playing a stream. If they do get a lot smaller, then we'll run into problems.
   //There's nothing we can do about it as the fragment size can't be changed
   //while the stream is playing.
   if(pTemp->ulBuffsz/2 < fragsize) {
	fragsize = pTemp->ulBuffsz/2;
	if(fragsize < _configinfo.ulPCMConsumeRate/256) 
	{//lower limit; don't accept extremely small buffers
		fragsize = _configinfo.ulPCMConsumeRate/256;
	}
   }
   OSS16_StreamSetFragment(this, fragsize);
   dprintf(("WAVESTREAM::StartStream: Fragment size %d", (USHORT)fragsize));
   _ulBytesProcessed = 0;
   fUnderrun = FALSE;

   ulStreamState = STREAM_STREAMING;
   //Adding the first buffer also starts playback
   if(ulStreamType == STREAM_WAVE_PLAY) {
   	AddBuffers(TRUE);
   }
   else {
	if(!fRecSrcIOCTL90)
		OSS16_SetVolume(this, MIX_SETINPUTSRC, inputsrc);
	if(!fRecGainIOCTL90)
		OSS16_SetVolume(this, MIX_SETINPUTGAIN, inputgain);
	OSS16_StartStream(this);
   }

   //Must set volume after adding buffers (voices inside sblive driver might not
   //be allocated otherwise (first start) )
   if(ulStreamType == STREAM_WAVE_PLAY)
   	OSS16_SetWaveOutVol(this, volume);

   dprintf(("WAVESTREAM::StartStream %lx", ulStreamId));
   return NO_ERROR;

}

ULONG  WAVESTREAM::StopStream(PCONTROL_PARM pControl)
{
   if(ulStreamState == STREAM_STOPPED) {
   	dprintf(("WAVESTREAM::StopStream %lx (already stopped)", ulStreamId));
   	fUnderrun = FALSE;
   	pControl->ulTime = GetCurrentTime();
	return NO_ERROR;
   }
   pahw->Stop(this);
   //Reset cleans up waveout instance
   OSS16_StreamReset(this);

   ulStreamState = STREAM_STOPPED;
   fUnderrun = FALSE;
   dprintf(("WAVESTREAM::StopStream %lx", ulStreamId));
   ReturnBuffers();
   pControl->ulTime = GetCurrentTime();
   _ulTimeBase = GetCurrentTime();
   return NO_ERROR;

}

ULONG  WAVESTREAM::PauseStream(PCONTROL_PARM pControl)
{
 ULONG endpos;

   OSS16_StreamGetPos(this, &endpos);

   pahw->Stop(this);
   //Reset cleans up waveout instance
   OSS16_StreamReset(this);

   ulStreamState = STREAM_PAUSED;
   fUnderrun = FALSE;

   dprintf(("WAVESTREAM::PauseStream %lx", ulStreamId));
   _vRealignPausedBuffers(endpos);

   _ulBytesProcessed = endpos;
   pControl->ulTime = GetCurrentTime();
   _ulTimeBase = GetCurrentTime();
   return NO_ERROR;

}
ULONG  WAVESTREAM::ResumeStream(void)
{
   // configure the wave device
   ((PWAVEAUDIO)pahw)->ConfigDev(this, &_configinfo);
   if(ulStreamType == STREAM_WAVE_PLAY)
   	OSS16_SetWaveOutVol(this, volume);

   dprintf(("WAVESTREAM::ResumeStream %lx", ulStreamId));
   _ulBytesProcessed = 0;
   fUnderrun = FALSE;

   ulStreamState = STREAM_STREAMING;
   //Adding the first buffer also starts playback
   AddBuffers(TRUE);

   return NO_ERROR;

}


BOOL WAVESTREAM::SetProperty(int type, ULONG value, ULONG reserved)
{
   switch(type) {
   case PROPERTY_VOLUME:
       volume = value; 
       if(ulStreamState == STREAM_STREAMING && ulStreamType == STREAM_WAVE_PLAY) {
           OSS16_SetWaveOutVol(this, volume);
       }
       break;

   case PROPERTY_INPUTSRC:
       inputsrc = value;
       break;

   case PROPERTY_INPUTGAIN:
       inputgain = value;
       break;

   default:
       return STREAM::SetProperty(type, value, reserved);

   }
   return TRUE;
}

ULONG WAVESTREAM::GetProperty(int type) 
{
   switch(type) {
   case PROPERTY_FREQUENCY:
       return _configinfo.ulSampleRate;

   case PROPERTY_INPUTSRC:
       return inputsrc;

   case PROPERTY_INPUTGAIN:
       return inputgain;

   default:
       return STREAM::GetProperty(type);
   }
}

WAVESTREAM::WAVESTREAM(ULONG streamtype, LPMCI_AUDIO_INIT pinit, USHORT filesysnum):
   STREAM(streamtype, filesysnum)
{
   _configinfo.ulSampleRate = pinit->lSRate;
   _configinfo.ulBitsPerSample = pinit->lBitsPerSRate;
   _configinfo.ulNumChannels = pinit->sChannels;
   _configinfo.ulDataType = pinit->sMode;
   _ulBytesProcessed = 0;
   _ulTimeBase = 0;

   fUnderrun = FALSE;

   pinit->ulFlags |= FIXED;             // Fixed length data
   pinit->ulFlags |= LEFT_ALIGNED;      // Left align bits on byte bndry
   if (pinit->lBitsPerSRate == 8)
      pinit->ulFlags|= TWOS_COMPLEMENT; // 2's complement data

   ulStreamId = OSS16_OpenStream(this);
   dprintf(("WAVESTREAM ctor %lx: rate %d bps %d numchan %d type %x", ulStreamId, (USHORT)_configinfo.ulSampleRate, (USHORT)_configinfo.ulBitsPerSample, (USHORT)_configinfo.ulNumChannels, (USHORT)_configinfo.ulNumChannels, (USHORT)_configinfo.ulDataType));
}

WAVESTREAM::~WAVESTREAM()
{
   dprintf(("WAVESTREAM dtor %lx", ulStreamId));
   if(ulStreamId) {
	OSS16_CloseStream(this);
   }
}

