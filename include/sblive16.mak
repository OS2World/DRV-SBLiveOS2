
CREATEPATH=$(SBLIVE_TOOLS)\CreatePath.cmd
WAT2MAP=$(SBLIVE_TOOLS)\wat2map.cmd

#########################################
# Options for Watcom 16-bit C compiler
#########################################
#  -bt=os2   = Build target OS is OS/2
#  -ms       = Memory model small
#  -3        = Enable use of 80386 instructions
#  -4        = Optimize for 486 (assumes -3)
#  -5        = Optimize for Pentium (assumes -3)
#  -j        = char default is unsigned
#  -d1       = Include line number info in object
#              (necessary to produce assembler listing)
#  -d2       = Include debugging info for ICAT
#              (necessary to produce assembler listing)
#  -o        = Optimization - i = enable inline intrinsic functions
#                             r = optimize for 80486 and pentium pipes
#                             s = space is preferred to time
#                             l = enable loop optimizations
#                             a = relax aliasing constraints
#                             n = allow numerically unstable optimizations
#  -s        = Omit stack size checking from start of each function
#  -zl       = Place no library references into objects
#  -wx       = Warning level set to maximum (vs 1..4)
#  -zfp      = Prevent use of FS selector
#  -zgp      = Prevent use of GS selector
#  -zq       = Operate quietly
#  -zm       = Put each function in its own segment
#  -zu       = Do not assume that SS contains segment of DGROUP
#
CC =wcc
CPP=wpp

CINCLUDES=-i$(%WATCOM)\H;$(%WATCOM)\H\SYS;$(INCLUDE)

!if "$(DEBUG)" == "1"
CFLAGS  =-ms -5 -bt=os2 -hc -d2 -oi -s -j -wx -zl -zfp -zgp -zq -zu -zp1 -DDEBUG -DTARGET_OS216 $(CINCLUDES)
CPPFLAGS=-ms -5 -bt=os2 -hc -d2 -oi -s -j -wx -zl -zfp -zgp -zq -zu -zp1 -DDEBUG -DTARGET_OS216 $(CINCLUDES)
LFLAGS  = d codeview
!else
CFLAGS  =-ms -5 -bt=os2         -oi -s -j -wx -zl -zfp -zgp -zq -zu -zp1 -DTARGET_OS216 $(CINCLUDES)
CPPFLAGS=-ms -5 -bt=os2    -olinars -s -j -wx -zl -zfp -zgp -zq -zu -zp1 -DTARGET_OS216 $(CINCLUDES)
LFLAGS  =
!endif

#########################################
# Options for Watcom assembler
#########################################
#  -bt=os2   = Build target OS is OS/2
#  -d1       = Include line number info in object
#              (necessary to produce assembler listing)
#  -i        = Include list
#  -zq       = Operate quietly
#  -3p       = 80386 protected-mode instructions
#
ASM=wasm
AFLAGS=-d1 -zq -3p -i
LINK=wlink $(LFLAGS)

#########################################
# Inference rules
#########################################

.obj: $(OBJDIR)

.c.obj: .AUTODEPEND
     $(CC) $(CPPFLAGS) -fo$(OBJDIR)\$^&.obj $^&.c

.cpp.obj: .AUTODEPEND
     $(CPP) $(CPPFLAGS) -fo$(OBJDIR)\$^&.obj $^&.cpp

.asm.obj: .AUTODEPEND
     $(ASM) $(AFLAGS) -fo=$(OBJDIR)\$^&.obj $^&.asm

.BEFORE
   @if not exist .\$(OBJDIR) $(CREATEPATH) .\$(OBJDIR)
