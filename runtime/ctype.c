/* $Id: ctype.c,v 1.1 2000/04/23 14:55:39 ktk Exp $ */

/* CTYPE.C - pseudo runtime library mimicking functions declared in ctype.h

   MODIFICATION HISTORY
   DATE       PROGRAMMER   COMMENT
   01-Jul-95  Timur Tabi   Creation
*/

int toupper(int c)
{
   return (unsigned) c - 'a' <= 'z' - 'a' ? c - ('a' - 'A') : c;
}
