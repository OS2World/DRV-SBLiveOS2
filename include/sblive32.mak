
CREATEPATH=$(SBLIVE_TOOLS)\CreatePath.cmd
WAT2MAP=$(SBLIVE_TOOLS)\wat2map.cmd

#===================================================================
#
#   Auto-dependency information
#
#===================================================================
.ERASE
.SUFFIXES:
.SUFFIXES: .lst .obj .lib .cpp .c .asm .def

!if "$(DEBUG)" == "1"
CFLAGS  = -dDEBUG -bt=os2v2 -e60 -hc -d2 -5r -omlinear -s -w4 -ze -zdp -zl -zq -nt=CODE32 -zff -zgf -zp1
CPPFLAGS= -xd
ASFLAGS = -Mb -Li -Sv:M510
!else
CFLAGS  = -bt=os2v2 -e60 -5r -omlinear -s -w4 -ze -zdp -zl -zq -nt=CODE32 -zff -zgf -zp1
CPPFLAGS= -xd
ASFLAGS = -Mb -Li -Sv:M510
!endif

!if "$(KEE)" == "1"
CFLAGS  +=  -mf -DKEE
ASFLAGS += -D:KEE
!else
CFLAGS  += -mc -zu
!endif

CC      = WCC386 $(CFLAGS) $(CDEFINES) -i$(CINCLUDES)
CPP     = WPP386 $(CFLAGS) $(CPPFLAGS) $(CDEFINES) -i$(CINCLUDES)
ASM     = alp $(ASFLAGS) $(AINCLUDES)

DFLAGS  = -l -s
DIS     = WDISASM $(DFLAGS)

!if "$(DEBUG)" == "1"
LFLAGS  = system os2v2 physdevice option int, dosseg, map, eliminate, mang, tog sort global d codeview
!else
LFLAGS  = system os2v2 physdevice option int, dosseg, map, eliminate, mang, tog sort global
!endif
QFLAGS  = system os2 option quiet, map, align=512
LINK    = WLINK $(LFLAGS)

!if "$(DEBUG)" == "1"
BFLAGS  = -c -b -q -n
!else
BFLAGS  = -s -t -c -b -q -n
!endif
LIB     = WLIB $(BFLAGS)

IFLAGS  = /nologo
IMPLIB  = IMPLIB $(IFLAGS)

.obj: $(OBJDIR)
.lib: $(OBJDIR)

.obj.lst:
        $(DIS) $*

!ifdef EVERYTHING_AS_CPP
.c.obj: .AUTODEPEND
        $(CPP) -fo$(OBJDIR)\$^&.obj $^&.c
!else
.c.obj: .AUTODEPEND
        $(CC) -fo$(OBJDIR)\$^&.obj $^&.c
!endif

.cpp.obj: .AUTODEPEND
        $(CPP) -fo$(OBJDIR)\$^&.obj $^&.cpp

.asm.obj: .AUTODEPEND
        $(ASM) $*.asm -Fo:$(OBJDIR)\$^&.obj -Fl:$(OBJDIR)\$^&.lst

.def.lib:
        $(IMPLIB) $(OBJDIR)\$^&.lib $^&.def

.BEFORE
   @if not exist .\$(OBJDIR) $(CREATEPATH) .\$(OBJDIR)

