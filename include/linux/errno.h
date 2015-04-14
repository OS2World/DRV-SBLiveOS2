/* $Id: errno.h,v 1.1 2000/04/23 14:55:30 ktk Exp $ */

#ifndef _LINUX_ERRNO_H
#define _LINUX_ERRNO_H

#include <asm/errno.h>

#ifdef __KERNEL__

/* Should never be seen by user programs */
#define ERESTARTSYS	512
#define ERESTARTNOINTR	513
#define ERESTARTNOHAND	514	/* restart if no handler.. */
#define ENOIOCTLCMD	515	/* No ioctl command */

#endif

#endif
