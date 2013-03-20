; $Id: startup.asm,v 1.3 2001/05/13 19:50:33 sandervl Exp $ 


; 16 bits entrypoints for the PDD and thunks to call 32 bits code
;
; Copyright (C) 2000  Sander van Leeuwen
;
; Partly based on MWDD32 (32 bits OS/2 device driver and IFS support driver)
; Copyright (C) 1995, 1996 Matthieu WILLM
;
; This program is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 2 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program; if not, write to the Free Software
; Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

        .386p


        INCL_DOS        equ 1
        INCL_DOSERRORS  equ 1
        include os2.inc

	include sbseg.inc

DevHlp_VirtToLin	EQU	5Bh
DevHlp_VMLock		EQU	55h

; Status word masks
STERR	EQU	8000H		; Bit 15 - Error
STINTER EQU	0400H		; Bit 10 - Interim character
STBUI	EQU	0200H		; Bit  9 - Busy
STDON	EQU	0100H		; Bit  8 - Done
STECODE EQU	00FFH		; Error code

; Definition of the request packet header.

reqPacket       struc
reqLenght       db ?
reqUnit         db ?
reqCommand      db ?
reqStatus       dw ?
reqFlags        db ?
                db 3 dup (?)    ; Reserved field
reqLink         dd ?
reqPacket       ends

rpInitIn        struc
i_rph           db size reqPacket dup (?)
i_unit          db ?
i_devHelp       dd ?
i_initArgs      dd ?
i_driveNum      db ?
rpInitIn        ends

rpInitOut       struc
o_rph           db size reqPacket dup (?)
o_unit          db ?
o_codeend	dw ?
o_dataend	dw ?
o_bpbarray	dd ?
o_status	dw ?
rpInitOut       ends

BEGINCS16       EQU	OFFSET CODE16:help_stub_strategy
BEGINDS16    	EQU     OFFSET DATA16:help_header

DATA16 segment
		extrn DOS32FLATDS : abs                ; ring 0 FLAT kernel data selector
  	        public __OffFinalDS16
		public help_header
		public sblive_header
		public _MSG_TABLE16
		public DevHelpInit
		public fOpen	
		public InitPktSeg
		public InitPktOff
		public _MESSAGE_STR
		public pddname16
		public FileName

;*********************************************************************************************
;************************* Device Driver Header **********************************************
;*********************************************************************************************
;DEZE MOET ALS EERSTE blijven staan!!!!
help_header     dw (OFFSET sblive_header - BEGINDS16)
;dw OFFSET DATA16:sblive_header         ; Pointer to next driver
		dw SEG DATA16:sblive_header
                dw 1000100110000000b            ; Device attributes
;                  ||||| +-+   ||||
;                  ||||| | |   |||+------------------ STDIN
;                  ||||| | |   ||+------------------- STDOUT
;                  ||||| | |   |+-------------------- NULL
;                  ||||| | |   +--------------------- CLOCK
;                  ||||| | |
;                  ||||| | +------------------------+ (001) OS/2
;                  ||||| |                          | (010) DosDevIOCtl2 + SHUTDOWN
;                  ||||| +--------------------------+ (011) Capability bit strip
;                  |||||
;                  ||||+----------------------------- OPEN/CLOSE (char) or Removable (blk)
;                  |||+------------------------------ Sharing support
;                  ||+------------------------------- IBM
;                  |+-------------------------------- IDC entry point
;                  +--------------------------------- char/block device driver

;                dw offset CODE16:help_stub_strategy       ; Strategy routine entry point
                dw (offset help_stub_strategy - BEGINCS16) ; Strategy routine entry point
                dw 0				    ; IDC routine entry point
                db 'OSSHLP$ '                   ; Device name
                db 8 dup (0)                    ; Reserved
                dw 0000000000010011b            ; Level 3 device driver capabilities
;                             |||||
;                             ||||+------------------ DosDevIOCtl2 + Shutdown
;                             |||+------------------- More than 16 MB support
;                             ||+-------------------- Parallel port driver
;                             |+--------------------- Adapter device driver
;                             +---------------------- InitComplete
                dw 0000000000000000b

sblive_header    dd -1
                dw 1101100110000000b            ; Device attributes
;                  ||||| +-+   ||||
;                  ||||| | |   |||+------------------ STDIN
;                  ||||| | |   ||+------------------- STDOUT
;                  ||||| | |   |+-------------------- NULL
;                  ||||| | |   +--------------------- CLOCK
;                  ||||| | |
;                  ||||| | +------------------------+ (001) OS/2
;                  ||||| |                          | (010) DosDevIOCtl2 + SHUTDOWN
;                  ||||| +--------------------------+ (011) Capability bit strip
;                  |||||
;                  ||||+----------------------------- OPEN/CLOSE (char) or Removable (blk)
;                  |||+------------------------------ Sharing support
;                  ||+------------------------------- IBM
;                  |+-------------------------------- IDC entry point
;                  +--------------------------------- char/block device driver

;                dw offset CODE16:sblive_stub_strategy; Strategy routine entry point
                dw (offset sblive_stub_strategy - BEGINCS16); Strategy routine entry point
                dw (offset sblive_stub_idc  - BEGINCS16)   ; IDC routine entry point
                db 'SBLIVE2$'                   ; Device name
                db 8 dup (0)                    ; Reserved
                dw 0000000000010011b            ; Level 3 device driver capabilities
;                             |||||
;                             ||||+------------------ DosDevIOCtl2 + Shutdown
;                             |||+------------------- More than 16 MB support
;                             ||+-------------------- Parallel port driver
;                             |+--------------------- Adapter device driver
;                             +---------------------- InitComplete
                dw 0000000000000000b

DevHelpInit	dd 0
fOpen		dd 0
InitPktSeg	dw 0
InitPktOff	dw 0
;needed for rmcalls.lib
_RM_Help0       dd 0
_RM_Help1       dd 0
_RM_Help3       dd 0
_RMFlags        dd 0
_MESSAGE_STR    db 1024 dup (0)
_MSG_TABLE16    dw 0	;message length
		dw (OFFSET _MESSAGE_STR - BEGINDS16)	;message far pointer
		dw SEG    _MESSAGE_STR

pddname16	db 'SBLIVE2$'
FileName 	db "OSSHLP$", 0

;last byte in 16 bits data segment
__OffFinalDS16 label byte

DATA16 ends

CODE16 segment
        assume cs:CODE16, ds:DATA16

        public __OffFinalCS16

        public help_stub_strategy
        public sblive_stub_strategy
	public sblive_stub_idc
	public thunk3216_devhelp
	public thunk3216_devhelp_modified_ds
        extrn DOSOPEN       : far
        extrn DOSCLOSE      : far
        extrn DOSWRITE      : far

;DEZE MOET ALS EERSTE blijven staan!!!!
help_stub_strategy proc far
	pushad  
	push	ds
	push	es
	push	fs
	push	gs

	mov	ax, DATA16
	mov	ds, ax

        movzx 	eax, byte ptr es:[bx].reqCommand
	cmp	eax, 0				; Init
	je	short @@help_init
        cmp 	eax, 0Dh			; DosOpen
        jne 	short @@help_error
;DosOpen
	cmp 	word ptr ds:(offset fOpen - BEGINDS16), 0
	je	short @@help_ret_ok		; not ours
	push	ebx				; later weer nodig
	push	es
	mov	word ptr ds:(offset fOpen - BEGINDS16), 0
	mov	ax, word ptr ds:(offset InitPktSeg-BEGINDS16)
	mov	fs, ax				; fs:ebx = req. packet
	xor	ebx, ebx
	mov     bx, word ptr ds:(offset InitPktOff - BEGINDS16)
	call	far ptr FLAT:STRATEGY_
	pop	es
	pop	ebx				; oude bx ptr
@@help_ret:
        mov 	word ptr es:[bx].reqStatus, ax
@@help_ret_error:
	pop	gs
	pop	fs
	pop	es
	pop	ds
	popad
	ret
		
@@help_init:
	mov	eax, dword ptr es:[bx].i_devHelp
	mov	dword ptr ds:(offset DevHelpInit - BEGINDS16), eax
        mov 	word ptr es:[bx].o_codeend, (offset __OffFinalCS16 - BEGINCS16)
        mov 	word ptr es:[bx].o_dataend, (offset __OffFinalDS16 - BEGINDS16)

@@help_ret_ok:
	mov	ax, STDON
	jmp	short @@help_ret

@@help_error:
	mov	ax, STDON + STERR + ERROR_I24_BAD_COMMAND
        mov 	word ptr es:[bx].reqStatus, ax
	jmp	short @@help_ret_error

help_stub_strategy endp


sblive_stub_strategy proc far
	pushad  
	push	ds
	push	es
	push	fs
	push	gs

	mov	ax, DATA16
	mov	ds, ax

        movzx 	eax, byte ptr es:[bx].reqCommand
        cmp 	eax, 0
        jz 	short @@init

	push	ebx
	push	es
	mov	ax, bx
	xor	ebx, ebx			
	mov	bx, ax
	mov	ax, es
	mov	fs, ax				; fs:ebx = req. packet

        call 	far ptr FLAT:STRATEGY_     ; 32 bits strategy entry point

	pop	es
	pop	ebx				; oude bx ptr
        mov 	word ptr es:[bx].reqStatus, ax  ; status code

@@sblive_ret:

	pop	gs
	pop	fs
	pop	es
	pop	ds
	popad
        ret

@@init:
        ;
        ; DEVICE= initialization
        ;
	mov  	word ptr ds:(InitPktSeg - BEGINDS16), es
	mov	word ptr ds:(InitPktOff - BEGINDS16), bx
	inc	word ptr ds:(fOpen - BEGINDS16)
	call 	device_init

        mov 	word ptr es:[bx].reqStatus, ax  ; status code (ret by device_init)
        mov 	word ptr es:[bx].o_codeend, (offset __OffFinalCS16 - BEGINCS16)
        mov 	word ptr es:[bx].o_dataend, (offset __OffFinalDS16 - BEGINDS16)
	jmp 	short @@sblive_ret

init_err:
	mov 	dword ptr es:[bx].i_devHelp, 0
	jmp 	short @@sblive_ret

sblive_stub_strategy endp

;in: cx = cmd
;    bx = lower 16 bits of ULONG parameter
;    dx = upper 16 bits of ULONG parameter
;return value in dx:ax
sblive_stub_idc proc far
        enter   0, 0
        and     sp, 0fffch

	shl	edx, 16
	mov	dx, bx
        call 	far ptr FLAT:IDC_     ; 32 bits strategy entry point

	mov	dx, ax
	shr	eax, 16
	xchg	ax, dx

        leave
	retf
sblive_stub_idc endp

device_init proc near
	enter 	24, 0
	push 	ds
	push 	es
	push 	bx
	push 	si
	push 	di

        ; bp      ->  old bp
        ; bp - 2  -> FileHandle
        ; bp - 4  -> ActionTaken
        ; bp - 8  -> IOCTL parm (4 bytes)  : union mwdd32_ioctl_init_device_parm
        ; bp - 24 -> IOCTL data (16 bytes) : union mwdd32_ioctl_init_device_data

        ;
        ; Opens wathlp$
        ;
        push 	seg DATA16                 ; seg  FileName
        push 	(offset FileName - BEGINDS16)     ; ofs  FileName
        push 	ss                         ; seg &FileHandle
        lea 	ax, [bp - 2]
        push 	ax                         ; ofs &FileHandle
        push 	ss                         ; seg &ActionTaken
        lea 	ax, [bp - 4]
        push 	ax                         ; ofs &ActionTaken
        push 	dword ptr 0                ; file size
        push 	0                          ; file attributes
        push 	OPEN_ACTION_FAIL_IF_NEW + OPEN_ACTION_OPEN_IF_EXISTS
        push 	OPEN_SHARE_DENYNONE + OPEN_ACCESS_READONLY
        push 	dword ptr 0                ; reserved
        call 	DOSOPEN
        cmp 	ax, NO_ERROR
        jnz 	short @@error


        ;
        ; Closes wathlp$
        ;
        push 	word ptr [bp - 2]                   ; FileHandle
        call 	DOSCLOSE
        cmp 	ax, NO_ERROR
        jnz 	short @@error

@@out:
	push	eax		;gemold door doswrite

        push    0001H
        push    DATA16
        push    (offset _MESSAGE_STR - BEGINDS16)
        push    word ptr ds:(offset _MSG_TABLE16 - BEGINDS16)
	push	ss
        lea     dx, [bp - 2]
        push    dx
        call    DOSWRITE

	pop	eax

	pop 	di
	pop 	si
	pop 	bx
	pop 	es
	pop 	ds
	leave
	ret
@@error:
	mov 	ax, STDON + STERR + ERROR_I24_GEN_FAILURE
	jmp 	short @@out

device_init endp

	ALIGN   2
;use devhlp pointer stored in 16 bits code segment
thunk3216_devhelp:
	push	ds
	push	DATA16
	pop	ds
        call 	dword ptr ds:(offset DevHelpInit - BEGINDS16)
	pop	ds

        jmp 	far ptr FLAT:thunk1632_devhelp

	ALIGN 	2
thunk3216_devhelp_modified_ds:
	push	gs
	push	DATA16
	pop	gs
        call 	dword ptr gs:(offset DevHelpInit - BEGINDS16)
	pop	gs
        jmp 	far ptr FLAT:thunk1632_devhelp_modified_ds

;end of 16 bits code segment
__OffFinalCS16 label byte

CODE16 ends

CODE32 segment
ASSUME CS:FLAT, DS:FLAT, ES:FLAT

        public __OffBeginCS32
	public __GETDS
        public thunk1632_devhelp
	public thunk1632_devhelp_modified_ds	
        public DevHlp
        public DevHlp_ModifiedDS
	public STRATEGY_
	public IDC_
        extrn  SBLIVE_STRATEGY  : near
	extrn  SBLIVE_IDC : near
IFDEF KEE
	extrn  KernThunkStackTo16 : near
	extrn  KernThunkStackTo32 : near
ENDIF

__OffBeginCS32:

;Called by Watcom to set the DS
__GETDS proc near
	push	eax
        mov 	eax, DOS32FLATDS
	mov	ds, eax
	pop	eax
	ret
__GETDS endp

__wcpp_2_pure_error__:
__wcpp_2_undef_vfun__:
__wcpp_2_undefed_cdtor__:
__wcpp_2_dtor_array_store__:
DevHelpDebug  proc near
        int 3
        int 3
        ret
DevHelpDebug  endp

	ALIGN 	4

DevHlp proc near
IFDEF KEE
	push	eax
        call	KernThunkStackTo16
	pop	eax	;trashed by KernThunkStackTo16
ENDIF

	jmp	far ptr CODE16:thunk3216_devhelp
	ALIGN 4
thunk1632_devhelp:
IFDEF KEE
	push	eax
	push	edx
        call	KernThunkStackTo32
	pop	edx	;trashed by KernThunkStackTo32
	pop	eax	;trashed by KernThunkStackTo32
ENDIF
	ret
DevHlp endp

	ALIGN 	4
DevHlp_ModifiedDS proc near
IFDEF KEE
	push	eax
        call	KernThunkStackTo16
	pop	eax	;trashed by KernThunkStackTo16
ENDIF
	jmp	far ptr CODE16:thunk3216_devhelp_modified_ds
	ALIGN 4
thunk1632_devhelp_modified_ds:
IFDEF KEE
	push	eax
	push	edx
        call	KernThunkStackTo32
	pop	edx	;trashed by KernThunkStackTo32
	pop	eax	;trashed by KernThunkStackTo32
ENDIF
	ret
DevHlp_ModifiedDS endp

STRATEGY_ proc far
	push	ds
	push	es
	push	fs
	push	gs

        mov 	eax, DOS32FLATDS
	mov	ds, eax
	mov	es, eax
IFDEF KEE
	push	stacksel
	push	stackbase

	push	edx
	mov	edx, ss
	mov	stacksel, edx

        call	KernThunkStackTo32
	mov	stackbase, edx
	pop	edx	;trashed by KernThunkStackTo32
	call 	SBLIVE_STRATEGY
	push	eax
        call	KernThunkStackTo16
	pop	eax	;trashed by KernThunkStackTo16

	pop	stackbase
	pop	stacksel
ELSE
	call 	SBLIVE_STRATEGY
ENDIF

	pop	gs
	pop	fs
	pop	es
	pop	ds
	retf
STRATEGY_ endp

;in: ecx = cmd
;    edx = ULONG parameter
;return value in eax
IDC_ proc far
	push	ds
	push	es
	push	fs
	push	gs
	push	ebx

        mov 	eax, DOS32FLATDS
	mov	ds, eax
	mov	es, eax

IFDEF KEE
	push	stacksel
	push	stackbase

	push	edx
	mov	edx, ss
	mov	stacksel, edx

        call	KernThunkStackTo32
	mov	stackbase, edx
	pop	edx	 ;trashed by KernThunkStackTo32

	call 	SBLIVE_IDC

	push	eax
        call	KernThunkStackTo16
	pop	eax	;trashed by KernThunkStackTo16

	pop	stackbase
	pop	stacksel
ELSE
	call 	SBLIVE_IDC
ENDIF
	pop	ebx
	pop	gs
	pop	fs
	pop	es
	pop	ds
	retf
IDC_ endp

CODE32 ends

DATA32 	segment
    public  __OffsetFinalCS16
    public  __OffsetFinalDS16
    public  __wcpp_2_pure_error__
    public  __wcpp_2_undef_vfun__
    public  __wcpp_2_undefed_cdtor__
    public  __wcpp_2_dtor_array_store__
    public  PDDName
    public  _MSG_TABLE32
    public  __OffBeginDS32
    public  stackbase
    public  stacksel

    __OffBeginDS32   dd 0

    stacksel         dd 0
    stackbase        dd 0

    __OffsetFinalCS16 dw (OFFSET CODE16:__OffFinalCS16 - BEGINCS16)
    __OffsetFinalDS16 dw (OFFSET DATA16:__OffFinalDS16 - BEGINDS16)

    _MSG_TABLE32     dw (OFFSET  DATA16:_MSG_TABLE16 - BEGINDS16)
    		     dw SEG     DATA16:_MSG_TABLE16
 
;16:16 address of driver name   
    PDDName          dw (OFFSET  DATA16:pddname16 - BEGINDS16)
		     dw SEG     DATA16:pddname16

DATA32 ends

end

