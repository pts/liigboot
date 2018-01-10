# -*- makefile -*-

include $(MAKEDIR)/syslinux.mk

ifeq ($(MAKECMDGOALS),clean)
GCCOPT :=
else
GCCOPT :=
GCCOPT += $(call gcc_ok,-std=gnu99,)
GCCOPT += $(call gcc_ok,-fno-stack-protector,)
GCCOPT += $(call gcc_ok,-fwrapv,)
GCCOPT += $(call gcc_ok,-freg-struct-return,)
GCCOPT += $(call gcc_ok,-fno-exceptions,)
GCCOPT += $(call gcc_ok,-fno-asynchronous-unwind-tables,)
GCCOPT += $(call gcc_ok,-fno-strict-aliasing,)
GCCOPT += $(call gcc_ok,-falign-functions=0,-malign-functions=0)
GCCOPT += $(call gcc_ok,-falign-jumps=0,-malign-jumps=0)
GCCOPT += $(call gcc_ok,-falign-labels=0,-malign-labels=0)
GCCOPT += $(call gcc_ok,-falign-loops=0,-malign-loops=0)
GCCOPT += $(call gcc_ok,-mpreferred-stack-boundary=2,)
endif

REQFLAGS  = $(GCCOPT) -g -mregparm=3 -DREGPARM=3 -Iinclude
OPTFLAGS  = -Os -falign-functions=0 -falign-jumps=0 \
	    -falign-labels=0 -ffast-math -fomit-frame-pointer
WARNFLAGS = $(GCCWARN) -Wpointer-arith -Wwrite-strings -Wstrict-prototypes -Winline

CFLAGS  = $(BASEGCCFLAGS) $(OPTFLAGS) $(REQFLAGS) $(WARNFLAGS)

.SUFFIXES: .c .o .a .i .S .s

% : %.c  # Cancel default rule.
% : %.S  # Cancel default rule.

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -D__ASSEMBLY__ -c -o $@ $<
