/* $Id: sbvsd.h,v 1.1 2000/04/23 14:55:27 ktk Exp $ */

//******************************************************************************
// Header with hardware ID definitions
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
#ifndef __SBVSD_H__
#define __SBVSD_H__

// known cards, others can be added via an .rc file

#define MAUDIO                  1
#define SB                      2
#define SB_PRO                  3
#define SB_16                   4
#define PAS_16                  5
#define FOXGLOVE                6
#define SPEAKER                 7
#define ULTRASND                66
#define SB_LIVE                 77
#define SB_LIVE_ID              "77"

// values for RC file
#define  STATIC_RATE             0L
#define  BEGIN_CONTINUOUS        1L
#define  END_CONTINUOUS          2L

// Classes used for resource management
#define  PCM_CLASS               1
#define  MIDI_CLASS              2

#endif //__SBVSD_H__
