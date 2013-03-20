/* $Id: strncpy.c,v 1.1 2000/04/23 14:55:39 ktk Exp $ */

/* strncpy.c (emx+gcc) -- Copyright (c) 1990-1995 by Eberhard Mattes */

#include <string.h>

char *strncpy (char *string1, const char *string2, size_t count)
{
  char *dst;

  dst = string1;
  while (count > 0 && *string2 != 0)
    {
      *dst++ = *string2++;
      --count;
    }
  while (count > 0)
    {
      *dst++ = 0;
      --count;
    }
  return string1;
}
