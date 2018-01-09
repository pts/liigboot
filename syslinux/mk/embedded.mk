## -*- makefile -*- ------------------------------------------------------
##
##   Copyright 2001-2008 H. Peter Anvin - All Rights Reserved
##
##   This program is free software; you can redistribute it and/or modify
##   it under the terms of the GNU General Public License as published by
##   the Free Software Foundation, Inc., 53 Temple Place Ste 330,
##   Boston MA 02111-1307, USA; either version 2 of the License, or
##   (at your option) any later version; incorporated herein by reference.
##
## -----------------------------------------------------------------------

##
## Make configuration for embedded directories
##

include $(MAKEDIR)/syslinux.mk

ifeq ($(MAKECMDGOALS),clean)
GCCOPT :=
else
GCCOPT :=
GCCOPT += $(call gcc_ok,-ffreestanding,)
GCCOPT += $(call gcc_ok,-fno-stack-protector,)
GCCOPT += $(call gcc_ok,-fwrapv,)
GCCOPT += $(call gcc_ok,-freg-struct-return,)
GCCOPT += -Os -fomit-frame-pointer -mregparm=3 -DREGPARM=3 -msoft-float
GCCOPT += $(call gcc_ok,-fno-exceptions,)
GCCOPT += $(call gcc_ok,-fno-asynchronous-unwind-tables,)
GCCOPT += $(call gcc_ok,-fno-strict-aliasing,)
GCCOPT += $(call gcc_ok,-falign-functions=0,-malign-functions=0)
GCCOPT += $(call gcc_ok,-falign-jumps=0,-malign-jumps=0)
GCCOPT += $(call gcc_ok,-falign-labels=0,-malign-labels=0)
GCCOPT += $(call gcc_ok,-falign-loops=0,-malign-loops=0)
GCCOPT += $(call gcc_ok,-mpreferred-stack-boundary=2,)
GCCOPT += $(call gcc_ok,-mincoming-stack-boundary=2,)
endif

# Note: use += for CFLAGS and SFLAGS in case something is set in MCONFIG.local
CFLAGS    = $(BASEGCCFLAGS) $(GCCOPT) -g $(GCCWARN) -Wno-sign-compare $(OPTFLAGS) $(INCLUDES)

.SUFFIXES: .c .o .S .s .i .elf .com .bin .asm .lst .c32 .lss

%.o: %.c
	$(CC) $(MAKEDEPS) $(CFLAGS) -c -o $@ $<
%.i: %.c
	$(CC) $(MAKEDEPS) $(CFLAGS) -E -o $@ $<
%.s: %.c
	$(CC) $(MAKEDEPS) $(CFLAGS) -S -o $@ $<
# -Wa,-a=$*.lst
%.o: %.S
	$(CC) $(MAKEDEPS) $(CFLAGS) -D__ASSEMBLY__ -c -o $@ $<
%.s: %.S
	$(CC) $(MAKEDEPS) $(CFLAGS) -D__ASSEMBLY__ -E -o $@ $<
