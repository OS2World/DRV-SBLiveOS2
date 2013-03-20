/* $Id: printf.c,v 1.1 2000/04/23 14:55:40 ktk Exp $ */

/* PRINTF.C - Support for PRINTF.SYS, the printf() debugging device driver

   MODIFICATION HISTORY
   DATE       PROGRAMMER   COMMENT
   01-Jul-95  Timur Tabi   Creation
   04-Nov-96  Timur Tabi   Updated for Watcom 10.6
*/

typedef void (__far __loadds __cdecl *PRINTF) (char _far *psz , ...);

#pragma off (unreferenced)

void __far _printf(void)
{
}

PRINTF printf = (PRINTF) _printf;
