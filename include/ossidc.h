/* $Id: ossidc.h,v 1.6 2001/09/28 12:13:05 sandervl Exp $ */

//******************************************************************************
// Header for idc definitions & declarations
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
#ifndef __OSSIDC_H__
#define __OSSIDC_H__

#include <ossdefos2.h>
#include <osssound.h>

typedef BOOL (FAR48 __cdecl *IDC16_HANDLER)(ULONG cmd, ULONG param);

extern IDC16_HANDLER idc16_PddHandler;

//IDC communication packet
typedef struct 
{
  union {
	struct {
		ULONG	handler16;	//16:16 address of pdd idc handler
	} init;
	struct {
		ULONG	irqnr;
	} irq;
	struct {
		ULONG	streamtype;
	} open;
	struct {
		ULONG	streamtype;
		ULONG	streamid;
	} close;
	struct {
		ULONG	streamtype;
		ULONG	streamid;
		ULONG   cmd;
		ULONG   ioctl_param;
	} ioctl;
	struct {
		ULONG	streamtype;
		ULONG	streamid;
	} startstop;
	struct {
		ULONG   streamtype;
		ULONG   streamid;
		ULONG 	buffer;		//0:32 address of mmpm/2 buffer
		ULONG 	size;
	} buffer;
	struct {
		ULONG   streamtype;
		ULONG   streamid;
		ULONG	cmd;
		ULONG	volume;
	} mixer;
	struct {
		ULONG   streamtype;
		ULONG   streamid;
		ULONG   midiByte;
	} midiwrite;
	struct {
		ULONG   streamtype;
		ULONG   streamid;
		ULONG   buffer;
		ULONG   bufsize;
	} midiread;
	struct {
		ULONG   streamtype;
		ULONG   streamid;
	} getspace;
	struct {
		ULONG 	param1;
		ULONG 	param2;
		ULONG 	param3;
		ULONG 	param4;
		ULONG   fileid;
	};
  };
} IDC32_PACKET, NEAR *PIDC32_PACKET;

typedef struct 
{
  union {
	struct {
		ULONG	devid;
		ULONG   pResource; 	//16:16 resource structure pointer	   
	} finddev;
	struct {
		ULONG	irqnr;
	} irq;
        struct {
		ULONG   size;
        } malloc;
	struct {
		ULONG   addr;		//16:16 address returned by malloc at start of memory block
	} free;
	struct {
		ULONG	type;
		ULONG   streamid;
	} process;
	struct {
		ULONG 	param1;
		ULONG 	param2;
	};
  };
} IDC16_PACKET;

//IDC commands (16->32 bits)
#define IDC32_INIT			0
#define IDC32_EXIT			1
#define IDC32_IRQ			2
#define IDC32_STREAM_OPEN               3
#define IDC32_STREAM_CLOSE              4
#define IDC32_STREAM_ADDBUFFER		5
#define IDC32_STREAM_START		6
#define IDC32_STREAM_STOP		7
#define IDC32_STREAM_RESET              8
#define IDC32_STREAM_PAUSE		9
#define IDC32_STREAM_IOCTL              10
#define IDC32_STREAM_MIXER		11
#define IDC32_MIDI_WRITE                12
#define IDC32_MIDI_READ                 13
#define IDC32_STREAM_GETSPACE		14

#define MIX_SETMASTERVOL		0
#define MIX_SETWAVEVOL			1
#define MIX_SETMIDIVOL			2
#define MIX_SETINPUTSRC			3
#define MIX_SETINPUTGAIN		4
#define MIX_SETLINEINVOL		5
#define MIX_SETLINEINMUTE		6
#define MIX_SETMICVOL			7
#define MIX_SETMICMUTE			8
#define MIX_SETCDVOL			9
#define MIX_SETCDMUTE			10
#define MIX_SETSPDIFVOL			11
#define MIX_SETSPDIFMUTE		12
#define MIX_SETBASS                     13
#define MIX_SETTREBLE                   14
#define MIX_SETVIDEOVOL			15
#define MIX_SETVIDEOMUTE                16
#define MIX_SETAUXVOL                   17
#define MIX_SETAUXMUTE                  18
#define MIX_SETPCMVOL                   19
#define MIX_SETPCMMUTE                  20

#define MAKE_VOLUME_LR(l, r)		((r << 8) | l)

#define GET_VOLUME_R(vol)               (vol >> 8)
#define GET_VOLUME_L(vol)               (vol & 0xff)

#define MIX_RECSRC_MIC			0
#define MIX_RECSRC_CD			1
#define MIX_RECSRC_LINE			2
#define MIX_RECSRC_VIDEO		3
#define MIX_RECSRC_MIXER	 	4
#define MIX_RECSRC_AUX		 	5

#define IDC16_INIT			0
#define IDC16_EXIT			1
#define IDC16_SETIRQ			2
#define IDC16_FREEIRQ			3
#define IDC16_WAVEOUT_BUFFER_DONE	4
#define IDC16_WAVEIN_BUFFER_DONE	5
#define IDC16_FIND_PCIDEVICE            6
#define IDC16_FIND_PNPDEVICE            7
#define IDC16_MALLOC                    8
#define IDC16_FREE                      9
#define IDC16_VMALLOC			10
#define IDC16_VMFREE			11
#define IDC16_PROCESS                   12

#define IDC16_WAVEIN_IRQ		0
#define IDC16_WAVEOUT_IRQ		1
#define IDC16_MIDI_IRQ			2

#define MAX_RES_IRQ	2
#define MAX_RES_DMA	2
#define MAX_RES_IO	8
#define MAX_RES_MEM	4

#define SOUND_MIXER_VOLUME	0
#define SOUND_MIXER_BASS	1
#define SOUND_MIXER_TREBLE	2
#define SOUND_MIXER_SYNTH	3
#define SOUND_MIXER_PCM		4
#define SOUND_MIXER_SPEAKER	5
#define SOUND_MIXER_LINE	6
#define SOUND_MIXER_MIC		7
#define SOUND_MIXER_CD		8
#define SOUND_MIXER_IMIX	9	/*  Recording monitor  */
#define SOUND_MIXER_ALTPCM	10
#define SOUND_MIXER_RECLEV	11	/* Recording level */
#define SOUND_MIXER_IGAIN	12	/* Input gain */
#define SOUND_MIXER_OGAIN	13	/* Output gain */
#define SOUND_MIXER_LINE1	14	/* Input source 1  (aux1) */
#define SOUND_MIXER_LINE2	15	/* Input source 2  (aux2) */
#define SOUND_MIXER_LINE3	16	/* Input source 3  (line) */
#define SOUND_MIXER_DIGITAL1	17	/* Digital (input) 1 */
#define SOUND_MIXER_DIGITAL2	18	/* Digital (input) 2 */
#define SOUND_MIXER_DIGITAL3	19	/* Digital (input) 3 */
#define SOUND_MIXER_PHONEIN	20	/* Phone input */
#define SOUND_MIXER_PHONEOUT	21	/* Phone output */
#define SOUND_MIXER_VIDEO	22	/* Video/TV (audio) in */
#define SOUND_MIXER_RADIO	23	/* Radio in */
#define SOUND_MIXER_MONITOR	24	/* Monitor (usually mic) volume */
#define SOUND_MIXER_NRDEVICES	25

typedef struct 
{
	USHORT irq[MAX_RES_IRQ];
	USHORT dma[MAX_RES_DMA];
	USHORT io[MAX_RES_IO];
	USHORT iolength[MAX_RES_IO];
	ULONG  mem[MAX_RES_MEM];
	ULONG  memlength[MAX_RES_MEM];
} IDC_RESOURCE;

typedef struct {
   	ULONG  ulSampleRate;
   	ULONG  ulBitsPerSample;
   	ULONG  ulNumChannels;
   	ULONG  ulDataType;
} FORMAT_INFO;

#define IOCTL_SETFORMAT		0
#define IOCTL_GETPOS            1
#define IOCTL_SETFRAGMENT       2

#ifdef __cplusplus
extern "C" {
#endif

//32 bits IDC procedures
//16 bits pdd calls this during init. OSS_InitDriver calls init_module
BOOL  OSS32_InitDriver();
void  OSS32_RemoveDriver();
BOOL  OSS32_SetIrq(int irq, ULONG handler);
BOOL  OSS32_FreeIrq(int irq);

ULONG OSS32_StreamOpen(ULONG streamtype);
ULONG OSS32_StreamClose(ULONG streamtype, ULONG streamid);
ULONG OSS32_StreamTrigger(ULONG streamtype, ULONG streamid, ULONG __near *fStart);
ULONG OSS32_StreamReset(ULONG  streamtype, ULONG streamid);
ULONG OSS32_StreamIOCtl(ULONG streamtype, ULONG streamid, ULONG cmd, char __near *buffer);
ULONG OSS32_StreamSetFormat(ULONG streamtype, ULONG streamid, ULONG cmd, FORMAT_INFO __near *pFormatInfo);
ULONG OSS32_StreamAddBuffer(ULONG streamtype, ULONG streamid, ULONG buffer, ULONG size);
ULONG OSS32_SetVolume(ULONG streamtype, ULONG streamid, ULONG cmd, ULONG volume);
ULONG OSS32_StreamMidiWrite(ULONG streamid, ULONG midiByte);
ULONG OSS32_StreamMidiRead(ULONG streamid, char NEAR *buffer, ULONG bufsize);
ULONG OSS32_StreamGetSpace(ULONG streamtype, ULONG streamid);

//Sets file id in current task structure
ULONG OSS32_SetFileId(ULONG fileid);

#ifdef TARGET_OS216
BOOL  OSS16_StartStream(STREAM *stream);
BOOL  OSS16_PauseStream(STREAM *stream);
BOOL  OSS16_StopStream(STREAM *stream);
BOOL  OSS16_StreamReset(STREAM *stream);
BOOL  OSS16_StreamSetFormat(STREAM *stream, ULONG param1);
ULONG OSS16_StreamAddBuffer(STREAM *stream, ULONG buffer, ULONG size);
BOOL  OSS16_StreamGetPos(STREAM *stream, ULONG FAR *pos);
BOOL  OSS16_SetMasterVol(STREAM *stream, ULONG volume);
BOOL  OSS16_SetWaveOutVol(STREAM *stream, ULONG volume);
BOOL  OSS16_SetVolume(STREAM *stream, USHORT line, ULONG volume);
BOOL  OSS16_SetGlobalVol(ULONG ulSysFileNum, USHORT line, ULONG volume);
BOOL  OSS16_StreamSetFragment(STREAM *stream, ULONG fragsize);
ULONG OSS16_StreamGetSpace(STREAM *stream);
BOOL  OSS16_AttachToPdd();
void  OSS16_DetachFromPdd();
ULONG OSS16_OpenStream(STREAM *stream);
void  OSS16_CloseStream(STREAM *stream);
typedef enum {
  MIDI_RECEIVE = 0,
  MIDI_SEND
} MIDITYPE;
ULONG OSS16_OpenMidiStream(MIDITYPE midiType);
void  OSS16_CloseMidiStream(MIDITYPE midiType, ULONG streamid);
BOOL  OSS16_WriteMidiByte(ULONG streamid, BYTE midiByte);
int   OSS16_ReadMidiBytes(ULONG streamid, char far *buffer, int bufsize);

#endif //TARGET_OS216

//16 bits IDC procedures
BOOL CallOSS16(ULONG cmd, ULONG param1, ULONG param2);

//Used to call into the 32 bits OSS driver
ULONG CallOSS32(USHORT cmd, ULONG fileid, ULONG param1, ULONG param2, ULONG param3, ULONG param4);

#ifdef __cplusplus
}
#endif

#endif
