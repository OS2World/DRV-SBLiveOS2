/* $Id: delay.h,v 1.2 2000/05/28 16:50:43 sandervl Exp $ */

#ifndef _LINUX_DELAY_H
#define _LINUX_DELAY_H

/*
 * Copyright (C) 1993 Linus Torvalds
 *
 * Delay routines, using a pre-computed "loops_per_second" value.
 */

extern unsigned long loops_per_sec;

#include <asm/delay.h>

/*
 * Using udelay() for intervals greater than a few milliseconds can
 * risk overflow for high loops_per_sec (high bogomips) machines. The
 * mdelay() provides a wrapper to prevent this.  For delays greater
 * than MAX_UDELAY_MS milliseconds, the wrapper is used.  Architecture
 * specific values can be defined in asm-???/delay.h as an override.
 * The 2nd mdelay() definition ensures GCC will optimize away the 
 * while loop for the common cases where n <= MAX_UDELAY_MS  --  Paul G.
 */

#ifndef MAX_UDELAY_MS
#define MAX_UDELAY_MS	5
#endif

void iodelay(unsigned long);
#pragma aux iodelay parm nomemory [ecx] modify nomemory exact [eax ecx];

#define mdelay(n) iodelay(n*2)

#endif /* defined(_LINUX_DELAY_H) */
