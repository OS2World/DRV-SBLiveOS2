/* $Id: io.h,v 1.1 2000/04/23 14:55:28 ktk Exp $ */

#ifndef _ASM_IO_H
#define _ASM_IO_H

/*
 * This file contains the definitions for the x86 IO instructions
 * inb/inw/inl/outb/outw/outl and the "string versions" of the same
 * (insb/insw/insl/outsb/outsw/outsl). You can also use "pausing"
 * versions of the single-IO instructions (inb_p/inw_p/..).
 *
 * This file is not meant to be obfuscating: it's just complicated
 * to (a) handle it all in a way that makes gcc able to optimize it
 * as well as possible and (b) trying to avoid writing the same thing
 * over and over again with slight variations and possibly making a
 * mistake somewhere.
 */

/*
 * Thanks to James van Artsdalen for a better timing-fix than
 * the two short jumps: using outb's to a nonexistent port seems
 * to guarantee better timings even on fast machines.
 *
 * On the other hand, I'd like to be sure of a non-existent port:
 * I feel a bit unsafe about using 0x80 (should be safe, though)
 *
 *		Linus
 */

 /*
  *  Bit simplified and optimized by Jan Hubicka
  *  Support of BIGMEM added by Gerhard Wichert, Siemens AG, July 1999.
  *
  *  isa_memset_io, isa_memcpy_fromio, isa_memcpy_toio added,
  *  isa_read[wl] and isa_write[wl] fixed
  *  - Arnaldo Carvalho de Melo <acme@conectiva.com.br>
  */

#ifdef SLOW_IO_BY_JUMPING
#define __SLOW_DOWN_IO "\njmp 1f\n1:\tjmp 1f\n1:"
#else
#define __SLOW_DOWN_IO "\noutb %%al,$0x80"
#endif

#ifdef REALLY_SLOW_IO
#define __FULL_SLOW_DOWN_IO __SLOW_DOWN_IO __SLOW_DOWN_IO __SLOW_DOWN_IO __SLOW_DOWN_IO
#else
#define __FULL_SLOW_DOWN_IO __SLOW_DOWN_IO
#endif

/*
 * Talk about misusing macros..
 */
#define __OUT1(s,x) \
extern inline void out##s(unsigned x value, unsigned short port) {


#define __IN1(s) \
extern inline RETURN_TYPE in##s(unsigned short port) { RETURN_TYPE _v;


void outb(unsigned char data, int port);
#pragma aux outb =       \
  "out dx, al"                  \
  parm [al] [dx];

unsigned char inb(int port);
#pragma aux inb =       \
  "in al,dx"            \
  parm [dx]             \
  value [al];

void outw(unsigned short data, int port);
#pragma aux outw =       \
  "out dx, ax"                  \
  parm [ax] [dx];

unsigned short inw(int port);
#pragma aux inw =       \
  "in ax,dx"            \
  parm [dx]             \
  value [ax];

void outl(unsigned long data, int port);
#pragma aux outl =       \
  "out dx, eax"                  \
  parm [eax] [dx];

unsigned long inl(int port);
#pragma aux inl =       \
  "in eax,dx"            \
  parm [dx]             \
  value [eax];

#endif
