.PHONY: all clean

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
	tools/mtools -c mcopy -i $@.tmp syslinux.cfg ::
	tools/mtools -c mcopy -i $@.tmp external/memtest86+-5.01.kernel ::memtest.k
	mv $@.tmp $@

liigboot.img.install: install.c
	gcc -m32 -D__LINTINY__ -D__LINTINY_DEFAULTLIBS__ -fno-stack-protector -fomit-frame-pointer -fno-ident -fno-builtin-exit -fno-builtin-_exit -fno-builtin-_Exit -fno-unwind-tables -fno-asynchronous-unwind-tables -isystem lintiny -Os -falign-functions=1 -mpreferred-stack-boundary=2 -falign-jumps=1 -falign-loops=1 -s -static -nostdlib -nostdinc -Wl,--build-id=none -Wl,-T,lintiny/lintiny.scr -W -Wall -Wextra -Werror -o $@ install.c lintiny/liblintiny.a
#	xtiny gcc -W -Wall -Wextra -Werror -o $@ install.c
#	gcc -m32 -s -static -nostdlib -nostdinc -Wl,--build-id=none -Wl,-T,lintiny/lintiny.scr -o liigboot.img.install o/install.o o/_start.o o/strcmp.o o/memcmp.o o/strcpy.o

liigboot.img.install.debug: install.c
	xstatic gcc -g -DDEBUG -W -Wall -Wextra -Werror -o $@ install.c

liigboot.zip: liigboot.img liigboot.img.install mkzip.py
	python mkzip.py --do-add-install-zip liigboot.img

# All dependencies are listed here.
LDLINUX_BIN_TARGETS = core/ldlinux.bin core/ldlinux.raw core/ldlinux.elf core/ldlinux.lsr core/ldlinux.lst core/ldlinux.map core/ldlinux.o core/ldlinux.sec
$(addprefix syslinux/,$(LDLINUX_BIN_TARGETS)):  syslinux/prebuilt/libcomcore.a syslinux/prebuilt/libcore.a syslinux/core/syslinux.ld syslinux/core/ldlinux.asm $(wildcard syslinux/core/*.inc)
	$(MAKE) -C syslinux $(LDLINUX_BIN_TARGETS)
SYSLINUX_VERSION_TARGETS = version.gen version.h version.mk
$(addprefux syslinux/,$(SYSLINUX_VERSION_TARGETS)): syslinux/version syslinux/version.pl
	$(MAKE) -C syslinux $(SYSLINUX_VERSION_TARGETS)

clean:
	rm -f liigresc_bs.bin liigboot_bs.bin liigresc_empty.img liigboot_empty.img liigboot.img liigboot.img.tmp liigboot.img.install liigboot.img.ziptmp liigboot.zip
	$(MAKE) -C syslinux clean
