; $Id: segments.asm,v 1.1 2000/04/23 14:55:20 ktk Exp $ 

; SCCSID = %W% %E%
;***************************************************************************;
;                                                                           ;
;  Copyright (c) IBM Corporation 1994 - 1997.                               ;
;                                                                           ;
;  The following IBM OS/2 source code is provided to you solely for the     ;
;  the purpose of assisting you in your development of OS/2 device drivers. ;
;  You may use this code in accordance with the IBM License Agreement       ;
;  provided in the IBM Device Driver Source Kit for OS/2.                   ;
;                                                                           ;
;;**************************************************************************;
;;@internal %W%
;  Defines    segment ordering for 16-bit DD's with Watcom C++
; @version %I%
; @context
;   Unless otherwise noted, all interfaces are Ring-0, 16-bit, kernel stack.
; @notes
; @history
;  01-Jul-95  Timur Tabi   Creation
;
;;**************************************************************************;

.386
.seq

                  public _end_of_data
                  public _end_of_heap
                  public _end_of_initdata
                  public _end_of_text

_HEADER           segment dword public use16 'DATA'
_HEADER           ends

_DATA             segment dword public use16 'DATA'
_DATA             ends

CONST             segment dword public use16 'DATA'
CONST             ends

CONST2            segment dword public use16 'DATA'
CONST2            ends

_BSS              segment dword public use16 'BSS'
_BSS              ends

_ENDDS            segment dword public use16 'ENDDS'
_end_of_data      dw    0
_ENDDS            ends

_HEAP             segment dword public use16 'ENDDS'
_HEAP             ends

_ENDHEAP          segment dword public use16 'ENDDS'
_end_of_heap      dw    0
_ENDHEAP          ends

_INITDATA         segment dword public use16 'ENDDS'
_INITDATA         ends

_ENDINITDATA      segment dword public use16 'ENDDS'
_end_of_initdata  dw    0
_ENDINITDATA      ends

_TEXT             segment dword public use16 'CODE'
_TEXT             ends

RMCODE            segment dword public use16 'CODE'
RMCODE            ends

_ENDCS            segment dword public use16 'CODE'
_end_of_text      dw    0
_ENDCS            ends

_INITTEXT         segment dword public use16 'CODE'
_INITTEXT         ends

DGROUP            group _HEADER, CONST, CONST2, _DATA, _BSS, _ENDDS, _HEAP, _ENDHEAP, _INITDATA, _ENDINITDATA
CGROUP            group _TEXT, RMCODE, _ENDCS, _INITTEXT

end
