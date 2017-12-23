#
# by pts@fazekas.hu at Sat Dec 23 12:51:04 CET 2017
#
# Based on: syslinux-4.07.tar.xz
#
# Build environment:
#
# * gcc: version 4.8.4 (Ubuntu 4.8.4-2ubuntu1~14.04.3) 
# * nasm: version 2.10.09 compiled on Dec 29 2013
#

topdir = .
MAKEDIR = $(topdir)/mk
include $(MAKEDIR)/syslinux.mk
#-include $(topdir)/version.mk

ifeq ($(HEXDATE),)
HEXDATE2 := 0x5a2af109
else
HEXDATE2 := $(HEXDATE)
endif
#export HEXDATE  # Doesn't show it explicily.

.PHONY: all clean

all: version.gen version.h version.mk core/ldlinux.raw core/ldlinux.bin

# All dependencies are listed here.
LDLINUX_RAW_TARGETS = core/ldlinux.raw core/ldlinux.elf core/ldlinux.lsr core/ldlinux.lst core/ldlinux.map core/ldlinux.o core/ldlinux.sec
$(LDLINUX_RAW_TARGETS): version.gen version.h version.mk prebuilt/libcomcore.a prebuilt/libcore.a core/syslinux.ld core/ldlinux.asm $(wildcard core/*.inc)
	$(MAKE) -C core $(notdir $(LDLINUX_RAW_TARGETS)) HEXDATE=$(HEXDATE2)
core/ldlinux.bin: core/ldlinux.raw lzo/prepcore
	$(MAKE) -C core $(notdir $@)
lzo/prepcore: lzo/prepcore.c lzo/prepcore_lzo.c lzo/prepcore_lzo.h lzo/prepcore_lzo_miniacc.h
	$(MAKE) -C lzo $(notdir $@)

version.gen: version version.pl
	$(PERL) version.pl $< $@ '%define < @'
version.h: version version.pl
	$(PERL) version.pl $< $@ '#define < @'
version.mk: version version.pl
	$(PERL) version.pl $< $@ '< := @'

clean:
	rm -f version.gen version.h version.mk
	$(MAKE) -C lzo clean
	$(MAKE) -C core clean
