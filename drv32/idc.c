/* $Id: idc.c,v 1.3 2000/07/17 18:34:54 sandervl Exp $ */

//******************************************************************************
// IDC entrypoint (all calls from 16 bits MMPM2 driver end up here)
//
// Copyright 2000 Sander van Leeuwen (sandervl@xs4all.nl)
//
//     This program is free software; you can redistribute it and/or
//     modify it under the terms of the GNU General Public License as
//     published by the Free Software Foundation; either version 2 of
//     the License, or (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public
//     License along with this program; if not, write to the Free
//     Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
//     USA.
//
//******************************************************************************
extern "C" {               // 16-bit header files are not C++ aware
#define INCL_NOPMAPI
#define INCL_DOSINFOSEG
#include <os2.h>
}
#include <devtype.h>
#include <devhelp.h>
#include <strategy.h>
#include <ossidc.h>
#include <irqos2.h>
#include <stacktoflat.h>

//16:32 address of 16 bits pdd idc handler
IDC16_HANDLER idc16_PddHandler = 0;

WORD32 OSS32IDC(ULONG cmd, PIDC32_PACKET pPacket);

//packet pointer must reference a structure on the stack

WORD32 SBLive32IDC(ULONG cmd, ULONG packet);
#pragma aux SBLive32IDC "SBLIVE_IDC" parm reverse [ecx edx]
WORD32 SBLive32IDC(ULONG cmd, ULONG packet)
{
 PIDC32_PACKET pPacket = (PIDC32_PACKET)__StackToFlat(packet);
 ULONG  oldfileid;
 WORD32 rc;

  //Sets file id in current task structure
  oldfileid = OSS32_SetFileId(pPacket->fileid);
  rc = OSS32IDC(cmd & 0xFFFF, pPacket);
  OSS32_SetFileId(oldfileid);
  return rc;
}

WORD32 OSS32IDC(ULONG cmd, PIDC32_PACKET pPacket)
{
  switch(cmd)
  {
  case IDC32_INIT:
	idc16_PddHandler = (IDC16_HANDLER)__Make48Pointer(pPacket->init.handler16);
        return OSS32_InitDriver();

  case IDC32_EXIT:
	idc16_PddHandler = 0;
	return TRUE;

  case IDC32_IRQ:
	return oss_process_interrupt(pPacket->irq.irqnr);

  case IDC32_STREAM_OPEN:
	return OSS32_StreamOpen(pPacket->open.streamtype);

  case IDC32_STREAM_CLOSE:
	return OSS32_StreamClose(pPacket->close.streamtype, pPacket->close.streamid);

  case IDC32_STREAM_ADDBUFFER:
	return OSS32_StreamAddBuffer(pPacket->buffer.streamtype, pPacket->buffer.streamid, pPacket->buffer.buffer, pPacket->buffer.size);

  case IDC32_STREAM_PAUSE:
	break;

  case IDC32_STREAM_START:
  {
    ULONG fStart = TRUE;

#ifdef KEE
	return OSS32_StreamTrigger(pPacket->startstop.streamtype, pPacket->startstop.streamid, (ULONG *)&fStart);
#else
	return OSS32_StreamTrigger(pPacket->startstop.streamtype, pPacket->startstop.streamid, (ULONG NEAR *)__StackToFlat((ULONG)&fStart));
#endif
  }

  case IDC32_STREAM_STOP:
  {
    ULONG fStart = FALSE;
    
#ifdef KEE
	return OSS32_StreamTrigger(pPacket->startstop.streamtype, pPacket->startstop.streamid, (ULONG *)&fStart);
#else
	return OSS32_StreamTrigger(pPacket->startstop.streamtype, pPacket->startstop.streamid, (ULONG NEAR *)__StackToFlat((ULONG)&fStart));
#endif
  }

  case IDC32_MIDI_WRITE:
        return OSS32_StreamMidiWrite(pPacket->midiwrite.streamid, pPacket->midiwrite.midiByte);

  case IDC32_MIDI_READ:
	return OSS32_StreamMidiRead(pPacket->midiread.streamid, (char NEAR *)__StackToFlat((ULONG)pPacket->midiread.buffer), pPacket->midiread.bufsize);

  case IDC32_STREAM_RESET:
	return OSS32_StreamReset(pPacket->startstop.streamtype, pPacket->startstop.streamid);

  case IDC32_STREAM_GETSPACE:
	return OSS32_StreamGetSpace(pPacket->getspace.streamtype, pPacket->getspace.streamid);
  
  case IDC32_STREAM_IOCTL:
	switch(pPacket->ioctl.cmd) {
	case IOCTL_SETFORMAT:
	{
 	  FORMAT_INFO NEAR *pFormatInfo = (FORMAT_INFO NEAR *)__StackToFlat(pPacket->ioctl.ioctl_param);

		return OSS32_StreamSetFormat(pPacket->ioctl.streamtype, pPacket->ioctl.streamid, pPacket->ioctl.cmd, pFormatInfo);
	}
	default:
		return OSS32_StreamIOCtl(pPacket->ioctl.streamtype, pPacket->ioctl.streamid, pPacket->ioctl.cmd, (char NEAR *)__StackToFlat(pPacket->ioctl.ioctl_param));
	}	
  case IDC32_STREAM_MIXER:
  	return OSS32_SetVolume(pPacket->mixer.streamtype, pPacket->mixer.streamid, pPacket->mixer.cmd, pPacket->mixer.volume);
  }
  return 0;
}
