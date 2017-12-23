## -----------------------------------------------------------------------
##
##   Copyright 1998-2009 H. Peter Anvin - All Rights Reserved
##   Copyright 2009-2010 Intel Corporation; author: H. Peter Anvin
##
##   This program is free software; you can redistribute it and/or modify
##   it under the terms of the GNU General Public License as published by
##   the Free Software Foundation, Inc., 53 Temple Place Ste 330,
##   Boston MA 02111-1307, USA; either version 2 of the License, or
##   (at your option) any later version; incorporated herein by reference.
##
## -----------------------------------------------------------------------

#
# Makefile for the SYSLINUX core
#

# No builtin rules
MAKEFLAGS += -r
MAKE      += -r

topdir = ..
MAKEDIR = $(topdir)/mk
include $(MAKEDIR)/embedded.mk

OPTFLAGS =
INCLUDES = -I./include -I$(com32)/include

# This is very similar to cp437; technically it's for Norway and Denmark,
# but it's unlikely the characters that are different will be used in
# filenames by other users.
CODEPAGE = cp865

# All primary source files for the main syslinux files
NASMSRC	 := $(wildcard *.asm)
NASMHDR  := $(wildcard *.inc)
CSRC	 := $(wildcard *.c */*.c */*/*.c)
SSRC	 := $(wildcard *.S */*.S */*/*.S)
CHDR	 := $(wildcard *.h)
OTHERSRC := keywords
ALLSRC    = $(NASMSRC) $(NASMHDR) $(CSRC) $(SSRC) $(CHDR) $(OTHERSRC)

COBJ	 := $(patsubst %.c,%.o,$(CSRC))
SOBJ	 := $(patsubst %.S,%.o,$(SSRC))

LIB	 = libcore.a
LIBS     = ../prebuilt/libcore.a ../prebuilt/libcomcore.a
LIBOBJS	 = $(COBJ) $(SOBJ)

NASMDEBUG = -g -F dwarf
NASMOPT  += $(NASMDEBUG)

PREPCORE = ../lzo/prepcore

# CFLAGS	+= -DDEBUG=1

ifndef HEXDATE
# core/ldlinux.bss: 0x7d2f: dd LDLINUX_MAGIC^HEXDATE
# LDLINUX_MAGIC equ 0x3EB202FE
# syslinux-4.07/core/ldlinux.bss: LDLINUX_MAGIC^HEXDATE == 0x6f42f3a1  ; HEXDATE := 0x51f0f15f
# our ldlinux.bss:                LDLINUX_MAGIC^HEXDATE == 0x6498f3f7  ; HEXDATE := 0x5a2af109
HEXDATE := 0x5a2af109
endif
DATE := $(HEXDATE)

all: ldlinux.bin

kwdhash.gen: keywords genhash.pl
	$(PERL) genhash.pl < keywords > kwdhash.gen

.PRECIOUS: %.elf

%.raw: %.elf
	$(OBJCOPY) -O binary $< $@

# GNU make 3.82 gets confused by the first form
.PRECIOUS: %.raw

# Don't remove this temporary build artifact (%.o).
.PRECIOUS: %.o

%.bin: %.raw $(PREPCORE)
	$(PREPCORE) $< $@

%.o %.lsr: %.asm kwdhash.gen ../version.gen $(NASMHDR)
	$(NASM) -f elf $(NASMOPT) -DDATE_STR="'$(DATE)'" \
		-DHEXDATE="$(HEXDATE)" \
		-l $(<:.asm=.lsr) -o $(<:.asm=.o) -MP -MD .$(<:.asm=.o).d $<

%.elf %.lst %.map %.sec: %.o %.lsr $(LIBS) syslinux.ld
	$(LD) $(LDFLAGS) -T syslinux.ld -M -o $(<:.o=.elf) $< \
		--start-group $(LIBS) --end-group \
		> $(<:.o=.map)
	$(OBJDUMP) -h $(<:.o=.elf) > $(@:.elf=.sec)
	$(PERL) lstadjust.pl $(<:.o=.lsr) $(<:.o=.sec) $(<:.o=.lst)

#$(LIB): $(LIBOBJS)
#	rm -f $@
#	$(AR) cq $@ $^
#	$(RANLIB) $@

codepage.cp: ../codepage/$(CODEPAGE).cp
	cp -f $< $@

codepage.o: codepage.S codepage.cp

# Include dependencies file
#-include .*.d */.*.d */*/.*.d

clean:
	rm -f ldlinux.o ldlinux.elf ldlinux.lst ldlinux.lsr ldlinux.raw ldlinux.map ldlinux.sec ldlinux.bin .*.d
