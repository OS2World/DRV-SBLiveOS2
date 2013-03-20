/* $Id: osssound.h,v 1.1 2000/04/23 14:55:26 ktk Exp $ */

//******************************************************************************
// Header for oss sound constants
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
#ifndef __OSSSOUND_H__
#define __OSSSOUND_H__

#ifdef __cplusplus
extern "C" {
#endif

#define OSS_IDMASK      	0xF
#define OSS_RWMASK      	0xF0

#define OSS_DSPID		0
#define OSS_SYNTHID		1
#define OSS_MIDIID		2
#define OSS_MIXERID		3
#define OSS_SPECIALID		4
#define OSS_MAX_DEVICES 	5

#define OSS_STREAM_READ 	0x10
#define OSS_STREAM_WRITE	0x20

#define OSS_STREAM_WAVEOUT	(OSS_STREAM_WRITE|OSS_DSPID)
#define OSS_STREAM_WAVEIN	(OSS_STREAM_READ|OSS_DSPID)
#define OSS_STREAM_MIDIOUT	(OSS_STREAM_WRITE|OSS_MIDIID)
#define OSS_STREAM_MIDIIN	(OSS_STREAM_READ|OSS_MIDIID)

#ifndef TARGET_OS2_16
extern struct file_operations oss_devices[OSS_MAX_DEVICES];
#endif

#ifdef __cplusplus
}
#endif

#endif
