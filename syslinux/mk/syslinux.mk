## -*- makefile -*- -------------------------------------------------------
##   
##   Copyright 2008 H. Peter Anvin - All Rights Reserved
##
##   This program is free software; you can redistribute it and/or modify
##   it under the terms of the GNU General Public License as published by
##   the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
##   Boston MA 02110-1301, USA; either version 2 of the License, or
##   (at your option) any later version; incorporated herein by reference.
##
## -----------------------------------------------------------------------

##
## Common configurables
##

# No builtin rules
MAKEFLAGS += -r
MAKE      += -r

BINDIR   = /usr/bin
SBINDIR  = /sbin
LIBDIR   = /usr/lib
DATADIR  = /usr/share
AUXDIR   = $(DATADIR)/syslinux
DIAGDIR  = $(AUXDIR)/diag
MANDIR	 = /usr/man
INCDIR   = /usr/include
TFTPBOOT = /tftpboot
COM32DIR = $(AUXDIR)/com32

BOOTDIR	    = /boot
EXTLINUXDIR = $(BOOTDIR)/extlinux

NASM	 = TZ= $(topdir)/../tools/nasm
NASMOPT  = -Ox

PERL	 = $(topdir)/../tools/perl

CHMOD	 = chmod

CC	 = gcc
gcc_ok   = $(shell tmpf=gcc_ok.$$$$.tmp; \
		   if $(CC) $(GCCOPT) $(1) -c $(topdir)/dummy.c \
			-o $$tmpf 2>/dev/null ; \
		   then echo '$(1)'; else echo '$(2)'; fi; \
		   $(RM) -f $$tmpf)

LD	 = ld
OBJDUMP	 = objdump
OBJCOPY  = objcopy
AR       = ar
RM       = $(topdir)/../tools/rm

com32    = $(topdir)/com32

# Common warnings we want for all gcc-generated code
GCCWARN := -W -Wall -Wstrict-prototypes
# Extremely useful variant for debugging...
#GCCWARN += -Wno-clobbered -Werror

# Common stanza to make gcc generate .*.d dependency files
MAKEDEPS = -Wp,-MT,$@,-MD,$(dir $@).$(notdir $@).d

# Dependencies that exclude system headers; use whenever we use
# header files from the platform.
UMAKEDEPS = -Wp,-MT,$@,-MMD,$(dir $@).$(notdir $@).d
