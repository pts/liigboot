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

ifeq ($(LOAD_ADDR),)
LOAD_ADDR2 := 0x8000
else
LOAD_ADDR2 := $(LOAD_ADDR)
endif

.PHONY: all clean

all: version.gen version.h version.mk core/ldlinux.raw

# All dependencies are listed here.
LDLINUX_RAW_TARGETS = core/ldlinux.raw core/ldlinux.elf core/ldlinux.lsr core/ldlinux.lst core/ldlinux.map core/ldlinux.o core/ldlinux.sec
$(LDLINUX_RAW_TARGETS): version.gen version.h version.mk libcomcore/libcomcore.a libcore/libcore.a core/syslinux.ld core/ldlinux.asm $(wildcard core/*.inc)
	$(MAKE) -C core $(notdir $(LDLINUX_RAW_TARGETS)) HEXDATE=$(HEXDATE2) LOAD_ADDR=$(LOAD_ADDR2)
libcomcore/libcomcore.a: $(wildcard libcomcore/include/*.[chsS] libcomcore/include/bitsize/*.[chsS] libcomcore/include/klibc/*.[chsS] libcomcore/include/sys/*.[chsS] libcomcore/libgcc/*.[chsS] libcomcore/sys/*.[chsS])
	$(MAKE) -C libcomcore $(notdir $@)
libcore/libcore.a: $(wildcard libcore/include/*.h libcore/fs/*.[chsS] libcore/fs/*/*.[chsS] libcore/mem/*.[chsS]) libcore/codepage.cp
	$(MAKE) -C libcore $(notdir $@)

version.gen: version version.pl
	$(PERL) version.pl $< $@ '%define < @'
version.h: version version.pl
	$(PERL) version.pl $< $@ '#define < @'
version.mk: version version.pl
	$(PERL) version.pl $< $@ '< := @'

clean:
	rm -f version.gen version.h version.mk
	$(MAKE) -C core clean
	$(MAKE) -C libcomcore clean
	$(MAKE) -C libcore clean
