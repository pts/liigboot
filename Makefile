.PHONY: all clean

# TODO(pts): Remove redundancy with syslinux/mk/syslinux.mk
PYTHON = tools/python -E
PERL = tools/perl
NASM = TZ= tools/nasm  # `TZ=' just to avoid opening /etc/TZ.
CC = gcc
BASEGCCFLAGS = -static -nostdlib -nostdinc -m32 -march=i686

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
# The memory address to which the boot manager (syslinux/core/ldlinux.raw or
# liigmain.bin) is loaded to. We don't want too early, we are still
# retaining a copy of the MBR boot sector at 0x7c00...0x7e00, and reserving
# 0x7e00...0x8000 for a future buffer sounds useful. Also below 0x7c00 some
# initial variables are stored (see the `push dx'... instructions in
# liigboot_boot.nasm).
#
# The default used to be 0x8800, it also works as well as 0x8000.
LOAD_ADDR2 := 0x8000
else
LOAD_ADDR2 := $(LOAD_ADDR)
endif

# Can be overridden on the command-line:
# * make liigboot.zip LIIGMAIN=hiiimain.compressed.bin
# Compressed.
LIIGMAIN := liigmain.bin

BOOT_DEFINES = -DLOAD_ADDR=$(LOAD_ADDR2)
EMPTYFS_DEFINES = $(BOOT_DEFINES) -DEMPTYFS -DLIIGMAIN="'$(LIIGMAIN)'"

# Can be overridden on the command-line:
# * make liigboot.zip LIIGMAIN_SECTOR_COUNT=80
LIIGMAIN_SECTOR_COUNT=
ifneq ($(LIIGMAIN_SECTOR_COUNT),)
EMPTYFS_DEFINES += -DLIIGMAIN_SECTOR_COUNT=$(LIIGMAIN_SECTOR_COUNT)
endif

all: liigboot.zip

liigresc_bs.bin: liigboot_boot.nasm
	$(NASM) -f bin -o $@ -DLIIGRESC $(BOOT_DEFINES) $<
liigboot_bs.bin: liigboot_boot.nasm
	$(NASM) -f bin -o $@ -DLIIGBOOT $(BOOT_DEFINES) $<
hiiimain.uncompressed.bin: hiiimain.nasm
	$(NASM) -f bin -o $@ $(BOOT_DEFINES) $<
grub_loader.bin: grub_loader.nasm
	$(NASM) -f bin -o $@ $<
memtest.uncompressed.bs: memtest_loader.nasm external/memtest86+-5.01.kernel
	$(NASM) -f bin -o $@ $< -DMEMTEST_BIN="'external/memtest86+-5.01.kernel'"

liigresc_empty.img: liigboot_boot.nasm $(LIIGMAIN)
	$(NASM) -f bin -o $@ -DLIIGRESC $(EMPTYFS_DEFINES) liigboot_boot.nasm
liigboot_empty.img: liigboot_boot.nasm $(LIIGMAIN)
	$(NASM) -f bin -o $@ -DLIIGBOOT $(EMPTYFS_DEFINES) liigboot_boot.nasm

.PRECIOUS: liigboot.zip
.PRECIOUS: liigresc.zip
# !! Why is this up to date after `rm memtest.compressed.bs' ?
%.zip: %_empty.img memtest.compressed.bs syslinux.cfg.simplified menu.lst.simplified grub4dos.bs mkzip.py liigboot.img.install
	tools/cp -a $< $@.tmp
	$(PYTHON) copy_to_fat.py --img=$@.tmp --in=syslinux.cfg.simplified  --out=syslinux.cfg  --mtime=$(HEXDATE2)
	$(PYTHON) copy_to_fat.py --img=$@.tmp --in=menu.lst.simplified      --out=menu.lst      --mtime=$(HEXDATE2)
	$(PYTHON) copy_to_fat.py --img=$@.tmp --in=grub4dos.bs              --out=grub4dos.bs   --mtime=$(HEXDATE2)
	$(PYTHON) copy_to_fat.py --img=$@.tmp --in=memtest.compressed.bs    --out=memtest.bs    --mtime=$(HEXDATE2)
	$(PYTHON) mkzip.py --do-add-install-zip --install=liigboot.img.install --mtime=$(HEXDATE2) --img=$@.tmp
	tools/mv $@.tmp $@

liigboot.img.install: install.c
	$(CC) $(BASEGCCFLAGS) -D__LINTINY__ -D__LINTINY_DEFAULTLIBS__ -fno-stack-protector -fomit-frame-pointer -fno-ident -fno-builtin-exit -fno-builtin-_exit -fno-builtin-_Exit -fno-unwind-tables -fno-asynchronous-unwind-tables -isystem lintiny -Os -falign-functions=1 -mpreferred-stack-boundary=2 -falign-jumps=1 -falign-loops=1 -s -Wl,--build-id=none -Wl,-T,lintiny/lintiny.scr -W -Wall -Wextra -Werror -o $@ install.c lintiny/liblintiny.a
#	xtiny gcc -W -Wall -Wextra -Werror -o $@ install.c
#	gcc -m32 -s -static -nostdlib -nostdinc -Wl,--build-id=none -Wl,-T,lintiny/lintiny.scr -o liigboot.img.install o/install.o o/_start.o o/strcmp.o o/memcmp.o o/strcpy.o

liigboot.img.install.debug: install.c
	xstatic gcc -g -DDEBUG -W -Wall -Wextra -Werror -o $@ install.c

liigmain.bin: syslinux/core/ldlinux.raw tools/upxbc tools/upx
	$(PYTHON) tools/upxbc --upx=tools/upx --flat16 -f --in=$< --out=$@ --load-addr=$(LOAD_ADDR2) --skip0=$(LOAD_ADDR2) --sig-ofs-max=0x20
.PRECIOUS: hiiimain.compressed.bin
%.compressed.bin: %.uncompressed.bin tools/upxbc tools/upx
	$(PYTHON) tools/upxbc --upx=tools/upx --flat16 -f --in=$< --out=$@ --load-addr=$(LOAD_ADDR2) --sig-ofs-max=0x200
.PRECIOUS: memtest.compressed.bs
.PRECIOUS: memtest.uncompressed.bs
%.compressed.bs: %.uncompressed.bs tools/upxbc tools/upx
	$(PYTHON) tools/upxbc --upx=tools/upx --flat16 -f --in=$< --out=$@ --load-addr=0x7c00 --sig-ofs-max=0x200

# All dependencies are listed here.
LDLINUX_BIN_TARGETS = core/ldlinux.raw core/ldlinux.elf core/ldlinux.lsr core/ldlinux.lst core/ldlinux.map core/ldlinux.o core/ldlinux.sec
$(addprefix syslinux/,$(LDLINUX_BIN_TARGETS)): syslinux/libcomcore/libcomcore.a syslinux/libcore/libcore.a syslinux/core/syslinux.ld syslinux/core/ldlinux.asm $(wildcard syslinux/core/*.inc) elftool.py
	$(MAKE) -C syslinux $(LDLINUX_BIN_TARGETS) HEXDATE=$(HEXDATE2) LOAD_ADDR=$(LOAD_ADDR2)
SYSLINUX_VERSION_TARGETS = version.gen version.h version.mk
$(addprefix syslinux/,$(SYSLINUX_VERSION_TARGETS)): syslinux/version syslinux/version.pl
	$(MAKE) -C syslinux $(SYSLINUX_VERSION_TARGETS)
syslinux/libcomcore/libcomcore.a: $(wildcard $(addprefix syslinux/,libcomcore/include/*.[chsS] libcomcore/include/bitsize/*.[chsS] libcomcore/include/klibc/*.[chsS] libcomcore/include/sys/*.[chsS] libcomcore/libgcc/*.[chsS] libcomcore/sys/*.[chsS]))
	$(MAKE) -C syslinux libcomcore/libcomcore.a
syslinux/libcore/libcore.a: $(wildcard $(addprefix syslinux/,libcore/include/*.h libcore/fs/*.[chsS] libcore/fs/*/*.[chsS] libcore/mem/*.[chsS] libcore/codepage.cp))
	$(MAKE) -C syslinux libcore/libcore.a

grub4dos.bs: external/grub4dos-0.4.4.grldr fallback_menu.lst patch_grldr.py grub_loader.bin tools/upxbc tools/upx
	$(PYTHON) patch_grldr.py --out=$@ --in=$< --menu=fallback_menu.lst --loader=grub_loader.bin --do-compress
.PRECIOUS: syslinux.cfg.simplified
.PRECIOUS: menu.lst.simplified
%.simplified: % patch_grldr.py
	$(PYTHON) patch_grldr.py --out=$@ --menu=$<

clean:
	tools/rm -f *.tmp liigresc_bs.bin liigboot_bs.bin liigresc_empty.img liigboot_empty.img liigboot.img liigboot.zip.tmp liigresc.zip.tmp liigboot.img.install liigboot.zip.tmp.ziptmp liigresc.zip.tmp.ziptmp liigboot.zip mcopy.tmp liigmain.bin hiiimain.uncompressed.bin hiiimain.compressed.bin grldr syslinux.cfg.simplified menu.lst.simplified grub4dos.bs grub4dos.bs.tmp memtest.uncompressed.bs memtest.compressed.bs
	$(MAKE) -C syslinux clean
