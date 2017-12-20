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

clean:
	rm -f liigresc_bs.bin liigboot_bs.bin liigresc_empty.img liigboot_empty.img
