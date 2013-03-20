# $Id: Makefile,v 1.1 2000/04/23 14:55:41 ktk Exp $ 

# Comment/uncomment the following line to disable/enable build options
DEBUG = n
KERNELVERSION = auto
KERNELSOURCE=/usr/src/linux

ifneq ($(shell grep '__rh_kernel_version_h__' $(KERNELSOURCE)/include/linux/version.h),)
# redhat kernel headers style
  SUB := $(shell grep __BOOT_KERNEL_ /boot/kernel.h 2>/dev/null | grep 1 | cut -d\_ -f5 | tr [:upper:] [:lower:] | cut -d' ' -f1)
  VERSION_H := version-$(SUB).h
  AUTOCONF_H := autoconf-$(SUB).h
else
  VERSION_H := version.h
  AUTOCONF_H := autoconf.h
endif

ifeq ($(KERNELVERSION),auto)
  KERNELVERSION := $(shell grep UTS_RELEASE $(KERNELSOURCE)/include/linux/$(VERSION_H) 2>/dev/null | cut -d\" -f2)
endif

KERNELRELEASE := $(shell echo $(KERNELVERSION) | cut -d\" -f2| cut -c-3)

ifeq ($(KERNELRELEASE),2.2)
  KERNEL_EXTRA_INCLUDES := -I2.2/
else
  ifeq ($(KERNELRELEASE),2.3)
    KERNELRELEASE := 2.4
    KERNEL_EXTRA_INCLUDES := -I./
  endif
endif

# extract config options from the kernel headers
dummy := $(shell grep 'define' $(KERNELSOURCE)/include/linux/$(AUTOCONF_H) | grep 'CONFIG' | awk '{ print $$2 "=y"}' - > .config)

include $(KERNELRELEASE)/Makefile

#CFLAGS += -DPRIVATE_PCM_VOLUME

ifeq ($(DEBUG),y)
  CFLAGS := $(CFLAGS) -DEMU10K1_DEBUG
endif

CFLAGS := $(CFLAGS) $(MODFLAGS) $(KERNEL_EXTRA_INCLUDES)

EMU10K1 = emu10k1.o
EMU10K1-JOY = emu10k1-joy.o
SRC = audio.c cardmi.c cardmo.c cardwi.c cardwo.c efxmgr.c emuadxmg.c\
      hwaccess.c irqmgr.c main.c midi.c mixer.c osutils.c recmgr.c timer.c voicemgr.c 

all: .depend $(EMU10K1) $(EMU10K1-JOY) 

$(EMU10K1): $(SRC:.c=.o)
	$(LD) -r $^ -o $@

install:
	mkdir -p /lib/modules/$(KERNELVERSION)/misc
	install -c -m 664 $(EMU10K1) $(EMU10K1-JOY) /lib/modules/$(KERNELVERSION)/misc
	/sbin/depmod -a $(KERNELVERSION)
	/sbin/modprobe -r $(EMU10K1) $(EMU10K1-JOY) 

clean:
	rm -f core .config `find . -name '*.[oas]' -o -name '*~' -o -name core -o -name dep -o -name .depend`

depend .depend dep:
	@if [ ! -f $(KERNELSOURCE)/include/linux/modversions.h ]; then \
		echo "Please run make dep in your kernel source tree modversions.h is missing" ;\
		exit 1; fi
	@echo "Driver for KERNELVERSION = $(KERNELVERSION)"
	$(CC) $(CFLAGS) -M $(SRC) > $@

ifeq (.depend,$(wildcard .depend))
include .depend
endif

