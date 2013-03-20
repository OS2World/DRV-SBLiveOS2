/* $Id: string.c,v 1.1 2000/04/23 14:55:41 ktk Exp $ */

/* STRING.C - Routines defined in string.h that aren't instrinisc (inlined)
              by the compiler.

   MODIFICATION HISTORY
   DATE       PROGRAMMER   COMMENT
   01-Jul-95  Timur Tabi   Creation
*/

#include <ctype.h>
#include <string.h>

char *strncpy(char *dst, const char *src, int n)
{
   int i;

   for (i=0; i<n; i++)
      if ((*dst++ = *src++) == 0)
         break;
   return dst;
}


char __far *_fstrncpy(char __far *dst, const char __far *src, int n)
{
   int i;

   for (i=0; i<n; i++)
      if ((*dst++ = *src++) == 0)
         break;
   return dst;
}

int _fstrnicmp(const char __far *string1, const char __far *string2, int n)
{  
   int i,a,b;

   for (i=0; i<n; i++) {
      a=toupper(*string1++);
      b=toupper(*string2++);
      if (!a)
         return b ? 1 : 0;
      if (!b) return -1;
      if (a<b) return -1;
      if (a>b) return 1;
   }
   return 0;
}

