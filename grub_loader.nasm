;
; grub_loader_nasm: boot sector code (+some more \0 bytes) to load GRUB4DOS
; by pts@fazekas.hu at Tue Dec 26 00:22:21 CET 2017
;
; GRUB pre_stage2:
;
; * pre_stage2 starts with (db '0.97', 0) at offset 0x12.
; * pre_stage2 is called `stage2_second_buffer' in the GRUB4DOS sources.
; * In GRUB4DOS grldr starts at file offset 0x2000.
; * (In Ubuntu stage2 starts at file offset 0x200).
; * GRUB4DOS grldr load to memory at 0x8200.
; * boot sector code is loaded by BIOS at 0x7c00.
; * Thus if the boot sector is loaded flat with pre_stage2 in the same image.
;   then pre_stage2 should start at 0x8200 - 0x7c00 = 0x600 in the image file.
;

; Boot sector code. BIOS has populated DL with the boot drive (e.g. 0x80 for
; (hd0)), DH the partition number, and DS:SI points to the partinfo.
bits 16
org 0x7c00

jmp strict short entry2  ; db 0xeb, 0x3c
nop  ; db 0x90
times 0x3e-($-$$) db 0
; Code starts at mem=0x7c3e disk=0x3e=62.
entry2:
cld
sti
xor ax, ax
mov ss, ax
mov sp, 0x2000
mov ds, ax
mov es, ax

mov ax, 0x0e00
mov bx, 7  ; Color for int 0x10 below. QEMU seems to be ignoring it.
mov al, dl  ; Boot drive: 0: (fd0), 0x80: (hd0).
rol al, 1
xor al, 96  ; `: (fd0), b: (fd1), a: (hd0), c: (hd1) ...
int 0x10  ; Teletype output (al as ASCII).

; Make sure to pass along the drive number in the original dl. GRUB4DOS
; respects it, and runs the corresponding `root (...)' command by default.
; An `xor dx, dx' here would break it.
push dx

jmp short decompress

; upxbc --flat16 compression signature, --load_addr=0x7c30
; Make load_addr larger if the code above becomes longer.
load_addr equ 0x7c30
times load_addr+44-0x7c00-($-$$) db 0
decompress:
db 0xeb, '?_SBARERP_COMPRESSION_WILL_BE_APPLIED_AFTER_THE_SLASH______LZMA/'
; Some stats about grub4dos.bs:
; 211437 bytes: uncompressed
; 107041 bytes: upx --ultra-brute --no-lzma
; 101403 bytes: upx --best --lzma

; At this point all registers except for ss and sp are overwritten by the
; decompressor.
pop dx  ; Pop the drive number.
jmp word 0:0x8200

; File offset 0x600
times 0x8200-0x7c00-($-$$) db 0
;incbin 'grldr', 0x2000
