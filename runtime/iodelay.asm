; $Id: iodelay.asm,v 1.1 2000/04/23 14:55:40 ktk Exp $ 

;; IODELAY.ASM - 500ns delay routine.  Calling convention defined in iodelay.h

;;   MODIFICATION HISTORY
;;   DATE       PROGRAMMER   COMMENT
;;   01-Jul-95  Timur Tabi   Creation

include     segments.inc

public      iodelay_

EXTRN       DOSIODELAYCNT:ABS

_TEXT       SEGMENT DWORD PUBLIC USE16 'CODE'
            assume cs:cgroup, ds:dgroup

iodelay_    proc  near
            db    0B8h
            dw    DOSIODELAYCNT
            align 4
@@:         dec   ax
            jnz   @b
            loop  iodelay_
            ret
iodelay_    endp

_TEXT       ENDS

            end         
