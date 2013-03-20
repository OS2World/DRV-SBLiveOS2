/* $Id: irqos2.h,v 1.1 2000/04/23 14:55:26 ktk Exp $ */

//******************************************************************************
// Header for irq definitions/structures
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
#ifndef __IRQ_H__
#define __IRQ_H__

#define MAX_SHAREDIRQS		4
#define MAX_IRQS		16

typedef void (NEAR * IRQHANDLER)(int, void *, void *);

typedef struct {
  IRQHANDLER handler;
  ULONG x0;
  char *x1;
  void *x2;
} IRQHANDLER_INFO;

#ifdef __cplusplus
extern "C" {
#endif

BOOL _cdecl oss_process_interrupt(int irq);

#ifdef __cplusplus
}
#endif

#endif
