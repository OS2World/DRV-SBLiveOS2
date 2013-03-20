; $Id: math.asm,v 1.1 2000/04/23 14:55:40 ktk Exp $ 

;; MATH.ASM - Math routines called by the compiler

;;   MODIFICATION HISTORY
;;   DATE       PROGRAMMER   COMMENT
;;   01-Jul-95  Timur Tabi   Creation

include     segments.inc

public      __U4M
public      __I4M
public      __U4D
public      __I4D

;; Long multiply routine
;;
;; Arguments
;;    DX:AX * CX:BX
;; Returns
;;    DX:AX = product
;; Notes
;;    Trashes high words of 32-bit registers EAX and EDX

_TEXT       SEGMENT DWORD PUBLIC USE16 'CODE'
            assume cs:cgroup, ds:dgroup

__U4M       proc  near
__I4M:      shl   edx,10h            ;; Load dx:ax into eax
            mov   dx,ax
            mov   eax,edx
            mov   dx,cx              ;; Load cx:bx into edx
            shl   edx,10h
            mov   dx,bx
            mul   edx                ;; Multiply eax*edx into edx:eax
            mov   edx,eax            ;; Load eax into dx:ax
            shr   edx,10h
            ret
__U4M       endp

_TEXT       ENDS

;; Long unsigned divide routine
;;
;; Arguments
;;    DX:AX / CX:BX
;; Returns
;;    DX:AX = quotient
;;    CX:BX = remainder
;; Notes
;;    Trashes high words of 32-bit registers EAX, ECX and EDX

_TEXT       SEGMENT DWORD PUBLIC USE16 'CODE'
            assume cs:cgroup, ds:dgroup

__U4D       proc  near
            shl   edx,10h            ;; Load dx:ax into eax
            mov   dx,ax
            mov   eax,edx
            xor   edx,edx            ;; Zero extend eax into edx
            shl   ecx,10h            ;; Load cx:bx into ecx
            mov   cx,bx
            div   ecx                ;; Divide eax/ecx into eax
            mov   ecx,edx            ;; Load edx into cx:bx
            shr   ecx,10h
            mov   bx,dx
            mov   edx,eax            ;; Load eax into dx:ax
            shr   edx,10h
            ret
__U4D       endp

_TEXT       ENDS

;; Long signed divide routine
;;
;; Arguments
;;    DX:AX / CX:BX
;; Returns
;;    DX:AX = quotient
;;    CX:BX = remainder
;; Notes
;;    Trashes high words of 32-bit registers EAX, ECX and EDX

_TEXT       SEGMENT DWORD PUBLIC USE16 'CODE'
            assume cs:cgroup, ds:dgroup

__I4D       proc  near
            shl   edx,10h            ;; Load dx:ax into eax
            mov   dx,ax
            mov   eax,edx
            cdq                      ;; Sign extend eax into edx
            shl   ecx,10h            ;; Load cx:bx into ecx
            mov   cx,bx
            idiv  ecx                ;; Divide eax/ecx into eax
            mov   ecx,edx            ;; Load edx into cx:bx
            shr   ecx,10h
            mov   bx,dx
            mov   edx,eax            ;; Load eax into dx:ax
            shr   edx,10h
            ret
__I4D       endp

_TEXT       ENDS

            end         
