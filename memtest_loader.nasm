;
; memtest_loader_nasm: boot sector code (0x200 bytes) to load memtest86+-5.01
; by pts@fazekas.hu at Tue Dec 26 00:22:21 CET 2017
;
; Important memory layout of original memtest86+-5.1 (offset is in the file
; memtest86+-5.01):
;
; * 0x07c00...0x07e00 (offset 0x0...0x200): ($BOOTSEG, bootsect.S)
;   boot sector, BIOS loads it here
; * 0x10000...0x3bfb0 (offset 0xa00...0x2c9b0): ($TSTLOAD segment, *.c)
;   system (memtest) code, entry point is at the beginning, setup does an
;   ljmp $KERNEL_CS : 0x10000, where start_32 resides (the entry point).
;   (How is it arranged by the linker script that this code starts here?)
; * 0x90000...0x90200: boot sector will be copied here ($INITSEG)
; * 0x90200...0x90400 (offset 0x200..0xa00): ($SETUPSEG, setup.S)
;   setup (setup.S), the boot
;   sector loads it here; actually the first 0xae bytes are enough;
;   the boot sector does a `jmp word 0x9020:0'; it can be entered with
;   any data and segment registers.
;
; Layout of this memtest_loader.bin:
;
; * 0x07c00...0x07e00 (offset 0x0...0x200): memtest_loader.bin code
; * 0x07e00...0x08000 (offset 0x200...0x400): first 512 bytes of setup code
; * 0x08000...0x10000 (offset 0x400...0x8400): padding of '\0' * 32768
; * 0x10000...0x3bfb0 (offset 0x8400...0x343b0): system (memtest) code
;
; Since the layout of memtest_loader.bin is consecutive in memory, and
; Syslinux loads the entire .bs file, all memtest_loader.bin has to do is
; copying the setup code from segment 0x7e0 to segment 0x9020, and jump to
; it (jmp word 0x9020:0).
;

setup_from_seg equ 0x7e0
setup_to_seg equ 0x9020
setup_size equ 0x200  ; Even less, 0xae can be enough.

; Boot sector code. BIOS has populated DL with the boot drive (e.g. 0x80 for
; (hd0)), DH the partition number, and DS:SI points to the partinfo.
bits 16
org 0x7c00

jmp strict short entry2  ; db 0xeb, 0x3c
nop  ; db 0x90
times 0x3e-($-$$) db 0
; Code starts at mem=0x7c3e disk=0x3e=64.
; $ ndisasm -b 16 -o 0x7c3e -e 0x3e memtest.uncompressed.bin | less
entry2:
cld
sti
xor ax, ax
mov ss, ax
mov sp, 0x2000  ; Set up stack.

jmp short decompress
; Make load_addr larger if the code above becomes longer.
; !! Better automatic alignment.
load_addr equ 0x7c30
times load_addr+44-0x7c00-($-$$) db 0
decompress:

; upxbc --flat16 compression signature, --load_addr=0x7c30
; memtest86+-5.01.kernel: 182704 bytes
; memtest.uncompressed.bs: 213936 bytes
; memtest.compressed.bs --ultra-brute: 57633 bytes
; memtest.compressed.bs --lzma: 49214 bytes
;db 0xeb, '"SBAFEUU_COMPRESSION_AFTER_SLASH__/'
db 0xeb, '?_SBARERP_COMPRESSION_WILL_BE_APPLIED_AFTER_THE_SLASH______LZMA/'

; At this point all registers except for ss and sp are overwritten by the
; decompressor.
mov ax, setup_from_seg
mov ds, ax
mov ax, setup_to_seg
mov es, ax
xor si, si
xor di, di
mov cx, (setup_size + 1) >> 1
rep movsw  ; Copy setup from setup_from_seg to setup_to_seg.
jmp word setup_to_seg:0

times 0x1fe-($-$$) db 0

times 0x200-($-$$) db 0  ; Pad to sector boundary.
%ifndef MEMTEST_BIN
%fatal Run nasm -DMEMTEST_BIN="'...'"
%endif
incbin MEMTEST_BIN, 0x200, 0x200

times 0x10000-0x7c00-($-$$) db 0  ; To file offset 0x8400.
incbin MEMTEST_BIN, 0xa00
