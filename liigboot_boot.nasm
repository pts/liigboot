; by pts@fazekas.hu at Fri Dec  8 02:00:46 CET 2017
; Load 2nd sector and give control to it.
;
; $ nasm -f bin boot2.nasm
; $ sudo dd if=boot2 of=/dev/sdc bs=438 count=1
; !! 440 -- what about the parititon ID?
;
; http://wiki.osdev.org/MBR_(x86)
; http://wiki.osdev.org/Boot_Sequence

; !! Add jump forward.

bits 16
; Actually loaded at 0x7c00, but relocated later.
org 0x7e00

push ds
push es
pusha
xor si, si
mov ds, si
mov es, si
mov si, 0x7c00
mov di, 0x7e00
mov cx, 256
rep movsw
jmp 0:printa


printa:
; Teletype output	AH=0Eh	AL = Character, BH = Page Number, BL = Color (only in graphic mode)
mov ax, 0x0e00 + 'A'
xor bx, bx
int 0x10

mov di, 0x7c00
xor ax, ax
mov cx, 256
rep stosw  ; Clear original.

; Now load the same boot sector again.

; https://en.wikipedia.org/wiki/INT_13H#INT_13h_AH.3D02h:_Read_Sectors_From_Drive
; AH	02h
; AL	Sectors To Read Count
; CH	Cylinder
; CL	Sector
; DH	Head
; DL	Drive
; ES:BX	Buffer Address Pointer

mov ax, 0x201
xor cx, cx
mov dh, 0
; Keep DL as the original drive.
mov bx, 0x7c00
inc cx  ; Read sector 1 (bytes 0..511), the MBR again.
inc cx  ; Read sector 2 (bytes 512..1023), the copied syslinux boot sector.
int 0x13  ; Read sector(s) from disk.
jc readerr  ; Print error message on read error.

;xor di, di
popa
; !! Why do we need to pop all for syslinux?
;    It expects DS:SI as partinfo, ES:DI as $PnP.
; DS:SI -> partinfo
pop es
pop ds
;xor si, si  ; partinfo
jmp 0:0x7c00

readerr:
mov ax, 0x0e00 + 'E'
xor bx, bx
int 0x10
hlt
