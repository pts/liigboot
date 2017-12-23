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

.PHONY: all clean

all: version.gen version.h version.mk core/ldlinux.raw core/ldlinux.bss core/ldlinux.bin core/ldlinux.sys
	cmp core/ldlinux.raw.orig core/ldlinux.raw
	test \! -f core/ldlinux.bss.orig || cmp core/ldlinux.bss.orig core/ldlinux.bss
	cmp core/ldlinux.bin.orig core/ldlinux.bin
	test \! -f core/ldlinux.sys.orig || cmp core/ldlinux.sys.orig core/ldlinux.sys
	: Compare OK.

# All dependencies are listed here.
LDLINUX_RAW_TARGETS = core/ldlinux.raw core/ldlinux.elf core/ldlinux.lsr core/ldlinux.lst core/ldlinux.map core/ldlinux.o core/ldlinux.sec
$(LDLINUX_RAW_TARGETS): version.gen version.h version.mk prebuilt/libcomcore.a prebuilt/libcore.a core/syslinux.ld core/ldlinux.asm $(wildcard core/*.inc)
	$(MAKE) -C core $(notdir $(LDLINUX_RAW_TARGETS))
core/ldlinux.bss: core/ldlinux.raw
	$(MAKE) -C core $(notdir $@)
core/ldlinux.bin: core/ldlinux.raw lzo/prepcore
	$(MAKE) -C core $(notdir $@)
core/ldlinux.sys: core/ldlinux.bin
	$(MAKE) -C core $(notdir $@)
lzo/prepcore: lzo/prepcore.c lzo/src/*.[ch] lzo/src/*.ch lzo/include/lzo/*.h
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
