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
#GCCOPT += $(call gcc_ok,-mpreferred-stack-boundary=2,)
endif

INCLUDE	= -I.
STRIP	= strip --strip-all -R .comment -R .note

# zlib and libpng configuration flags
LIBFLAGS = -DDYNAMIC_CRC_TABLE -DPNG_NO_CONSOLE_IO \
	   -DPNG_NO_WRITE_SUPPORTED \
	   -DPNG_NO_MNG_FEATURES \
	   -DPNG_NO_READ_tIME -DPNG_NO_WRITE_tIME

# We need some features in libpng which apparently aren't available in the
# fixed-point versions.  It's OK, because we have to have a non-graphical
# fallback anyway, just use that on old machines...
# LIBFLAGS += -DPNG_NO_FLOATING_POINT_SUPPORTED

REQFLAGS  = $(GCCOPT) -g -mregparm=3 -DREGPARM=3 -Iinclude
OPTFLAGS  = -Os -falign-functions=0 -falign-jumps=0 \
	    -falign-labels=0 -ffast-math -fomit-frame-pointer
WARNFLAGS = $(GCCWARN) -Wpointer-arith -Wwrite-strings -Wstrict-prototypes -Winline

CFLAGS  = $(BASEGCCFLAGS) $(OPTFLAGS) $(REQFLAGS) $(WARNFLAGS) $(LIBFLAGS)

.SUFFIXES: .c .o .a .so .lo .i .S .s .ls .ss .lss

% : %.c # Cancel default rule

% : %.S

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

.c.i:
	$(CC) $(CFLAGS) -E -o $@ $<

.c.s:
	$(CC) $(CFLAGS) -S -o $@ $<

.S.o:
	$(CC) $(CFLAGS) -D__ASSEMBLY__ -c -o $@ $<

.S.s:
	$(CC) $(CFLAGS) -D__ASSEMBLY__ -E -o $@ $<

.S.lo:
	$(CC) $(CFLAGS) $(SOFLAGS) -D__ASSEMBLY__ -c -o $@ $<

.S.ls:
	$(CC) $(CFLAGS) $(SOFLAGS) -D__ASSEMBLY__ -E -o $@ $<

.s.o:
	$(CC) $(CFLAGS) -x assembler -c -o $@ $<

.ls.lo:
	$(CC) $(CFLAGS) $(SOFLAGS) -x assembler -c -o $@ $<

.c.lo:
	$(CC) $(CFLAGS) $(SOFLAGS) -c -o $@ $<

.c.ls:
	$(CC) $(CFLAGS) $(SOFLAGS) -S -o $@ $<
