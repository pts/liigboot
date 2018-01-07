; -----------------------------------------------------------------------
;
;   Copyright 1994-2009 H. Peter Anvin - All Rights Reserved
;   Copyright 2009-2011 Intel Corporation; author: H. Peter Anvin
;
;   This program is free software; you can redistribute it and/or modify
;   it under the terms of the GNU General Public License as published by
;   the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
;   Boston MA 02110-1301, USA; either version 2 of the License, or
;   (at your option) any later version; incorporated herein by reference.
;
; -----------------------------------------------------------------------

;
; diskstart.inc
;
; Common early-bootstrap code for harddisk-based Syslinux derivatives.
;

bootsec equ 0x7c00
superblock equ 0x7c0b
superblock_len_fat16 equ 0x33
superinfo_size equ 0x10
MaxLMA equ (96<<10) + 0x8000  ; 0x20000

; !! kaboom.patch -- why is it better to clear vga? why not immediately properly
;;;;

		section .init

; All these symbols have the value 0x7c00: STACK_TOP, TEXT_START, __init_start,
; __stack16_end, bootsec. This is the address where the BIOS loads the MBR
; boot sector (i.e. LBA sector 0 of the drive).
StackBuf	equ STACK_TOP-44-92	; Start the stack here (grow down - 4K)
PartInfo	equ StackBuf
.mbr		equ PartInfo
.gptlen		equ PartInfo+16
.gpt		equ PartInfo+20
FloppyTable	equ PartInfo+76
; Total size of PartInfo + FloppyTable == 76+16 = 92 bytes
Hidden		equ StackBuf-24		; Partition offset (qword)
OrigFDCTabPtr	equ StackBuf-16		; Original FDC table
OrigDSSI	equ StackBuf-12		; DS:SI -> partinfo
OrigESDI	equ StackBuf-8		; ES:DI -> $PnP structure
DriveNumber	equ StackBuf-4		; Drive number
StackHome	equ Hidden		; The start of the canonical stack

%ifndef LOAD_ADDR
%fatal Run nasm -DLOAD_ADDR=0x...
%endif
		zb LOAD_ADDR-0x7c00-($-$$)
load_addr:  ; 0x8000, -DLOAD_ADDR=0x8000

; ---
;
; At most 0x2c bytes of headers. Will be retained by both the compressed and
; the uncompressed variant.

; This must start at the beginning.
ADVSec0		dd 0			; Not specified
ADVSec1		dd 0			; Not specified
MaxTransfer	dw 127		; Max sectors to transfer
		dw 0

syslinux_banner db 13, 10
late_banner	db 13, 'Liigboot ', DATE_STR, 0
		zb 0x23-($-load_addr)

		zb 0x2c-($-load_addr)

; ---

_start:  ; 0x802c
		; The liigmain.bin upxbc --flat16 compression signature,
		; Harmless code (jumps over itself).
		db 0xeb, '"SBAFEUU_COMPRESSION_AFTER_SLASH__/'

		jmp near all_read

; !! Move the rest of .init to .text16 below.

;
;
; writestr_early: write a null-terminated string to the console
;	    This assumes we're on page 0.  This is only used for early
;           messages, so it should be OK.
;
writestr_early:
		pushad
.loop:		lodsb
		and al,al
                jz .return
		mov ah,0Eh		; Write to screen as TTY
		mov bx,0007h		; Attribute  !! --- why not 0?
		int 10h
		jmp short .loop
.return:	popad
		ret

;
; getlinsec: load a sequence of BP floppy sector given by the linear sector
;	     number in EAX into the buffer at ES:BX.  We try to optimize
;	     by loading up to a whole track at a time, but the user
;	     is responsible for not crossing a 64K boundary.
;	     (Yes, BP is weird for a count, but it was available...)
;
;	     On return, BX points to the first byte after the transferred
;	     block.
;
;            This routine assumes CS == DS.
;
		global getlinsec
getlinsec:
		pushad
		add eax,[Hidden]		; Add partition offset
		adc edx,[Hidden+4]
; getlinsec implementation for EBIOS (EDD)
getlinsec_ebios:
.loop:
                push bp                         ; Sectors left
.retry2:
		cmp bp,[MaxTransfer]		; Enforce maximum transfer size
		jna .maxtransok
		mov bp,[MaxTransfer]
.maxtransok:
		movzx edi,bp			; Sectors we are about to read
		mov cx,retry_count
.retry:

		; Form DAPA on stack
		push edx
		push eax
		push es
		push bx
		push di
		push word 16
		mov si,sp
		pushad
                mov ah,42h                      ; Extended Read
		push ds
		push ss
		pop ds
		mov dl,[DriveNumber]
		push es		; ES destroyed by INT 13h AH 08h
		int 13h
		pop es
		pop ds
		popad
		lea sp,[si+16]			; Remove DAPA
		jc short kaboom
		pop bp
		add eax,edi			; Advance sector pointer
		adc edx,0
		sub bp,di			; Sectors left
		shl di, 9  ; imul di, [bsBytesPerSec]  ; 512
                add bx,di			; Advance buffer pointer
                and bp,bp
                jnz .loop

		popad
                ret

;
; kaboom: write a message (bailmsg) and bail out.
;
		global kaboom
kaboom:
		xor si,si
		mov ss,si
		mov sp,OrigFDCTabPtr	; Reset stack
		mov ds,si		; Reset data segment
		pop dword [fdctab]	; Restore FDC table
.patch:					; When we have full code, intercept here
		mov si,bailmsg
.loop:		lodsb  ; !! reuse writestr_early
		and al,al
                jz .done
		mov ah,0Eh		; Write to screen as TTY
		mov bx,0007h		; Attribute !! why not 0?
		int 10h
		jmp short .loop

.done:
		xor ax,ax
.again:		int 16h			; Wait for keypress
					; NB: replaced by int 18h if
					; chosen at install time..
		int 19h			; And try once more to boot...
.norge:		hlt			; If int 19h returned; this is the end
		jmp short .norge

bailmsg:	db 'Boot error', 0Dh, 0Ah
global SubvolName  ; Needed by fs/btrfs/btrfs.c .
SubvolName      db 0

		section .text16
all_read:
		; We enter here with ES scrambled...
		xor ax,ax
		mov es,ax
;
; Let the user (and programmer!) know we got this far.  This used to be
; in Sector 1, but makes a lot more sense here.
;
		mov si,late_banner
		call writestr_early

		mov si,copyright_str
		call writestr_early


;
; Insane hack to expand the DOS superblock to dwords
;
expand_super:
		xor eax,eax
		mov si,superblock
		mov di,SuperInfo
		mov cx,superinfo_size
.loop:
		lodsw
		dec si
		stosd				; Store expanded word
		xor ah,ah
		stosd				; Store expanded byte
		loop .loop


;
; Common initialization code
;
%include "init.inc"
		
		pushad
		mov eax,ROOT_FS_OPS
		movzx dx,byte [DriveNumber]
		; DH = 0: we are boot from disk not CDROM
		mov ecx,[Hidden]
		mov ebx,[Hidden+4]
		mov si, 255  ; Fake value for [bsHeads], unused with LBA, EDD (EBIOS).
		mov di, 63   ; Fake value for [bsSecPerTrack], unused with LBA, EDD (EBIOS).
		movzx ebp,word [MaxTransfer]
		pm_call fs_init
		popad

		section .bss16
SuperInfo	resq 16			; The first 16 bytes expanded 8 times

;
; Banner information not needed in sector 1
;		section .data16

		section .text16
