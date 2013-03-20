/* $Id: runtime.h,v 1.1 2000/04/23 14:55:20 ktk Exp $ */

//ษอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
//บ runtime.h                                                       บ
//บ Avoid problems using the runtime library functions              บ
//บ                                                                 บ
//บ MODIFICATION HISTORY                                            บ
//บ DATE       PROGRAMMER   COMMENT                                 บ
//บ 03-26-97   Rich Jerant  Creation How did we ever do without thisบ
//บ                         Nothing pretty here just prototypes for บ
//บ                         most of the functions in runtime.lib    บ
//บ                                                                 บ
//ศอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
#ifndef RUNTIME_INCLUDED
#define RUNTIME_INCLUDED

int toupper(int c);
char *strncpy(char *dst, const char *src, int n);
char __far *_fstrncpy(char __far *dst, const char __far *src, int n);
int _fstrnicmp(const char __far *string1, const char __far *string2, int n);

#endif
