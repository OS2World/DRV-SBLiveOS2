PROJ = daudio

all: $(PROJ).exe

CC     = icc
CXX    = icc
AS     = alp
LD     = icc

# Define project specific macros
PROJ_OBJS  = daudio.obj 

CFLAGS     = -Q  -Si -Ti -Tm+ -Ge- -Ss+ -W3 -Gm+ /Gn- -I..\include
CXXFLAGS   = -Q  -Si -Ti -Tm+ -Ge- -Ss+ -W3 -Gm+ /Gn- -I..\include

CFLAGS = $(CFLAGS) /Ge+
CXXFLAGS = $(CXXFLAGS) /Ge+
LDTARGETFLAGS    = -Ge+ -B"/pmtype:vio /stack:0x10000"
LDFLAGS_ODINCRT  = -Q   -B"/noe /map /packcode /packdata /exepack:2 /dbgpack " -Ti -Si -W3 -Gm+ $(LDTARGETFLAGS)
LDFLAGSEXE = $(LDFLAGS_ODINCRT) /Ge+ /B"/pmtype:vio /stack:0x30000 /NOBASE /Map"

.c.obj:
    $(CC) $(CFLAGS) $(CINCLUDES) $(CDEFINES) -c $<

.cpp.obj:
    $(CXX) $(CXXFLAGS) $(CINCLUDES) $(CDEFINES) -c $<

# Build rule for EXE
$(PROJ).EXE: $(PROJ_OBJS) 
        $(LD) $(LDFLAGSEXE) -Fe$@ $(PROJ_OBJS)

# Rules for cleaning out those old files
clean:
    del *.bak *.pdb *.obj *.res *.exp *.map *.sbr *.bsc
