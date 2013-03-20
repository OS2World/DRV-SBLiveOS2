/* $Id: irq.c,v 1.1 2000/04/23 14:55:37 ktk Exp $ */

//******************************************************************************
// OS/2 implementation of Linux irq kernel services
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
#define INCL_NOPMAPI
#define INCL_DOSERRORS           // for ERROR_INVALID_FUNCTION
#include <os2.h>
#include <ossidc.h>
#include "irqos2.h"

unsigned long volatile jiffies = 0;

static IRQHANDLER_INFO irqHandlers[MAX_IRQS][MAX_SHAREDIRQS] = {0};
static ULONG           nrIrqHandlers[MAX_IRQS] = {0};
static ULONG           eoiIrq[MAX_IRQS] = {0};

//******************************************************************************
//******************************************************************************
int request_irq(unsigned int irq,
                void (*handler)(int, void *, void *),
	        unsigned long x0, const char *x1, void *x2)
{
 int i;
	if(irq > 0xF)
		return 0;

	for(i=0;i<MAX_SHAREDIRQS;i++) {
		if(irqHandlers[irq][i].handler == 0) {
			irqHandlers[irq][i].handler = handler;
			irqHandlers[irq][i].x0      = x0;
			irqHandlers[irq][i].x1      = (char *)x1;
			irqHandlers[irq][i].x2      = x2;
			nrIrqHandlers[irq]++;
			if(OSS32_SetIrq(irq, (ULONG)&oss_process_interrupt) == FALSE) {
				break;
			}
			return 0;
		}
	}
	dprintf(("request_irq: Unable to register irq handler for irq %d\n", irq));
	return 1;
}
//******************************************************************************
//******************************************************************************
void free_irq(unsigned int irq, void *userdata)
{
 int i;
	for(i=0;i<MAX_SHAREDIRQS;i++) {
		if(irqHandlers[irq][i].x2 == userdata) {
			irqHandlers[irq][i].handler = 0;
			irqHandlers[irq][i].x0      = 0;
			irqHandlers[irq][i].x1      = 0;
			irqHandlers[irq][i].x2      = 0;
			if(--nrIrqHandlers[irq] == 0) {
				OSS32_FreeIrq(irq);
			}
			break;
		}
	}
}
//******************************************************************************
//******************************************************************************
void eoi_irq(unsigned int irq)
{
	if(irq > 0xf) {
		DebugInt3();
		return;
	}
	eoiIrq[irq]++;
}
//******************************************************************************
//******************************************************************************
BOOL _cdecl oss_process_interrupt(int irq)
{
 BOOL rc;
 int  i;
	for(i=0;i<MAX_SHAREDIRQS;i++) {
		if(irqHandlers[irq][i].handler != 0) {
			irqHandlers[irq][i].handler(irq, irqHandlers[irq][i].x2, 0);
			rc = (eoiIrq[irq] > 0);
			if(rc) {
				eoiIrq[irq] = 0;
				return rc;
			}
		}
	}
	return FALSE;
}
//******************************************************************************
//******************************************************************************
