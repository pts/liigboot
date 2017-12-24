.PHONY: all clean

ifeq ($(HEXDATE),)
# Unix timestamp corresponding to `Fri Dec  8 20:07:37 GMT 2017'.
# Also 1512763657.
# Can be overridden in the command-line.
# !! Bump this when released.
HEXDATE2 := 0x5a2af109
else
HEXDATE2 := $(HEXDATE)
endif

all: liigresc_bs.bin liigboot_bs.bin

liigresc_bs.bin: liigboot_boot.nasm
	nasm -f bin -o $@ -DLIIGRESC liigboot_boot.nasm
liigboot_bs.bin: liigboot_boot.nasm
	nasm -f bin -o $@ -DLIIGBOOT liigboot_boot.nasm

liigresc_empty.img: liigboot_boot.nasm syslinux/core/ldlinux.bin
	nasm -f bin -o $@ -DLIIGRESC -DEMPTYFS liigboot_boot.nasm
liigboot_empty.img: liigboot_boot.nasm syslinux/core/ldlinux.bin
	nasm -f bin -o $@ -DLIIGBOOT -DEMPTYFS liigboot_boot.nasm

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

# All dependencies are listed here.
LDLINUX_BIN_TARGETS = core/ldlinux.bin core/ldlinux.raw core/ldlinux.elf core/ldlinux.lsr core/ldlinux.lst core/ldlinux.map core/ldlinux.o core/ldlinux.sec
$(addprefix syslinux/,$(LDLINUX_BIN_TARGETS)): syslinux/libcomcore/libcomcore.a syslinux/prebuilt/libcore.a syslinux/core/syslinux.ld syslinux/core/ldlinux.asm $(wildcard syslinux/core/*.inc)
	$(MAKE) -C syslinux $(LDLINUX_BIN_TARGETS) HEXDATE=$(HEXDATE2)
SYSLINUX_VERSION_TARGETS = version.gen version.h version.mk
$(addprefix syslinux/,$(SYSLINUX_VERSION_TARGETS)): syslinux/version syslinux/version.pl
	$(MAKE) -C syslinux $(SYSLINUX_VERSION_TARGETS)
syslinux/lzo/prepcore: syslinux/lzo/prepcore.c syslinux/lzo/prepcore_lzo.c syslinux/lzo/prepcore_lzo.h syslinux/lzo/prepcore_lzo_miniacc.h
	$(MAKE) -C syslinux lzo/prepcore
syslinux/libcomcore/libcomcore.a: $(wildcard $(addprefix syslinux/,libcomcore/include/*.[chsS] libcomcore/include/bitsize/*.[chsS] libcomcore/include/klibc/*.[chsS] libcomcore/include/sys/*.[chsS] libcomcore/libgcc/*.[chsS] libcomcore/sys/*.[chsS]))
	$(MAKE) -C syslinux libcomcore/libcomcore.a

clean:
	rm -f liigresc_bs.bin liigboot_bs.bin liigresc_empty.img liigboot_empty.img liigboot.img liigboot.img.tmp liigboot.img.install liigboot.img.ziptmp liigboot.zip mcopy.tmp
	$(MAKE) -C syslinux clean
