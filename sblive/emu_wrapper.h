/* $Id: emu_wrapper.h,v 1.2 2001/04/14 17:03:36 sandervl Exp $ */

#ifndef __EMU_WRAPPER_H
#define __EMU_WRAPPER_H

/* wrapper for 2.2 kernel */

#include <linux/wrapper.h>

#ifndef TARGET_OS2
#define vma_get_pgoff(v)	vma_get_offset(v)
#endif

#ifndef TARGET_OS2
#define DECLARE_WAITQUEUE(a, b)	struct wait_queue a = {b, NULL};
#endif

#ifdef TARGET_OS2
#define RSRCISIOREGION(dev,num) (!dev || ((dev)->resource[(num)].start != 0 && \
                                 ((dev)->resource[(num)].flags & PCI_BASE_ADDRESS_SPACE) == PCI_BASE_ADDRESS_SPACE_IO))
#define RSRCADDRESS(dev,num) ((dev)->resource[(num)].start)
#else
#define wait_queue_head_t	struct wait_queue *
#define init_waitqueue_head(a)	init_waitqueue(a)
#define RSRCADDRESS(dev,num)	((dev)->base_address[(num)])

#define RSRCISIOREGION(dev,num) (RSRCADDRESS(dev,num) != 0 && \
	(RSRCADDRESS(dev,num) & PCI_BASE_ADDRESS_SPACE) == PCI_BASE_ADDRESS_SPACE_IO)
#define init_MUTEX(a)		*(a) = MUTEX
#endif

#define UP_INODE_SEM(a)
#define DOWN_INODE_SEM(a)

#define GET_INODE_STRUCT()	struct inode *inode = file->f_dentry->d_inode

#define tasklet_struct		tq_struct 

#ifdef TARGET_OS2
void tasklet_hi_schedule(struct tasklet_struct *t);
#else
#define tasklet_hi_schedule(t)	queue_task((t), &tq_immediate); \
				mark_bh(IMMEDIATE_BH)
#endif

#define tasklet_init(t,f,d)	(t)->next = NULL; \
				(t)->sync = 0; \
				(t)->routine = (void (*)(void *))(f); \
				(t)->data = (void *)(d)


#ifdef TARGET_OS2
#define tasklet_unlock_wait(t)	
#else
#define tasklet_unlock_wait(t)	while (test_bit(0, &(t)->sync)) { }
#endif

#ifdef MODULE
#define __exit
#define module_init(x)		int init_module(void) { return x(); }
#define module_exit(x)		void cleanup_module(void) { x(); }
#endif


#ifdef TARGET_OS2
#define MODULE_DEVICE_TABLE(type,name)
#endif
#endif
