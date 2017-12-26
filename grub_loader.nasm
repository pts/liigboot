; by pts@fazekas.hu at Tue Dec 26 00:22:21 CET 2017
bits 16
org 0x7c00
; Boot sector code. DL contains the boot drive (e.g. 0x80 for (hd0)).

; !! disable PXE
; !! compress
; !! patch config file name
; !! communicate drive number
; !! the original fallback_menu.lst
; !! where is it? why don't we have the pointer to it?
;    const char *preset_menu = PRESET_MENU_STRING;
; !! 
;
; pre_stage2:
; * GRUB4DOS grldr file offset 0x2000
; * Ubuntu stage2 file offset 0x200
; * GRUB4DOS grldr load to memory at 0x8200
; * mem_addr = file_offset_in_mygrub_bin + 0x8200 - 0x600
; * it's sector number 1, called `stage2_second_buffer'
;
; #define STAGE2_INSTALLPART      0x8
; #define STAGE2_SAVED_ENTRYNO    0xc
; #define STAGE2_VER_STR_OFFS     0x12
;
; from stage2/builtins.c
; (*((short *) (stage2_second_buffer + STAGE2_VER_MAJ_OFFS)) != COMPAT_VERSION)

; !! still root (hd0) is needed -- why isn't it automatic? where is it set? real_root_func? bootdev?
; use the `read' command to figure out? or gdb?
; !! force_lba

; from stage2/asm.S
;
; 0x00: jmp word 0:0x8270
; 0x05: db 0  ; !! who checks this byte (source code? pxe has effect)
;        bit 0 = 1: disable pxe !! in stage2/common.c: if (! ((*(char *)0x8205) & 0x01))   /* if it is not disable pxe */
;        bit 1 = 1: disable keyboard intervention in boot process
;        bit 2 = 1: disable the "unconditional command-line entrance" (UCE) feature; disabling it is DUCE
;        bit 3 = 1: disable geometry tune; chs-no-tune; don't call geometry_tune !! what does it mean?
; 0x06: db 3, 2  ; Compatibility version number: COMPAT_VERSION_MAJOR, COMPAT_VERSION_MINOR
; 0x08: dd 0xffffff  ; install_partition; STAGE2_INSTALLPART !! how is this used?
;       the paritition number is acually at the byte 0x0a (0-based offset)
;        grub_printf ("(%cd%d", (current_drive & 0x80) ? 'h' : 'f', current_drive & ~0x80);
;        if ((current_partition & 0xFF0000) != 0xFF0000)
;            grub_printf (",%d", (current_partition >> 16) & 0xFF);  /* partition number */
;        if ((current_partition & 0x00FF00) != 0x00FF00)
;            grub_printf (",%c", 'a' + ((current_partition >> 8) & 0xFF));
;        grub_printf (")");
;       printf ("Partition type for (hd%d,%d) is 0x%X.\n", (current_drive & 0x7F), (current_partition >> 16), new_type);
; 0x0c: dd 0x84cc  ; saved_entryno pointing to preset_menu  ; !! GRUB4DOS-specific commandline preset_menu
;       !! what is the built-in preset_menu??
;       !! it's also called saved_entryno, STAGE2_SAVED_ENTRYNO
;       !! (0 = cleared; common.c)
;       mem addr 0x84cc is at file offset 0x8cc: dd 0x3b25c
;       preset_menu is at file offset 0x3366c; mem addr 0x3b26c
;       \x6c\xb2\x03\x00
;       Note: GRUB for DOS uses this for the commandline preset_menu.
;       A preset_menu can be embedded in the commandline of GRUB.EXE.
;       This new preset_menu overrides the built-in preset_menu.
;       If the variable is not touched, and the first byte at config_file is 0,
;       then the new menu at 0x0800 will work.
;       If the variable here is cleared to 0, or the first byte at config_file is
;       not 0, then the built-in preset_menu will work.
; 0x10: db 0  ; STAGE2_ID_STAGE2
; 0x11: db 0  ; !! force_lba 
; 0x12: db '0.97', 0  ; Version string. Ubuntu stage1 has the same.
; 0x17: db '/boot/grub/menu.lst', 0  ; Default config file name. The command `configfile' without arguments will load this as default argument. May or may not start with (hd0)/... etc.
; 0x2b: db 0, ...
; 0x6c: dd 0x3b25c  ; __bss_start, same as edata
; 0x70: db 0xeb, 0x4e  ; jmp short 0x82c0 ; real_codestart
; 0x72: ... padding of db 0, ...
; 0x80: dd 0  ; boot_drive  (will be set in asm.S)
; 0x84: dd 0  ; pxe_yip
; 0x88: dd 0  ; pxe_sip
; 0x8c: dd 0  ; pxe_gip
; 0x90: dq 0  ; filesize
; 0x94: dd 0  ; saved_mem_upper
; 0x98: dd 0  ; saved_partition
; 0x9c: dd 0  ; saved_drive
; 0xa0: dd 0  ; no_decompression
; 0xa4: ... padding of db 0, ...
; 0xc0: 0xfa, 0xfc ; cli; cld ... boot code starts here

;        movb    %dh, 0x820A     /* this is the boot partition number */
;
;        #; clear saved_entryno so that force_cdrom_as_boot_device be cleared
;        #; later in common.c
;
;        movl    %ebp, 0x820C    /* EBP=0, clear saved_entryno */


jmp strict short entry2  ; db 0xeb, 0x3c
nop  ; db 0x90
times 0x3e-($-$$) db 0
; Code starts at mem=0x7c58 disk=0x58=90.
entry2:
cld
sti
xor ax, ax
mov ss, ax
mov sp, 0x2000
mov ds, ax
mov es, ax

mov ax, 0x0e00 + 'G'
mov bx, 7  ; Color for int 0x10 below. QEMU seems to be ignoring it.
int 0x10  ; Teletype output.

mov al, dl
sub al, 0x40
int 0x10  ; !! Remove '@'.

; Make sure to pass along the drive number in the original dl. GRUB4DOS
; respects it, and runs the corresponding `root (...)' command by default.
; An `xor xd, dx' here would break it.

push dx
jmp short decompress

; bmcompress.py compression header, --load_addr=0x7c30
; Make load_addr larger if the code above becomes longer.
load_addr equ 0x7c30
times load_addr+44-0x7c00-($-$$) db 0
decompress:
db 0xeb, 0x22
times 16 dw 0x5b53  ; This will be destroyed by compression.
db 0xfa, 0xf4
pop dx
jmp word 0:0x8200

; File offset 0x600
times 0x8200-0x7c00-($-$$) db 0
;incbin 'grldr', 0x2000
