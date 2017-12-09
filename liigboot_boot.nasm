;
; by pts@fazekas.hu at Sat Dec  9 15:48:02 CET 2017
; Load ldlinux.sys and give control to it. Works.
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
entry2:
jmp 0:0x7c58 + 5  ; 5 bytes for this jump instruction.
cli
cld
mov esp, 0x7b76
xor eax, eax
mov ss, ax

; We have to push 12 items.
push dx  ; Save drive number, goes to 0x7b74.
push es  ; Save ES:DI ($PnP), goes to 0x7b72.
push di  ; Goes to 0x7b70.
push ds  ; Save DS:SI (partinfo), goes to 0x7b6e.
push si  ; Goes to 0x7b6c.
push ax  ; Just copy 0. Part of OrigFDCTabPtr, goes to 0x7b6a.
push ax  ; Just copy 0. Part of OrigFDCTabPtr, goes to 0x7b68.
push ax  ; Should be 0. Part of Hidden, goes to 0x7b66.
push ax  ; Should be 0. Part of Hidden, goes to 0x7b64.
push ax  ; Should be 0. Part of Hidden, goes to 0x7b62.
push ax  ; Should be 0. Part of Hidden, goes to 0x7b60.

; STACK_TOP == 0x7c00
; StackBuf == 0x7b78
; Hidden == 0x7b60  (dq)
; OrigFDCTabPtr == 0x7b68 (dd)
; OrigDSSI == partinfo == 0x7b6c (dd)
; OrigESDI == $PNP == 0x7b70 (dd)
; DriveNumber == 0x7b74 (dw)
; PartInfo == 0x7b78  (76 bytes)
; PartInfo.mbr == 0x7b78
; PartInfo.gptlen == 0x7b88
; PartInfo.gpt == 0x7b8c
; FloppyTable == 0x7bc4  (16 bytes)
; !! Should we copy the partinfo?

mov es, ax
;mov cs, ax  ; The jmp above did the trick.
mov ds, ax

; ASSERT: sp == 0x7b60
;cmp sp, 0x7b60
;jne .spok

; Teletype output	AH=0Eh	AL = Character, BH = Page Number, BL = Color (only in graphic mode)
mov ax, 0x0e00 + 'B'
xor bx, bx
int 0x10

;.spok:

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
