; $Id: sbseg.inc,v 1.2 2000/05/28 16:50:42 sandervl Exp $ 

DATA16 segment word public use16 'DATA16'
DATA16 ends

CODE16 segment word public use16 'CODE'
CODE16 ends

CODE32 segment dword public use32 'CODE'
CODE32 ends

_TEXT segment dword public use32 'CODE'
_TEXT ends

BSS32   segment dword use32 public 'BSS'
BSS32   ends

_BSS   segment dword use32 public 'BSS'
_BSS   ends

C_COMMON segment dword use32 public 'DATA'
C_COMMON ends

c_common segment dword use32 public 'DATA'
c_common ends

CONST32 segment dword use32 public 'CONST'
CONST32 ends

DATA32  segment dword use32 public 'DATA'
DATA32  ends

CONST   SEGMENT DWORD PUBLIC USE32 'DATA'
CONST   ENDS

CONST2  SEGMENT DWORD PUBLIC USE32 'DATA'
CONST2  ENDS

;volgorde laten staan!
DGROUP  group DATA32, CONST32, C_COMMON, c_common, CONST2, CONST, BSS32, _BSS

CGROUP  group CODE32, _TEXT
