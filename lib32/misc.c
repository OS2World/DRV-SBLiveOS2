/* $Id: misc.c,v 1.4 2001/12/20 19:58:01 sandervl Exp $ */

//******************************************************************************
// OS/2 implementation of misc. Linux kernel services
//
// Copyright 2000 Sander van Leeuwen (sandervl@xs4all.nl)
//
// Parts based on Linux kernel code (set/clear_bit)
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
#include "hwaccess.h"
#include <linux/init.h>
#include <linux/poll.h>
#include <asm/uaccess.h>
#include <asm/hardirq.h>
#include <linux\ioport.h>
#include <linux\utsname.h>

struct new_utsname system_utsname = {0};
struct resource ioport_resource = {0};
mem_map_t *mem_map = 0;

void set_bit(int nr, volatile void * addr)
{
 volatile unsigned long *pAddr = (volatile unsigned long *)addr;

   *pAddr = (*pAddr) | (1 << nr);
}

void clear_bit(int nr, volatile void * addr)
{
 volatile unsigned long *pAddr = (volatile unsigned long *)addr;

   *pAddr = (*pAddr) & ~(1 << nr);
}

#define CR 0x0d
#define LF 0x0a


#define LEADING_ZEROES          0x8000
#define SIGNIFICANT_FIELD       0x0007

char *HexLongToASCII(char *StrPtr, unsigned long wHexVal, unsigned short Option);
char *DecLongToASCII(char *StrPtr, unsigned long lDecVal, unsigned short Option);


//SvL: Not safe to use in non-KEE driver
int sprintf (char *buffer, const char *format, ...)
{
   char *BuildPtr=buffer;
   char *pStr = (char *) format;
   char *SubStr;
   union {
         void   *VoidPtr;
         unsigned short *WordPtr;
         unsigned long  *LongPtr;
#ifdef KEE
         unsigned long  *StringPtr;
#else
	 double *StringPtr;
#endif
         } Parm;
   int wBuildOption;

   Parm.VoidPtr=(void *) &format;
   Parm.StringPtr++;                            // skip size of string pointer

   while (*pStr)
      {
      switch (*pStr)
         {
         case '%':
            wBuildOption=0;
            pStr++;
            if (*pStr=='0')
               {
               wBuildOption|=LEADING_ZEROES;
               pStr++;
               }
            if (*pStr=='u')                                                         // always unsigned
               pStr++;

            switch(*pStr)
               {
               case 'x':
	       case 'X':
                  BuildPtr=HexLongToASCII(BuildPtr, *Parm.LongPtr++,wBuildOption);
                  pStr++;
                  continue;

               case 'd':
                  BuildPtr=DecLongToASCII(BuildPtr, *Parm.LongPtr++,wBuildOption);
                  pStr++;
                  continue;

#ifdef KEE
               case 's':
                  SubStr=(char *)*Parm.StringPtr;
                  while (*BuildPtr++ = *SubStr++);
                  Parm.StringPtr++;
                  BuildPtr--;                      // remove the \0
                  pStr++;
                  continue;
#endif
               case 'l':
                  pStr++;
                  switch (*pStr)
                  {
                  case 'x':
                  case 'X':
                  BuildPtr=HexLongToASCII(BuildPtr, *Parm.LongPtr++,wBuildOption);
                  pStr++;
                  continue;

                  case 'd':
                     BuildPtr=DecLongToASCII(BuildPtr, *Parm.LongPtr++,wBuildOption);
                     pStr++;
                     continue;
                  } // end switch
                  continue;                        // dunno what he wants

               case 0:
                  continue;
               } // end switch
            break;

      case '\\':
         pStr++;
         switch (*pStr)
            {
            case 'n':
            *BuildPtr++=LF;
            pStr++;
            continue;

            case 'r':
            *BuildPtr++=CR;
            pStr++;
            continue;

            case 0:
            continue;
            break;
            } // end switch

         break;
         } // end switch

      *BuildPtr++=*pStr++;
      } // end while

   *BuildPtr=0;                                 // cauterize the string
   return 1; //not correct
}

int printk(const char * fmt, ...)
{
  return 0;
}

void schedule(void)
{

}

void poll_wait(struct file * filp, wait_queue_head_t * wait_address, poll_table *p)
{

}


int __check_region(struct resource *a, unsigned long b, unsigned long c)
{
  return 0;
}

void __release_region(struct resource *a, unsigned long b, unsigned long c)
{

}

struct resource * __request_region(struct resource *a, unsigned long start, unsigned long n, const char *name)
{
  return a;
}

void iodelay32(unsigned long);
#pragma aux iodelay32 parm nomemory [ecx] modify nomemory exact [eax ecx];

void __udelay(unsigned long usecs)
{
  iodelay32(usecs*2);
}


/* --------------------------------------------------------------------- */
/*
 * hweightN: returns the hamming weight (i.e. the number
 * of bits set) of a N-bit word
 */

#ifdef hweight32
#undef hweight32
#endif

unsigned int hweight32(unsigned int w)
{
	unsigned int res = (w & 0x55555555) + ((w >> 1) & 0x55555555);
	res = (res & 0x33333333) + ((res >> 2) & 0x33333333);
	res = (res & 0x0F0F0F0F) + ((res >> 4) & 0x0F0F0F0F);
	res = (res & 0x00FF00FF) + ((res >> 8) & 0x00FF00FF);
	return (res & 0x0000FFFF) + ((res >> 16) & 0x0000FFFF);
}
