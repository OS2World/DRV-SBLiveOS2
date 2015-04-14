/* $Id: delay.h,v 1.1 2000/04/23 14:55:28 ktk Exp $ */

#ifndef _I386_DELAY_H
#define _I386_DELAY_H

/*
 * Copyright (C) 1993 Linus Torvalds
 *
 * Delay routines calling functions in arch/i386/lib/delay.c
 */
 
extern void __udelay(unsigned long usecs);
extern void __const_udelay(unsigned long usecs);
extern void __delay(unsigned long loops);

#define udelay(n) __udelay(n)

#endif /* defined(_I386_DELAY_H) */
