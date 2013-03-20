; $Id: vddentry.asm,v 1.3 2001/03/22 18:13:01 sandervl Exp $ 

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


; Route VDD IDC request to C code to implement
; 16:16 entry from 16:32 caller
;
; Called from VDD using pascal calling conventions:
;       Parms pushed left to right, callee clears the stack
;
ulFunc  EQU DWORD PTR [bp+18]   ; Pascal conventions push parms left to right
ul1     EQU DWORD PTR [bp+14]   ; 8 bytes for 16:32 far return address
ul2     EQU DWORD PTR [bp+10]   ; 2 bytes for save of callers stack frame ptr

        PUBLIC  IDCENTRY_VDD
        EXTERN  IDCENTRY_VDD_C:near
IDCENTRY_VDD PROC FAR  ; 16:16 entry from 16:32 ; VDD calls PDD
        push    bp
        mov     bp, sp
        push    ds
        mov     ax, seg _DATA
        push    ulFunc                          ; pascal calling convention
        mov     ds, ax
        push    ul1
        push    ul2
        call    IDCENTRY_VDD_C
        shl     edx, 16                         ; Move DX:AX return value
        and     eax, 0000FFFFh                  ; into eax
        pop     ds
        or      eax, edx
        leave
        db      66h                             ; Force next instruction 32-bit
        ret     12                              ; 16:32 far return, pop parms
IDCENTRY_VDD ENDP

pPacket EQU DWORD PTR [bp+10h]   ; 8 bytes for 16:32 far return address
ulCmd   EQU DWORD PTR [bp+0Ch]   ; cdecl conventions push parms right to left

        PUBLIC  _OSSIDC_ENTRY
        EXTERN  _OSSIDC_EntryPoint:near
_OSSIDC_ENTRY PROC FAR  ; 16:16 entry from 16:32 ; called from 32 bits pdd
        push    ebp
        mov     bp, sp
        push    ds
        mov     ax, seg _DATA
        push    DWORD PTR [bp+10h]		; cdecl calling convention
        mov     ds, ax
        push    DWORD PTR [bp+0Ch]
        call    _OSSIDC_EntryPoint
	add	sp, 8
        shl     edx, 16                         ; Move DX:AX return value
        and     eax, 0000FFFFh                  ; into eax
        pop     ds
        or      eax, edx
        pop	ebp
        db      66h                             ; Force next instruction 32-bit
        ret                                     ; 16:32 far return
_OSSIDC_ENTRY ENDP

        PUBLIC  HookHandlerAsm_
        EXTERN  _HookHandler:near
HookHandlerAsm_ PROC FAR
        push	eax
        call    _HookHandler
        add     sp, 4
	retf
HookHandlerAsm_ ENDP

        PUBLIC  _inpd
_inpd proc near
	in   eax, dx
	mov  dx, ax
	shr  eax, 16
	xchg ax, dx
	ret
_inpd endp

        PUBLIC  _outpd
_outpd proc near
	mov ax, cx
	shl eax, 16
	mov ax, bx
	out dx, eax
	ret
_outpd endp

_TEXT   ENDS

END
