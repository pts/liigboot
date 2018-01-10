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
PYTHON   = $(topdir)/../tools/python -E
AR       = $(topdir)/../tools/ar
RM       = $(topdir)/../tools/rm
CC	 = gcc

BASEGCCFLAGS = -static -nostdlib -nostdinc -m32 -march=i686

gcc_ok   = $(shell tmpf=gcc_ok.$$$$.tmp; \
		   if $(CC) $(BASEGCCFLAGS) $(GCCOPT) $(1) -c $(topdir)/dummy.c \
			-o $$tmpf 2>/dev/null ; \
		   then echo '$(1)'; else echo '$(2)'; fi; \
		   $(RM) -f $$tmpf)

com32    = $(topdir)/com32

# Common warnings we want for all gcc-generated code
GCCWARN := -W -Wall -Wstrict-prototypes
# Extremely useful variant for debugging...
#GCCWARN += -Wno-clobbered -Werror
