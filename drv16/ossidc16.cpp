/* $Id: ossidc16.cpp,v 1.6 2001/04/30 21:07:58 sandervl Exp $ */

//******************************************************************************
// IDC interface for calling 32 bits SB Live PDD + IDC entrypoint
//
// Copyright 2000 Sander van Leeuwen (sandervl@xs4all.nl)
//
//******************************************************************************
extern "C" {               // 16-bit header files are not C++ aware
#define  INCL_16
#define  INCL_DOSINFOSEG
#include <os2.h>
}

#include <ctype.h>
#include <string.h>
#include <devhelp.h>

#include "idc_vdd.h"
#include <include.h>
#include "rm.hpp"
#include "memutil.h"
#include "irq.hpp"
#include "stream.hpp"
#include "wavestrm.hpp"
#include "mpu401.hpp"
#include "malloc.h"
#include <ossidc.h>
#include <dbgos2.h>

//init.c
extern ResourceManager* pRM;               // Resource manager object.

IDCTABLE IDCTable= {0};

extern "C" ULONG far __cdecl OSSIDC_ENTRY(USHORT cmd, IDC16_PACKET FAR *packet);

//******************************************************************************
//******************************************************************************
BOOL OSS16_AttachToPdd()
{
   // Attach to the 32 bits OSS driver
   if(DevHelp_AttachDD((NPSZ)"SBLIVE2$",(NPBYTE)&IDCTable)) {
      return FALSE;
   }
   return (BOOL) CallOSS32(IDC32_INIT, 0, (ULONG)(VOID FAR *)OSSIDC_ENTRY, 0, 0, 0);
}
//******************************************************************************
//******************************************************************************
void OSS16_DetachFromPdd()
{
   CallOSS32(IDC32_EXIT, 0, 0, 0, 0, 0);
}
//******************************************************************************
//******************************************************************************
ULONG MMPMToOSSStreamType(ULONG streamtype)
{
   switch((USHORT)streamtype) {
   case STREAM_WAVE_CAPTURE:
	return OSS_STREAM_WAVEIN;
   case STREAM_WAVE_PLAY:
	return OSS_STREAM_WAVEOUT;
   case STREAM_MPU401_CAPTURE:
	return OSS_STREAM_MIDIIN;
   case STREAM_MPU401_PLAY:
	return OSS_STREAM_MIDIOUT;
   }
   DebugInt3();
   return -1;
}
//******************************************************************************
//******************************************************************************
ULONG OSS16_OpenStream(STREAM *stream)
{
   return CallOSS32(IDC32_STREAM_OPEN, stream->ulSysFileNum, MMPMToOSSStreamType(stream->ulStreamType), 0, 0, 0);
}
//******************************************************************************
//******************************************************************************
void OSS16_CloseStream(STREAM *stream)
{
   CallOSS32(IDC32_STREAM_CLOSE, stream->ulSysFileNum, MMPMToOSSStreamType(stream->ulStreamType), stream->ulStreamId, 0, 0);
}
//******************************************************************************
//******************************************************************************
ULONG OSS16_OpenMidiStream(MIDITYPE midiType)
{
   return CallOSS32(IDC32_STREAM_OPEN, 0x666, (midiType == MIDI_RECEIVE) ? OSS_STREAM_MIDIOUT: OSS_STREAM_MIDIIN, 0, 0, 0);
}
//******************************************************************************
//******************************************************************************
BOOL OSS16_WriteMidiByte(ULONG streamid, BYTE midiByte)
{
   return (BOOL)CallOSS32(IDC32_MIDI_WRITE, 0x666, OSS_STREAM_MIDIOUT, streamid, midiByte, 0);
}
//******************************************************************************
//******************************************************************************
int OSS16_ReadMidiBytes(ULONG streamid, char far *buffer, int bufsize)
{
   return (int)CallOSS32(IDC32_MIDI_READ, 0x666, OSS_STREAM_MIDIIN, streamid, (ULONG)buffer, bufsize);
}
//******************************************************************************
//******************************************************************************
void OSS16_CloseMidiStream(MIDITYPE midiType, ULONG streamid)
{
   CallOSS32(IDC32_STREAM_CLOSE, 0x666, (midiType == MIDI_RECEIVE) ? OSS_STREAM_MIDIOUT : OSS_STREAM_MIDIIN, streamid, 0, 0);
}
//******************************************************************************
//******************************************************************************
BOOL OSS16_StartStream(STREAM *stream)
{
   return (BOOL)CallOSS32(IDC32_STREAM_START, stream->ulSysFileNum, MMPMToOSSStreamType(stream->ulStreamType), stream->ulStreamId, 0, 0);
}
//******************************************************************************
//******************************************************************************
BOOL OSS16_PauseStream(STREAM *stream)
{
   return (BOOL)CallOSS32(IDC32_STREAM_PAUSE, stream->ulSysFileNum, MMPMToOSSStreamType(stream->ulStreamType), stream->ulStreamId, 0, 0);
}
//******************************************************************************
//******************************************************************************
BOOL OSS16_StopStream(STREAM *stream)
{
   return (BOOL)CallOSS32(IDC32_STREAM_STOP, stream->ulSysFileNum, MMPMToOSSStreamType(stream->ulStreamType), stream->ulStreamId, 0, 0);
}
//******************************************************************************
//******************************************************************************
BOOL OSS16_StreamReset(STREAM *stream)
{
   return (BOOL)CallOSS32(IDC32_STREAM_RESET, stream->ulSysFileNum, MMPMToOSSStreamType(stream->ulStreamType), stream->ulStreamId, 0, 0);
}
//******************************************************************************
//******************************************************************************
BOOL OSS16_StreamSetFormat(STREAM *stream, ULONG param1)
{
   return (BOOL)CallOSS32(IDC32_STREAM_IOCTL, stream->ulSysFileNum, MMPMToOSSStreamType(stream->ulStreamType), stream->ulStreamId, IOCTL_SETFORMAT, param1);
}
//******************************************************************************
//Returns nr of bytes written
//******************************************************************************
ULONG OSS16_StreamAddBuffer(STREAM *stream, ULONG buffer, ULONG size)
{
   return CallOSS32(IDC32_STREAM_ADDBUFFER, stream->ulSysFileNum, MMPMToOSSStreamType(stream->ulStreamType), stream->ulStreamId, buffer, size);
}
//******************************************************************************
//******************************************************************************
BOOL OSS16_StreamGetPos(STREAM *stream, ULONG FAR *pos)
{
   return (BOOL)CallOSS32(IDC32_STREAM_IOCTL, stream->ulSysFileNum, MMPMToOSSStreamType(stream->ulStreamType), stream->ulStreamId, IOCTL_GETPOS, (ULONG)pos);
}
//******************************************************************************
//******************************************************************************
BOOL OSS16_StreamSetFragment(STREAM *stream, ULONG fragsize)
{
 ULONG fsize = fragsize;

   return (BOOL)CallOSS32(IDC32_STREAM_IOCTL, stream->ulSysFileNum, MMPMToOSSStreamType(stream->ulStreamType), stream->ulStreamId, IOCTL_SETFRAGMENT, (ULONG)((ULONG FAR *)&fsize));
}
//******************************************************************************
//******************************************************************************
BOOL OSS16_SetMasterVol(STREAM *stream, ULONG volume)
{
   return (BOOL)CallOSS32(IDC32_STREAM_MIXER, stream->ulSysFileNum, MMPMToOSSStreamType(stream->ulStreamType), stream->ulStreamId, MIX_SETMASTERVOL, (ULONG)volume);
}
//******************************************************************************
//******************************************************************************
BOOL OSS16_SetWaveOutVol(STREAM *stream, ULONG volume)
{
   return (BOOL)CallOSS32(IDC32_STREAM_MIXER, stream->ulSysFileNum, MMPMToOSSStreamType(stream->ulStreamType), stream->ulStreamId, MIX_SETWAVEVOL, (ULONG)volume);
}
//******************************************************************************
//******************************************************************************
BOOL OSS16_SetVolume(STREAM *stream, USHORT line, ULONG volume)
{
   return (BOOL)CallOSS32(IDC32_STREAM_MIXER, stream->ulSysFileNum, MMPMToOSSStreamType(stream->ulStreamType), stream->ulStreamId, line, (ULONG)volume);
}
//******************************************************************************
//******************************************************************************
BOOL OSS16_SetGlobalVol(ULONG ulSysFileNum, USHORT line, ULONG volume)
{
   return (BOOL)CallOSS32(IDC32_STREAM_MIXER, ulSysFileNum, 0, 0, line, (ULONG)volume);
}
//******************************************************************************
//******************************************************************************
ULONG OSS16_StreamGetSpace(STREAM *stream)
{
   return CallOSS32(IDC32_STREAM_GETSPACE, stream->ulSysFileNum, MMPMToOSSStreamType(stream->ulStreamType), stream->ulStreamId, 0, 0);
}
//******************************************************************************
//******************************************************************************
ULONG CallOSS32(USHORT cmd, ULONG fileid, ULONG param1, ULONG param2, ULONG param3, ULONG param4)
{
 ULONG rc = 0;
 ULONG idcptr  = (ULONG)IDCTable.ProtIDCEntry;
 ULONG idcparm;
 IDC32_PACKET idcpacket;

   if(idcptr == 0)
	return(0);

   idcpacket.fileid = fileid;
   idcpacket.param1 = param1;
   idcpacket.param2 = param2;
   idcpacket.param3 = param3;
   idcpacket.param4 = param4;
   idcparm = (ULONG)&idcpacket;

   _asm {
	pusha
	mov  cx, cmd
	mov  dx, word ptr [idcparm+2]
	mov  bx, word ptr [idcparm]
	call dword ptr [idcptr]
	mov  word ptr rc, ax
	mov  word ptr [rc+2], dx
	popa
   }
   return(rc);
}
//******************************************************************************
//******************************************************************************
BOOL __far __loadds __saveregs OSS_Irq_Handler(int irqnr)
{
  return (BOOL) CallOSS32(IDC32_IRQ, 0, irqnr, 0, 0, 0);
}
//******************************************************************************
//******************************************************************************
extern "C" ULONG  __cdecl __saveregs OSSIDC_EntryPoint(ULONG cmd, IDC16_PACKET FAR *packet)
{
 BOOL fPciDevice = TRUE;
 int  i;

   switch(cmd & 0xFFFF) 
   {
   case IDC16_INIT:
	break;

   case IDC16_EXIT:
	IDCTable.ProtIDCEntry = 0;
	break;

   case IDC16_SETIRQ:
   {
	IRQ *pIrq = pMakeIRQ((USHORT)packet->irq.irqnr);
	if(!pIrq) return FALSE;

	if(pIrq->bAddHandler(OSS_Irq_Handler) == FALSE) {
		DebugInt3();
		return FALSE;
	}

	if(pIrq->bEnableHandler(OSS_Irq_Handler) == FALSE) {
		DebugInt3();
		return FALSE;
	}
	return TRUE;
   }
   case IDC16_FREEIRQ:
   {
	IRQ *pIrq = getIRQ((USHORT)packet->irq.irqnr);
	if(!pIrq) return FALSE;

	if(pIrq->bDisableHandler(OSS_Irq_Handler) == FALSE) {
		DebugInt3();
		return FALSE;
	}
	return TRUE;
   }

   case IDC16_FIND_PNPDEVICE:
	fPciDevice = FALSE;
	//fall through	
   case IDC16_FIND_PCIDEVICE:
   {
    LDev_Resources* pResources;
    IDC_RESOURCE FAR *idcres = (IDC_RESOURCE FAR *)packet->finddev.pResource;

   	if(!pRM->bIsDevDetected(packet->finddev.devid, SEARCH_ID_DEVICEID, fPciDevice)) {
      		return 0;
   	}

   	pResources = pRM->pGetDevResources(packet->finddev.devid, SEARCH_ID_DEVICEID, fPciDevice);
   	if ((!pResources) || pResources->isEmpty()) {
      		return 0;
   	}
      	// Available device resources identified
	for(i=0;i<MAX_ISA_Dev_IO;i++) {
		idcres->io[i] = pResources->uIOBase[i];
		idcres->iolength[i] = pResources->uIOLength[i];
	}
	for(i=0;i<MAX_ISA_Dev_IRQ;i++) {
		idcres->irq[i] = pResources->uIRQLevel[i];
	}
	for(i=0;i<MAX_ISA_Dev_DMA;i++) {
		idcres->dma[i] = pResources->uDMAChannel[i];
	}
	for(i=0;i<MAX_ISA_Dev_MEM;i++) {
		idcres->mem[i] = pResources->uMemBase[i];
		idcres->memlength[i] = pResources->uMemLength[i];
	}
	delete pResources;
	return 1;
   }
   case IDC16_MALLOC:
   {
	LIN linaddr;
	ULONG near *addr16 = (ULONG near *)malloc((USHORT)packet->malloc.size+4);
	ULONG far *addr = (ULONG far *)addr16;

	if(addr16 == NULL) {
		return 0;
	}
	*addr = (ULONG)addr;
	if(DevHelp_VirtToLin(SELECTOROF(addr), OFFSETOF(addr), &linaddr)) {
		dprintf(("DevHelp_VirtToLin failed for %x:%x\n", SELECTOROF(addr), OFFSETOF(addr)));
		DebugInt3();
		return 0;
	}
	return linaddr;
   }
   case IDC16_FREE:
	free((void __near *)(void far *)packet->free.addr);
	break;
   case IDC16_VMALLOC:
	break;
   case IDC16_VMFREE:
	break;
   case IDC16_PROCESS:
   {
        PWAVESTREAM pStream;

	if(packet->process.type == IDC16_MIDI_IRQ) {
		MPU_401::processIrq(packet->process.streamid);
		break;
	}

	pStream = (PWAVESTREAM)FindActiveStream((packet->process.type == IDC16_WAVEOUT_IRQ) ? STREAM_WAVE_PLAY : STREAM_WAVE_CAPTURE,
                                                 packet->process.streamid);
	if(pStream) {
		pStream->Process();
		return 0;
	}
	dprintf(("Stream %lx not found or not active!", packet->process.streamid));
	break;
   }

   }
   return 0;
}
