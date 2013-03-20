/* $Id: runtime.h,v 1.1 2000/04/23 14:55:20 ktk Exp $ */

//�����������������������������������������������������������������ͻ
//� runtime.h                                                       �
//� Avoid problems using the runtime library functions              �
//�                                                                 �
//� MODIFICATION HISTORY                                            �
//� DATE       PROGRAMMER   COMMENT                                 �
//� 03-26-97   Rich Jerant  Creation How did we ever do without this�
//�                         Nothing pretty here just prototypes for �
//�                         most of the functions in runtime.lib    �
//�                                                                 �
//�����������������������������������������������������������������ͼ
#ifndef RUNTIME_INCLUDED
#define RUNTIME_INCLUDED

int toupper(int c);
char *strncpy(char *dst, const char *src, int n);
char __far *_fstrncpy(char __far *dst, const char __far *src, int n);
int _fstrnicmp(const char __far *string1, const char __far *string2, int n);

#endif
