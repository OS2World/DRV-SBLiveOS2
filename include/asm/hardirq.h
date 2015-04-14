/* $Id: hardirq.h,v 1.1 2000/04/23 14:55:28 ktk Exp $ */

#ifndef __ASM_HARDIRQ_H
#define __ASM_HARDIRQ_H

extern unsigned int local_irq_count[1];

/*
 * Are we in an interrupt context? Either doing bottom half
 * or hardware interrupt processing?
 */
#define in_interrupt() asdfasdf

#define hardirq_trylock(cpu)	(local_irq_count[cpu] == 0)
#define hardirq_endlock(cpu)	do { } while (0)

#define hardirq_enter(cpu)	(local_irq_count[cpu]++)
#define hardirq_exit(cpu)	(local_irq_count[cpu]--)

#define synchronize_irq()	barrier()

#endif /* __ASM_HARDIRQ_H */
