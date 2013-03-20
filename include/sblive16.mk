#
# Target directories.
# Both bin and lib directories are compiler dependent.
#
!ifndef SBLIVE_BIN
!  if "$(DEBUG)" == "1"
SBLIVE_BIN  = $(SBLIVE_BIN_)\Debug
SBLIVE_BIN__= $(SBLIVE_BIN_)\Debug
!  else
SBLIVE_BIN  = $(SBLIVE_BIN_)\Release
SBLIVE_BIN__= $(SBLIVE_BIN_)\Release
!  endif
!endif

!ifndef SBLIVE_LIB
!  if "$(DEBUG)" == "1"
SBLIVE_LIB  = $(SBLIVE_LIB_)\Debug
SBLIVE_LIB__= $(SBLIVE_LIB_)\Debug
!  else
SBLIVE_LIB  = $(SBLIVE_LIB_)\Release
SBLIVE_LIB__= $(SBLIVE_LIB_)\Release
!  endif
!endif

!ifndef OBJDIR
!  if "$(DEBUG)" == "1"
OBJDIR   = bin\Debug
!    else
OBJDIR   = bin\Release
!  endif
!endif
