; $Id: memutil.asm,v 1.1 2000/04/23 14:55:17 ktk Exp $ 

;/* SCCSID = %W% %E% */
;/***************************************************************************
;                                                                           *
;  Copyright (c) IBM Corporation 1994 - 1997.                               *
;                                                                           *
;  The following IBM OS/2 source code is provided to you solely for the     *
;  the purpose of assisting you in your development of OS/2 device drivers. *
;  You may use this code in accordance with the IBM License Agreement       *
;  provided in the IBM Device Driver Source Kit for OS/2.                   *
;                                                                           *
; ***************************************************************************/
;/*@internal %W%
;  @notes
;  The Watcom __fmemcpy intrinsic function resolves to a movsw instruction
;  which requires 4 clock cycles to move 2 bytes of data. the movsd instruction
;  can move 4 bytes in the same 4 clock cycles. Such a deal !!!!!
;  Since the compiler won't let us write a pragma aux to implement this
;  function, we have written it in assembler. There is also a memfill function
;  that uses the stosd instruction to fill memory with the a value in eax.
;  This function is used to write "silence" into the audio buffer
;
;  @version %I%
;  @context Unless otherwise noted, all interfaces are Ring-0, 16-bit,
;   <stack context>.
;  @history
;
;

.386
.seq

_HEADER       segment dword public use16 'DATA'
_HEADER       ends

_DATA         segment dword public use16 'DATA'
_DATA         ends

CONST         segment dword public use16 'DATA'
CONST         ends

CONST2        segment dword public use16 'DATA'
CONST2        ends

_BSS          segment dword public use16 'BSS'
_BSS          ends

_ENDDS        segment dword public use16 'ENDDS'
_ENDDS        ends

_INITDATA     segment dword public use16 'ENDDS'
_INITDATA     ends

_ENDINITDATA  segment dword public use16 'ENDDS'
_ENDINITDATA  ends

_TEXT         segment dword public use16 'CODE'
_TEXT         ends

RMCODE        segment dword public use16 'CODE'
RMCODE        ends

_ENDCS        segment dword public use16 'CODE'
_ENDCS        ends

_INITTEXT     segment dword public use16 'CODE'
_INITTEXT     ends

DGROUP        group _HEADER, CONST, CONST2, _DATA, _BSS, _ENDDS, _INITDATA, _ENDINITDATA
CGROUP        group _TEXT, RMCODE, _ENDCS, _INITTEXT


_TEXT       SEGMENT DWORD PUBLIC USE16 'CODE'
            assume cs:cgroup, ds:dgroup


;    void cdecl ddmemmov(PVOID pdest, PVOID psrc, USHORT count);

public _ddmemmov
_ddmemmov proc near

DEST1_LOW equ  [bp+4]
DEST1_HIGH equ [bp+6]
SRC1_LOW equ   [bp+8]
SRC1_HIGH equ  [bp+0ah]
BUF1_LEN equ   [bp+0ch]

    ; point to parameters and local variables
        push   bp
        mov    bp,sp

    ; save registers
        push   cx
        push   bx
        push   ds
        push   es
        push   si
        push   di

    ; store number of bytes to transfer and calculate number of double words
    ; and set of transfer value
        mov    bx,BUF1_LEN
        mov    cx,bx
        shr    cx,2

    ; set size in bytes of object to be allocated
        mov    es,DEST1_HIGH
        mov    di,DEST1_LOW

    ; set up pointers to source buffer
        mov    ds,SRC1_HIGH
        mov    si,SRC1_LOW

    ; set up transfer direction
        cld

    ; do the transfer
        rep    movsd

    ; transfer number of odd bytes
        mov    cx,bx
        and    cx,03h
        rep    movsb

    ;set up return value and restore registers
        pop    di
        pop    si
        pop    es
        pop    ds
        pop    bx
        pop    cx
        pop    bp

   ;done
        ret
_ddmemmov endp


;VOID memfill(UCHAR *destP, ULONG length, UCHAR value)
;    This function fills a block of memory with a particular value.
;    The function receives address of the memory, the length of the memory, and
; the fill value as parameters on the stack.
;    The function does not return a value.

public _ddmemfill
_ddmemfill proc near

DEST2_LOW equ  [bp+4]
DEST2_HIGH equ [bp+6]
BUF2_LEN equ   [bp+8]
VALUE2 equ     [bp+0ah]

    ; point to parameters and local variables
        push   bp
        mov    bp,sp

    ; save registers
        push   bx
        push   cx
        push   es
        push   di

    ; store number of bytes to transfer and calculate number of double words
    ; and set of transfer value
        mov    bx,BUF2_LEN
        mov    cx,bx
        shr    cx,2

    ; set size in bytes of object to be allocated
        mov    es,DEST2_HIGH
        mov    di,DEST2_LOW

    ; set up value
        mov    ax,VALUE2
        shl    eax,010h
        mov    ax,VALUE2

    ; set up transfer direction
        cld

    ; do the transfer
        rep    stosd

    ; transfer number of odd bytes
        mov    cx,bx
        and    cx,03h
        rep    stosb

    ;set up return value and restore registers
        pop    di
        pop    es
        pop    cx
        pop    bx
        pop    bp

   ;done
        ret
_ddmemfill endp

_TEXT       ENDS

            end
