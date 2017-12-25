MAKEFLAGS += -r
MAKE      += -r

topdir = ..
MAKEDIR = $(topdir)/mk
include $(MAKEDIR)/embedded.mk

LIBCOREOBJS = \
    call16.o console.o idle.o kaboom.o pmapi.o printf.o rllpack.o \
    strcasecmp.o strncasecmp.o fs/cache.o fs/chdir.o fs/diskio.o fs/fs.o \
    fs/getcwd.o fs/getfssec.o fs/loadhigh.o fs/newconfig.o fs/nonextextent.o \
    fs/readdir.o mem/free.o mem/init.o mem/malloc.o fs/btrfs/btrfs.o \
    fs/ext2/bmap.o fs/ext2/ext2.o fs/fat/fat.o fs/lib/close.o \
    fs/lib/loadconfig.o fs/lib/mangle.o fs/lib/searchconfig.o fs/ntfs/ntfs.o \
    codepage.o

INCLUDES = -Iinclude -I../libcomcore/include

HEADERS := $(wildcard include/*.h)

all: libcore.a

codepage.o: codepage.cp

MAKEDEPS =

libcore.a: $(LIBCOREOBJS)
	rm -f $@
	$(AR) cq $@ $^
	$(RANLIB) $@  

clean:
	rm -f libcore.a $(LIBCOREOBJS) .*.o.d sys/.*.o.d libgcc/.*.o.d *.lst
