; $Id: segments.inc,v 1.1 2000/04/23 14:55:40 ktk Exp $ 

;;DDK*************************************************************************/
;;                                                                           */
;; COPYRIGHT    Copyright (C) 1995 IBM Corporation                           */
;;                                                                           */
;;    The following IBM OS/2 WARP source code is provided to you solely for  */
;;    the purpose of assisting you in your development of OS/2 WARP device   */
;;    drivers. You may use this code in accordance with the IBM License      */
;;    Agreement provided in the IBM Device Driver Source Kit for OS/2. This  */
;;    Copyright statement may not be removed.                                */
;;                                                                           */
;;****************************************************************************/

;; Defines segment ordering for 16-bit DD's with MSC 6.0

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

_ENDCS        segment dword public use16 'CODE'
_ENDCS        ends

_INITTEXT     segment dword public use16 'CODE'
_INITTEXT     ends

DGROUP        group _HEADER, CONST, CONST2, _DATA, _BSS, _ENDDS, _INITDATA, _ENDINITDATA
CGROUP        group _TEXT, _ENDCS, _INITTEXT

