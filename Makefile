.PHONY: all clean

all: liigresc_bs.bin liigboot_bs.bin

liigresc_bs.bin: liigboot_boot.nasm
	nasm -f bin -o $@ -DLIIGRESC liigboot_boot.nasm
liigboot_bs.bin: liigboot_boot.nasm
	nasm -f bin -o $@ -DLIIGBOOT liigboot_boot.nasm

liigresc_empty.img: liigboot_boot.nasm syslinux_liigboot.ldlinux.sys
	nasm -f bin -o $@ -DLIIGRESC -DEMPTYFS liigboot_boot.nasm
liigboot_empty.img: liigboot_boot.nasm syslinux_liigboot.ldlinux.sys
	nasm -f bin -o $@ -DLIIGBOOT -DEMPTYFS liigboot_boot.nasm

liigboot.img: liigboot_empty.img external/memtest86+-5.01.kernel syslinux.cfg
	cp -a liigboot_empty.img $@.tmp
	tools/mtools -c mcopy -i $@.tmp syslinux.cfg ::
	tools/mtools -c mcopy -i $@.tmp external/memtest86+-5.01.kernel ::memtest.k
	mv $@.tmp $@

liigboot.img.install: install.c
	xtiny gcc -W -Wall -Wextra -Werror -o $@ install.c

liigboot.img.install.debug: install.c
	xstatic gcc -g -DDEBUG -W -Wall -Wextra -Werror -o $@ install.c

liigboot.zip: liigboot.img liigboot.img.install mkzip.py
	python mkzip.py --do-add-install-zip liigboot.img

clean:
	rm -f liigresc_bs.bin liigboot_bs.bin liigresc_empty.img liigboot_empty.img liigboot.img liigboot.img.tmp liigboot.img.install liigboot.img.ziptmp liigboot.zip
