; by pts@fazekas.hu at Sat Dec  9 15:48:02 CET 2017
; Load ldlinux.sys and give control to it.
;
; $ nasm -f bin -o boot3.bin boot3.nasm
; $ dd if=boot3.bin of=prescue.img bs=1k conv=notrunc
;
; http://wiki.osdev.org/MBR_(x86)
; http://wiki.osdev.org/Boot_Sequence

bits 16
org 0x7c00  ; The BIOS loads the boot sector 

; 90 bytes of FAT16 headers, including the BPB (BIOS parameter block).
; Copied from the beginning of ldlinuxmbr.bin
db 235,88,144,83,89,83,76,73,78,85,88,0,2,4,80,0,1,0,2,0,0,248,0,1,32,0,64,0,0,0,0,0,0,0,4,0,128,0,41,197,22,168,51,112,114,101,115,99,117,101,32,32,32,32,70,65,84,49,54,32,32,32,14,31,190,91,124,172,34,192,116,11,86,180,14,187,7,0,205,16,94,235,240,50,228,205,22,205,25,235

; Code starts at mem=0x7c58 disk=0x58=90.
jmp 0:0x7c58 + 5  ; 5 bytes for this jump instruction.
cli
cld
xor eax, eax
mov es, ax
;mov cs, ax  ; The jmp above did the trick.
mov ds, ax
mov ss, ax
mov esp, 0x7b60

; Teletype output	AH=0Eh	AL = Character, BH = Page Number, BL = Color (only in graphic mode)
mov ax, 0x0e00 + 'B'
xor bx, bx
int 0x10

; !! Detect EBIOS first.

; https://en.wikipedia.org/wiki/INT_13H#INT_13h_AH.3D42h:_Extended_Read_Sectors_From_Drive
mov ah, 0x42
; dl already contains the drive index.
mov si, dap
int 0x13
jnc strict short read_ok

; Teletype output: report read error.
mov ax, 0x0e00 + 'E'
xor bx, bx
int 0x10
hlt

read_ok:
mov ax, 0x0e00 + 'O'
xor bx, bx
int 0x10
; hlt

; Register values copied from gdb3.reg.
; !! TODO(pts): Where do we need cli?
; !! TODO(pts): Do we need all these registers?
; !! TODO(pts): Where did we save the drive (0x80) to?
;cli
mov ax, 0x820
mov es, ax
mov ax, 0xf000
mov fs, ax
xor eax, eax
mov gs, ax
xor ecx, ecx
mov edx, 0x1000
mov ebx, 0x10e00
mov ebp, 0x8c00
mov esi, 0xe00
mov edi, 0x7bc4

mov [0x80f5], al  ; Use LBA mode (getlinsec_ebios).
sti  ; !! When? Earlier?
jmp 0:0x8970  ; Entry point of ldlinux.sys in syslinux-4.07.

; https://en.wikipedia.org/wiki/INT_13H#INT_13h_AH.3D42h:_Extended_Read_Sectors_From_Drive
; Disk Address Packet, for reading ldlinux.sys from disk.
dap:
dw 0x10
dw 80 - 2
dw 0, 0x800  ; segment:offset of 0x8000
dq 2  ; Read from sector 2 (0x400), that's wher ldlinux.sys starts.
