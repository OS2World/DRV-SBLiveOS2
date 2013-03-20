#ifndef __COMMDBG_H__
#define __COMMDBG_H__

#ifdef __cplusplus
extern "C" {
#endif

char far * cdecl HexLongToASCII(char far *StrPtr, ULONG wHexVal, USHORT Option);
char far * cdecl HexWordToASCII(char far *StrPtr, USHORT wHexVal, USHORT Option);
char far * cdecl DecLongToASCII(char far *StrPtr, ULONG lDecVal,USHORT Option);
char far * cdecl DecWordToASCII(char far *StrPtr, USHORT wDecVal, USHORT Option);

#ifdef __cplusplus
}
#endif

#endif //__COMMDBG_H__