; $Id: end.asm,v 1.1 2000/04/23 14:55:23 ktk Exp $ 


; Labels that signal the end of the code & data segments 
;
; Copyright (C) 2000  Sander van Leeuwen
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

	include sbseg.inc

CODE32 segment
       public __OffFinalCS32
__OffFinalCS32 label byte
CODE32 ends

_BSS segment
       public __OffFinalDS32
__OffFinalDS32 label byte
_BSS ends

	end
