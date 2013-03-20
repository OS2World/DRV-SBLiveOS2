/* $Id: sound.c,v 1.6 2001/09/28 12:10:07 sandervl Exp $ */

//******************************************************************************
// MMPM/2 to OSS interface translation layer
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
//Note: Opening and closing the mixer device for each mixer command might
//      not be a good idea if the SB Live code changes. (right now it hardly
//      matters)
#include "hwaccess.h"
#include <linux/init.h>
#include <linux/poll.h>
#include <asm/uaccess.h>
#include <asm/hardirq.h>
#include "..\sblive\icardmid.h"
#include "..\sblive\cardmi.h"
#include "..\sblive\midi.h"

#define LINUX
#include <ossidc.h>
#include <stacktoflat.h>

struct file_operations oss_devices[OSS_MAX_DEVICES] = {0};

//******************************************************************************
//******************************************************************************
int register_sound_special(struct file_operations *fops, int unit)
{
	memcpy(&oss_devices[OSS_SPECIALID], fops, sizeof(struct file_operations));
	return OSS_SPECIALID;
}
//******************************************************************************
//******************************************************************************
int register_sound_mixer(struct file_operations *fops, int dev)
{
	memcpy(&oss_devices[OSS_MIXERID], fops, sizeof(struct file_operations));
	return OSS_MIXERID;
}
//******************************************************************************
//******************************************************************************
int register_sound_midi(struct file_operations *fops, int dev)
{
	memcpy(&oss_devices[OSS_MIDIID], fops, sizeof(struct file_operations));
	return OSS_MIDIID;
}
//******************************************************************************
//******************************************************************************
int register_sound_dsp(struct file_operations *fops, int dev)
{
	memcpy(&oss_devices[OSS_DSPID], fops, sizeof(struct file_operations));
	return OSS_DSPID;
}
//******************************************************************************
//******************************************************************************
int register_sound_synth(struct file_operations *fops, int dev)
{
	memcpy(&oss_devices[OSS_SYNTHID], fops, sizeof(struct file_operations));
	return OSS_SYNTHID;
}
//******************************************************************************
//******************************************************************************
void unregister_sound_special(int unit)
{
	memset(&oss_devices[OSS_SPECIALID], 0, sizeof(struct file_operations));
}
//******************************************************************************
//******************************************************************************
void unregister_sound_mixer(int unit)
{
	memset(&oss_devices[OSS_MIXERID], 0, sizeof(struct file_operations));
}
//******************************************************************************
//******************************************************************************
void unregister_sound_midi(int unit)
{
	memset(&oss_devices[OSS_MIDIID], 0, sizeof(struct file_operations));
}
//******************************************************************************
//******************************************************************************
void unregister_sound_dsp(int unit)
{
	memset(&oss_devices[OSS_DSPID], 0, sizeof(struct file_operations));
}
//******************************************************************************
//******************************************************************************
void unregister_sound_synth(int unit)
{
	memset(&oss_devices[OSS_SYNTHID], 0, sizeof(struct file_operations));
}
//******************************************************************************
//******************************************************************************
ULONG OSS32_StreamOpen(ULONG streamtype)
{
struct inode ossinode;
struct file  ossfile;
ULONG        ossid = streamtype & OSS_IDMASK;

  ossinode.i_rdev = ossid;
  ossfile.f_flags = 0;

  switch(streamtype) {
  case OSS_STREAM_WAVEOUT:
  	ossfile.f_mode = FMODE_WRITE;
	break;
  case OSS_STREAM_WAVEIN:
  	ossfile.f_mode = FMODE_READ;
	break;
  case OSS_STREAM_MIDIOUT:
  	ossfile.f_mode = FMODE_WRITE;
	break;
  case OSS_STREAM_MIDIIN:
  	ossfile.f_mode = FMODE_READ;
	break;
  }

  if(!oss_devices[ossid].open) {
	return 0;
  }
  if(oss_devices[ossid].open(&ossinode, &ossfile)) {
	DebugInt3();
	return 0;
  }
  return (ULONG)ossfile.private_data;
}
//******************************************************************************
//******************************************************************************
ULONG OSS32_StreamClose(ULONG streamtype, ULONG streamid)
{
struct inode ossinode;
struct file  ossfile;
ULONG        ossid = streamtype & OSS_IDMASK;

  ossinode.i_rdev = ossid;
  ossfile.private_data = (void *)streamid;
  ossfile.f_flags = 0;

  switch(streamtype) {
  case OSS_STREAM_WAVEOUT:
  	ossfile.f_mode = FMODE_WRITE;
	break;
  case OSS_STREAM_WAVEIN:
  	ossfile.f_mode = FMODE_READ;
	break;
  case OSS_STREAM_MIDIOUT:
  	ossfile.f_mode = FMODE_WRITE;
	break;
  case OSS_STREAM_MIDIIN:
  	ossfile.f_mode = FMODE_READ;
	break;
  }
  if(!oss_devices[ossid].release) {
	return 0;
  }
  if(oss_devices[ossid].release(&ossinode, &ossfile)) {
	DebugInt3();
	return 0;
  }
  return 0;
}
//******************************************************************************
//******************************************************************************
ULONG OSS32_StreamIOCtl(ULONG streamtype, ULONG streamid, ULONG cmd, char NEAR *buffer)
{
 struct inode ossinode;
 struct file  ossfile;
 ULONG        ossid = streamtype & OSS_IDMASK, newcmd = -1, fraginfo;
 count_info   cinfo;
 char NEAR   *tmpbuf;

  ossinode.i_rdev = ossid;
  ossfile.private_data = (void *)streamid;
  ossfile.f_flags = 0;

  switch(streamtype) {
  case OSS_STREAM_WAVEOUT:
  	ossfile.f_mode = FMODE_WRITE;
	break;
  case OSS_STREAM_WAVEIN:
  	ossfile.f_mode = FMODE_READ;
	break;
  case OSS_STREAM_MIDIOUT:
  	ossfile.f_mode = FMODE_WRITE;
	break;
  case OSS_STREAM_MIDIIN:
  	ossfile.f_mode = FMODE_READ;
	break;
  }

  if(!oss_devices[ossid].ioctl) {
	return 0;
  }
  switch(cmd) {
  case IOCTL_GETPOS:
	if(streamtype == OSS_STREAM_WAVEOUT) {
		newcmd = SNDCTL_DSP_GETOPTR;
	}
	else	newcmd = SNDCTL_DSP_GETIPTR;
#ifdef KEE
	tmpbuf = (char NEAR *)&cinfo;
#else
	tmpbuf = (char NEAR *)__StackToFlat((ULONG)&cinfo);
#endif
	break;

  case IOCTL_SETFRAGMENT:
  {
   ULONG fragsize;

	newcmd   = SNDCTL_DSP_SETFRAGMENT;

	fragsize = *(ULONG NEAR *)buffer; //fragsize
  	fraginfo = 0;
  	while(fragsize) {
		fragsize >>= 1;
		fraginfo++;
  	} 
  	fraginfo--;

#ifdef KEE
	tmpbuf = (char NEAR *)&fraginfo;
#else
  	tmpbuf = (char NEAR *)__StackToFlat((ULONG)&fraginfo);
#endif
  	break;
  }	
  default:
	DebugInt3();
	return 0;
  }
  if(oss_devices[ossid].ioctl(&ossinode, &ossfile, newcmd, (ULONG)tmpbuf)) {
	return 0;
  }
  switch(cmd) {
  case IOCTL_GETPOS:
	*(ULONG NEAR *)buffer = cinfo.bytes;
	break;
  }
  return 1;
}
//******************************************************************************
//******************************************************************************
ULONG OSS32_StreamSetFormat(ULONG streamtype, ULONG streamid, ULONG cmd, FORMAT_INFO NEAR *pFormatInfo)
{
 struct inode ossinode;
 struct file  ossfile;
 ULONG        ossid = streamtype & OSS_IDMASK;

  ossinode.i_rdev = ossid;
  ossfile.private_data = (void *)streamid;
  ossfile.f_flags = 0;

  switch(streamtype) {
  case OSS_STREAM_WAVEOUT:
  	ossfile.f_mode = FMODE_WRITE;
	break;
  case OSS_STREAM_WAVEIN:
  	ossfile.f_mode = FMODE_READ;
	break;
  case OSS_STREAM_MIDIOUT:
  	ossfile.f_mode = FMODE_WRITE;
	break;
  case OSS_STREAM_MIDIIN:
  	ossfile.f_mode = FMODE_READ;
	break;
  }

  if(!oss_devices[ossid].ioctl) {
	return 0;
  }
  if(oss_devices[ossid].ioctl(&ossinode, &ossfile, SNDCTL_DSP_SPEED, (ULONG)&pFormatInfo->ulSampleRate)) {
	return 0;
  }
  if(oss_devices[ossid].ioctl(&ossinode, &ossfile, SNDCTL_DSP_CHANNELS, (ULONG)&pFormatInfo->ulNumChannels)) {
	return 0;
  }
  if(oss_devices[ossid].ioctl(&ossinode, &ossfile, SNDCTL_DSP_SETFMT, (ULONG)&pFormatInfo->ulBitsPerSample)) {
	return 0;
  }

  return 1;
}
//******************************************************************************
//******************************************************************************
ULONG OSS32_StreamTrigger(ULONG streamtype, ULONG streamid, ULONG NEAR *fStart)
{
 struct inode ossinode;
 struct file  ossfile;
 ULONG        ossid = streamtype & OSS_IDMASK;
 ULONG        cmd;

  ossinode.i_rdev = ossid;
  ossfile.private_data = (void *)streamid;
  ossfile.f_flags = 0;

  switch(streamtype) {
  case OSS_STREAM_WAVEOUT:
  	ossfile.f_mode = FMODE_WRITE;
	*fStart = (*fStart) ? PCM_ENABLE_OUTPUT : 0; 
	cmd = SNDCTL_DSP_SETTRIGGER;
	break;
  case OSS_STREAM_WAVEIN:
  	ossfile.f_mode = FMODE_READ;
	*fStart = (*fStart) ? PCM_ENABLE_INPUT : 0; 
	cmd = SNDCTL_DSP_SETTRIGGER;
	break;
  case OSS_STREAM_MIDIOUT:
  	ossfile.f_mode = FMODE_WRITE;
	break;
  case OSS_STREAM_MIDIIN:
  	ossfile.f_mode = FMODE_READ;
	break;
  }

  if(!oss_devices[ossid].ioctl) {
	return 0;
  }
  if(ossfile.f_mode == FMODE_READ && *fStart == PCM_ENABLE_INPUT) {
	//need to call poll to really start the stream
	int rc;
	struct poll_table_struct poll = {0};

	rc = oss_devices[ossid].poll(&ossfile, &poll);
	if(rc & POLLERR)
		return 0;
	return 1;
  }
  else
  if(oss_devices[ossid].ioctl(&ossinode, &ossfile, cmd, (ULONG)fStart)) {
	return 0;
  }
  return 1;
}
//******************************************************************************
//******************************************************************************
ULONG OSS32_StreamReset(ULONG streamtype, ULONG streamid)
{
 struct inode ossinode;
 struct file  ossfile;
 ULONG        ossid = streamtype & OSS_IDMASK;
 ULONG        cmd;

  ossinode.i_rdev = ossid;
  ossfile.private_data = (void *)streamid;
  ossfile.f_flags = 0;

  switch(streamtype) {
  case OSS_STREAM_WAVEOUT:
  	ossfile.f_mode = FMODE_WRITE;
	cmd = SNDCTL_DSP_RESET;
	break;
  case OSS_STREAM_WAVEIN:
  	ossfile.f_mode = FMODE_READ;
	cmd = SNDCTL_DSP_RESET;
	break;
  case OSS_STREAM_MIDIOUT:
  	ossfile.f_mode = FMODE_WRITE;
	break;
  case OSS_STREAM_MIDIIN:
  	ossfile.f_mode = FMODE_READ;
	break;
  }

  if(!oss_devices[ossid].ioctl) {
	return 0;
  }
  if(oss_devices[ossid].ioctl(&ossinode, &ossfile, cmd, 0)) {
	return 0;
  }
  return 1;
}
//******************************************************************************
//******************************************************************************
ULONG OSS32_StreamAddBuffer(ULONG streamtype, ULONG streamid, ULONG buffer, ULONG size)
{
 struct inode   ossinode;
 struct file    ossfile;
 struct dentry  ossdentry;
 ULONG          ossid = streamtype & OSS_IDMASK;
 audio_buf_info info;
 char NEAR     *tmpbuf;
 ULONG          cmd;
 int            transferred;

  ossinode.i_rdev = ossid;
  ossfile.private_data = (void *)streamid;
  ossfile.f_flags = 0;
  ossfile.f_dentry = &ossdentry;
  ossdentry.d_inode = &ossinode;

  switch(streamtype) {
  case OSS_STREAM_WAVEOUT:
  	ossfile.f_mode = FMODE_WRITE;
	cmd = SNDCTL_DSP_GETOSPACE;
	break;
  case OSS_STREAM_WAVEIN:
  	ossfile.f_mode = FMODE_READ;
	cmd = SNDCTL_DSP_GETISPACE;
	break;
  case OSS_STREAM_MIDIOUT:
  	ossfile.f_mode = FMODE_WRITE;
	break;
  case OSS_STREAM_MIDIIN:
  	ossfile.f_mode = FMODE_READ;
	break;
  }

  if(!oss_devices[ossid].write || !oss_devices[ossid].read || !oss_devices[ossid].ioctl) {
	return 0;
  }
  //check how much room is left in the circular dma buffer
#ifdef KEE
  tmpbuf = (char NEAR *)&info;
#else
  tmpbuf = (char NEAR *)__StackToFlat((ULONG)&info);
#endif
  if(oss_devices[ossid].ioctl(&ossinode, &ossfile, cmd, (ULONG)tmpbuf)) {
	return 0;
  }
  size = min(size, info.bytes);

  if(size == 0) {
	return 0; //no room left, fail
  }

  if(streamtype == OSS_STREAM_WAVEIN) {
	transferred = oss_devices[ossid].read(&ossfile, (char *)buffer, size, &ossfile.f_pos);
  }
  else  transferred = oss_devices[ossid].write(&ossfile, (char *)buffer, size, &ossfile.f_pos);
  if(transferred < 0) {
	return 0;
  }
  return transferred;
}
//******************************************************************************
//******************************************************************************
ULONG OSS32_StreamGetSpace(ULONG streamtype, ULONG streamid)
{
 struct inode   ossinode;
 struct file    ossfile;
 struct dentry  ossdentry;
 ULONG          ossid = streamtype & OSS_IDMASK;
 audio_buf_info info;
 char NEAR     *tmpbuf;
 ULONG          cmd;
 int            transferred;

  ossinode.i_rdev = ossid;
  ossfile.private_data = (void *)streamid;
  ossfile.f_flags = 0;
  ossfile.f_dentry = &ossdentry;
  ossdentry.d_inode = &ossinode;

  switch(streamtype) {
  case OSS_STREAM_WAVEOUT:
  	ossfile.f_mode = FMODE_WRITE;
	cmd = SNDCTL_DSP_GETOSPACE;
	break;
  case OSS_STREAM_WAVEIN:
  	ossfile.f_mode = FMODE_READ;
	cmd = SNDCTL_DSP_GETISPACE;
	break;
  case OSS_STREAM_MIDIOUT:
  	ossfile.f_mode = FMODE_WRITE;
	break;
  case OSS_STREAM_MIDIIN:
  	ossfile.f_mode = FMODE_READ;
	break;
  }

  if(!oss_devices[ossid].write || !oss_devices[ossid].read || !oss_devices[ossid].ioctl) {
	return 0;
  }
  //check how much room is left in the circular dma buffer
#ifdef KEE
  tmpbuf = (char NEAR *)&info;
#else
  tmpbuf = (char NEAR *)__StackToFlat((ULONG)&info);
#endif
  if(oss_devices[ossid].ioctl(&ossinode, &ossfile, cmd, (ULONG)tmpbuf)) {
	return 0;
  }
  return info.bytes;
}
//******************************************************************************
//******************************************************************************
ULONG OSS32_SetVolume(ULONG streamtype, ULONG streamid, ULONG cmd, ULONG volume)
{
 struct inode   ossinode;
 struct file    ossfile;
 struct dentry  ossdentry;
 ULONG          ossid = streamtype & OSS_IDMASK;
 char NEAR     *tmpbuf;
 ULONG          ioctl;

  ossinode.i_rdev = OSS_MIXERID;
  ossfile.private_data = (void *)streamid;
  ossfile.f_flags = 0;
  ossfile.f_dentry = &ossdentry;
  ossfile.f_mode = FMODE_WRITE;
  ossdentry.d_inode = &ossinode;

  switch(cmd) {
  case MIX_SETMASTERVOL:
	ioctl = SOUND_MIXER_WRITE_VOLUME;
	break;
  case MIX_SETWAVEVOL:
	ioctl = SOUND_MIXER_WRITE_PCM;
	break;
  case MIX_SETMIDIVOL:
//	ioctl = 
	break;
  case MIX_SETINPUTSRC:
	ioctl = SOUND_MIXER_WRITE_RECSRC; //todo
	switch(volume)
	{
	case MIX_RECSRC_MIC:
		volume = SOUND_MASK_MIC;
		break;
	case MIX_RECSRC_CD:
		volume = SOUND_MASK_CD;
		break;
	case MIX_RECSRC_LINE:
		volume = SOUND_MASK_LINE;
		break;
	case MIX_RECSRC_VIDEO:
		volume = SOUND_MASK_VIDEO;
		break;
	case MIX_RECSRC_MIXER:
		volume = SOUND_MASK_MONITOR;
		break;
	case MIX_RECSRC_AUX:
		volume = SOUND_MASK_LINE1;
		break;
	default:
		DebugInt3();
		break;
	}
	break;
  case MIX_SETINPUTGAIN:
	ioctl = SOUND_MIXER_WRITE_RECLEV;
	break;
 
  case MIX_SETLINEINVOL:
  case MIX_SETLINEINMUTE:
	ioctl = SOUND_MIXER_WRITE_LINE;
	break;

  case MIX_SETMICVOL:
  case MIX_SETMICMUTE:
	ioctl = SOUND_MIXER_WRITE_MIC;
	break;

  case MIX_SETCDVOL:
  case MIX_SETCDMUTE:
	ioctl = SOUND_MIXER_WRITE_CD;
	break;

  case MIX_SETVIDEOVOL:
  case MIX_SETVIDEOMUTE:
	ioctl = SOUND_MIXER_WRITE_VIDEO;
	break;

  case MIX_SETPCMVOL:
  case MIX_SETPCMMUTE:
	ioctl = SOUND_MIXER_WRITE_LINE2;
	break;

  case MIX_SETAUXVOL:
  case MIX_SETAUXMUTE:
	ioctl = SOUND_MIXER_WRITE_LINE1;
  	break;

  case MIX_SETBASS:
	ioctl = SOUND_MIXER_WRITE_BASS;
  	break;
  case MIX_SETTREBLE:
	ioctl = SOUND_MIXER_WRITE_TREBLE;
  	break;
  case MIX_SETSPDIFVOL:
//	ioctl = 
	break;
  case MIX_SETSPDIFMUTE:
//	ioctl = 
	break;
  }
  if(!oss_devices[OSS_MIXERID].ioctl || !oss_devices[OSS_MIXERID].open || !oss_devices[OSS_MIXERID].release) {
	return 0;
  }

  if(oss_devices[OSS_MIXERID].open(&ossinode, &ossfile)) {
	return 0;
  }

#ifdef KEE
  tmpbuf = (char NEAR *)&volume;
#else
  tmpbuf = (char NEAR *)__StackToFlat((ULONG)&volume);
#endif
  if(oss_devices[OSS_MIXERID].ioctl(&ossinode, &ossfile, ioctl, (ULONG)tmpbuf)) {
  	oss_devices[OSS_MIXERID].release(&ossinode, &ossfile);
	return 0;
  }
  oss_devices[OSS_MIXERID].release(&ossinode, &ossfile);
  return 1;
}
//******************************************************************************
//******************************************************************************
unsigned long OSS32_StreamMidiWrite(unsigned long streamid, unsigned long midiByte)
{
 struct emu10k1_mididevice *midi_dev = (struct emu10k1_mididevice *)streamid;

  return emu10k1_mpu_write_data(midi_dev->card, (u8)midiByte) == 0;
}
//******************************************************************************
//******************************************************************************
unsigned long OSS32_StreamMidiRead(unsigned long streamid, char near *buffer, unsigned long bufsize)
{
 struct emu10k1_mididevice *midi_dev = (struct emu10k1_mididevice *)streamid;
 int count = 0;
 u8  MPUIvalue;

  while(TRUE) {
  	if(emu10k1_mpu_read_data(midi_dev->card, &MPUIvalue) == 0) {
		buffer[count] = MPUIvalue;
		count++;
	}
	else	break;

	if(count >= bufsize) {
		break;
	}
  }
  return count;
}
//******************************************************************************
//******************************************************************************
