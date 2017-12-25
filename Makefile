.PHONY: all clean

ifeq ($(HEXDATE),)
# Unix timestamp corresponding to `Fri Dec  8 20:07:37 GMT 2017'.
# Also 1512763657.
# Can be overridden on the command-line.
# !! Bump this when released.
HEXDATE2 := 0x5a2af109
else
HEXDATE2 := $(HEXDATE)
endif

ifeq ($(LOAD_ADDR),)
LOAD_ADDR2 := 0x8800
else
LOAD_ADDR2 := $(LOAD_ADDR)
endif

# Can be overridden on the command-line:
# * make liigboot.zip LIIGMAIN=hiiimain.compressed.bin
# Compressed.
LIIGMAIN := liigmain.bin

BOOT_DEFINES = -DLOAD_ADDR=$(LOAD_ADDR2)
EMPTYFS_DEFINES = $(BOOT_DEFINES) -DEMPTYFS -DLIIGMAIN="'$(LIIGMAIN)'"

all: liigresc_bs.bin liigboot_bs.bin

liigresc_bs.bin: liigboot_boot.nasm
	nasm -f bin -o $@ -DLIIGRESC $(BOOT_DEFINES) liigboot_boot.nasm
liigboot_bs.bin: liigboot_boot.nasm
	nasm -f bin -o $@ -DLIIGBOOT $(BOOT_DEFINES) liigboot_boot.nasm
hiiimain.uncompressed.bin: hiiimain.nasm
	nasm -f bin -o $@ $(BOOT_DEFINES) hiiimain.nasm

liigresc_empty.img: liigboot_boot.nasm $(LIIGMAIN)
	nasm -f bin -o $@ -DLIIGRESC $(EMPTYFS_DEFINES) liigboot_boot.nasm
liigboot_empty.img: liigboot_boot.nasm $(LIIGMAIN)
	nasm -f bin -o $@ -DLIIGBOOT $(EMPTYFS_DEFINES) liigboot_boot.nasm

liigboot.img: liigboot_empty.img external/memtest86+-5.01.kernel syslinux.cfg
	cp -a liigboot_empty.img $@.tmp
	cp -a syslinux.cfg mcopy.tmp
	python -c 'import os, sys; mtime = int(sys.argv[1], 0); os.utime(sys.argv[2], (mtime, mtime))' $(HEXDATE2) mcopy.tmp
	tools/mtools -c mcopy -m -i $@.tmp mcopy.tmp ::syslinux.cfg
	cp -a external/memtest86+-5.01.kernel mcopy.tmp
	python -c 'import os, sys; mtime = int(sys.argv[1], 0); os.utime(sys.argv[2], (mtime, mtime))' $(HEXDATE2) mcopy.tmp
	tools/mtools -c mcopy -m -i $@.tmp mcopy.tmp ::memtest.k
	rm -f mcopy.tmp
	mv $@.tmp $@

liigboot.img.install: install.c
	gcc -m32 -D__LINTINY__ -D__LINTINY_DEFAULTLIBS__ -fno-stack-protector -fomit-frame-pointer -fno-ident -fno-builtin-exit -fno-builtin-_exit -fno-builtin-_Exit -fno-unwind-tables -fno-asynchronous-unwind-tables -isystem lintiny -Os -falign-functions=1 -mpreferred-stack-boundary=2 -falign-jumps=1 -falign-loops=1 -s -static -nostdlib -nostdinc -Wl,--build-id=none -Wl,-T,lintiny/lintiny.scr -W -Wall -Wextra -Werror -o $@ install.c lintiny/liblintiny.a
#	xtiny gcc -W -Wall -Wextra -Werror -o $@ install.c
#	gcc -m32 -s -static -nostdlib -nostdinc -Wl,--build-id=none -Wl,-T,lintiny/lintiny.scr -o liigboot.img.install o/install.o o/_start.o o/strcmp.o o/memcmp.o o/strcpy.o

liigboot.img.install.debug: install.c
	xstatic gcc -g -DDEBUG -W -Wall -Wextra -Werror -o $@ install.c

liigboot.zip: liigboot.img liigboot.img.install mkzip.py
	python mkzip.py --do-add-install-zip --mtime=$(HEXDATE2) liigboot.img

liigmain.bin: syslinux/core/ldlinux.raw bmcompress.py
	python bmcompress.py --bin=$< --out=$@ --load-addr=$(LOAD_ADDR2) --skip0=$(LOAD_ADDR2)
.PRECIOUS: hiiimain.compressed.bin
%.compressed.bin: %.uncompressed.bin bmcompress.py
	python bmcompress.py --bin=$< --out=$@ --load-addr=$(LOAD_ADDR2)

# All dependencies are listed here.
LDLINUX_BIN_TARGETS = core/ldlinux.raw core/ldlinux.elf core/ldlinux.lsr core/ldlinux.lst core/ldlinux.map core/ldlinux.o core/ldlinux.sec
$(addprefix syslinux/,$(LDLINUX_BIN_TARGETS)): syslinux/libcomcore/libcomcore.a syslinux/libcore/libcore.a syslinux/core/syslinux.ld syslinux/core/ldlinux.asm $(wildcard syslinux/core/*.inc)
	$(MAKE) -C syslinux $(LDLINUX_BIN_TARGETS) HEXDATE=$(HEXDATE2) LOAD_ADDR=$(LOAD_ADDR2)
SYSLINUX_VERSION_TARGETS = version.gen version.h version.mk
$(addprefix syslinux/,$(SYSLINUX_VERSION_TARGETS)): syslinux/version syslinux/version.pl
	$(MAKE) -C syslinux $(SYSLINUX_VERSION_TARGETS)
syslinux/libcomcore/libcomcore.a: $(wildcard $(addprefix syslinux/,libcomcore/include/*.[chsS] libcomcore/include/bitsize/*.[chsS] libcomcore/include/klibc/*.[chsS] libcomcore/include/sys/*.[chsS] libcomcore/libgcc/*.[chsS] libcomcore/sys/*.[chsS]))
	$(MAKE) -C syslinux libcomcore/libcomcore.a
syslinux/libcore/libcore.a: $(wildcard $(addprefix syslinux/,libcore/include/*.h libcore/fs/*.[chsS] libcore/fs/*/*.[chsS] libcore/mem/*.[chsS] libcore/codepage.cp))
	$(MAKE) -C syslinux libcore/libcore.a

clean:
	rm -f liigresc_bs.bin liigboot_bs.bin liigresc_empty.img liigboot_empty.img liigboot.img liigboot.img.tmp liigboot.img.install liigboot.img.ziptmp liigboot.zip mcopy.tmp liigmain.bin hiiimain.uncompressed.bin hiiimain.compressed.bin
	$(MAKE) -C syslinux clean
