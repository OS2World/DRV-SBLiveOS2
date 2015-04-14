/* $Id: signal.h,v 1.1 2000/04/23 14:55:33 ktk Exp $ */

#ifndef _LINUX_SIGNAL_H
#define _LINUX_SIGNAL_H

#define signal_pending(p) 0

#ifdef __KERNEL__

/*
 * These values of sa_flags are used only by the kernel as part of the
 * irq handling routines.
 *
 * SA_INTERRUPT is also used by the irq handling routines.
 * SA_SHIRQ is for shared interrupt support on PCI and EISA.
 */
#define SA_PROBE		SA_ONESHOT
#define SA_SAMPLE_RANDOM	SA_RESTART
#define SA_SHIRQ		0x04000000
#endif

#endif
