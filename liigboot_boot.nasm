; by pts@fazekas.hu at Fri Dec  8 02:00:46 CET 2017
; Load 2nd sector and give control to it.

bits 16

xor si, si
mov ds, si
mov es, si
mov di, 0x7e00

; AH	02h
; AL	Sectors To Read Count
; CH	Cylinder
; CL	Sector
; DH	Head
; DL	Drive
; ES:BX	Buffer Address Pointer
; mov ax, 0x201
; mov ax, 

; Teletype output	AH=0Eh	AL = Character, BH = Page Number, BL = Color (only in graphic mode)
mov ax, 0x0e41
xor bx, bx
int 0x10
hlt
