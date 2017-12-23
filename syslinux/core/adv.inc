;; -----------------------------------------------------------------------
;;
;;   Copyright 2007-2008 H. Peter Anvin - All Rights Reserved
;;
;;   This program is free software; you can redistribute it and/or modify
;;   it under the terms of the GNU General Public License as published by
;;   the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
;;   Boston MA 02110-1301, USA; either version 2 of the License, or
;;   (at your option) any later version; incorporated herein by reference.
;;
;; -----------------------------------------------------------------------

;;
;; adv.inc
;;
;; The auxillary data vector and its routines
;;
;; The auxillary data vector is a 512-byte aligned block that on the
;; disk-based derivatives can be part of the syslinux file itself.  It
;; exists in two copies; when written, both copies are written (with a
;; sync in between, if from the operating system.)  The first two
;; dwords are magic number and inverse checksum, then follows the data
;; area as a tagged array similar to BOOTP/DHCP, finally a tail
;; signature.
;;
;; Note that unlike BOOTP/DHCP, zero terminates the chain, and FF
;; has no special meaning.
;;

;;
;; List of ADV tags...
;;
ADV_BOOTONCE	equ 1

;;
;; Other ADV data...
;;
ADV_MAGIC1	equ 0x5a2d2fa5			; Head signature
ADV_MAGIC2	equ 0xa3041767			; Total checksum
ADV_MAGIC3	equ 0xdd28bf64			; Tail signature

ADV_LEN		equ 500				; Data bytes

adv_retries	equ 6				; Disk retries

		section .adv
		; Introduce the ADVs to valid but blank
adv0:
.head		resd 1
.csum		resd 1
.data		resb ADV_LEN
.tail		resd 1
.end		equ $
adv1:
.head		resd 1
.csum		resd 1
.data		resb ADV_LEN
.tail		resd 1
.end		equ $
		section .text16

		;
		; This is called after config file parsing, so we know
		; the intended location of the ADV
		;
adv_init:
		;cmp byte [ADVDrive],-1
		;jne adv_read

%if IS_SYSLINUX || IS_EXTLINUX
		mov edx, [ADVSec0]
		test edx, edx
		jz adv_verify.reset  ; No ADV if ADVSec0 is 0.
		mov eax,[Hidden]  ; ignore high 4 bytes of Hidden
		add [ADVSec0],eax
		add [ADVSec1],eax
		mov al,[DriveNumber]
		mov [ADVDrive],al
		jmp adv_read
%endif

		;
		; Initialize the ADV data structure in memory
		;
adv_verify:
		cmp byte [ADVDrive],-1		; No ADV configured, still?
		je .reset			; Then unconditionally reset

		mov si,adv0
		call .check_adv
		jz .ok				; Primary ADV okay
		mov si,adv1
		call .check_adv
		jz .adv1ok

		; Neither ADV is usable; initialize to blank
.reset:
		mov di,adv0
		mov eax,ADV_MAGIC1
		stosd
		mov eax,ADV_MAGIC2
		stosd
		xor eax,eax
		mov cx,ADV_LEN/4
		rep stosd
		mov eax,ADV_MAGIC3
		stosd

.ok:
		ret

		; The primary ADV is bad, but the backup is OK
.adv1ok:
		mov di,adv0
		mov cx,512/4
		rep movsd
		ret


		; SI points to the putative ADV; unchanged by routine
		; ZF=1 on return if good
.check_adv:
		push si
		lodsd
		cmp eax,ADV_MAGIC1
		jne .done			; ZF=0, i.e. bad
		xor edx,edx
		mov cx,ADV_LEN/4+1		; Remaining dwords
.csum:
		lodsd
		add edx,eax
		loop .csum
		cmp edx,ADV_MAGIC2
		jne .done
		lodsd
		cmp eax,ADV_MAGIC3
.done:
		pop si
		ret

;
; adv_get: find an ADV string if present
;
; Input:	DL	= ADV ID
; Output:	CX	= byte count (zero on not found)
;		SI	= pointer to data
;		DL	= unchanged
;
; Assumes CS == DS.
;

adv_get:
		push ax
		mov si,adv0.data
		xor ax,ax			; Keep AH=0 at all times
.loop:
		lodsb				; Read ID
		cmp al,dl
		je .found
		and al,al
		jz .end
		lodsb				; Read length
		add si,ax
		cmp si,adv0.tail
		jb .loop
		jmp .end

.found:
		lodsb
		mov cx,ax
		add ax,si			; Make sure it fits
		cmp ax,adv0.tail
		jbe .ok
.end:
		xor cx,cx
.ok:
		pop ax
		ret

;
; adv_set: insert a string into the ADV in memory
;
; Input:	DL	= ADV ID
;		FS:BX	= input buffer
;		CX	= byte count (max = 255!)
; Output:	CF=1 on error
;		CX	clobbered
;
; Assumes CS == DS == ES.
;
adv_set:
		push ax
		push si
		push di
		and ch,ch
		jnz .overflow

		push cx
		mov si,adv0.data
		xor ax,ax
.loop:
		lodsb
		cmp al,dl
		je .found
		and al,al
		jz .endz
		lodsb
		add si,ax
		cmp si,adv0.tail
		jb .loop
		jmp .end

.found:		; Found, need to delete old copy
		lodsb
		lea di,[si-2]
		push di
		add si,ax
		mov cx,adv0.tail
		sub cx,si
		jb .nukeit
		rep movsb			; Remove the old one
		mov [di],ah			; Termination zero
		pop si
		jmp .loop
.nukeit:
		pop si
		jmp .end
.endz:
		dec si
.end:
		; Now SI points to where we want to put our data
		pop cx
		mov di,si
		jcxz .empty
		add si,cx
		cmp si,adv0.tail-2
		jae .overflow			; CF=0

		mov si,bx
		mov al,dl
		stosb
		mov al,cl
		stosb
		fs rep movsb

.empty:
		mov cx,adv0.tail
		sub cx,di
		xor ax,ax
		rep stosb			; Zero-fill remainder

		clc
.done:
		pop di
		pop si
		pop ax
		ret
.overflow:
		stc
		jmp .done

;
; adv_cleanup:	checksum adv0 and copy to adv1
;		Assumes CS == DS == ES.
;
adv_cleanup:
		pushad
		mov si,adv0.data
		mov cx,ADV_LEN/4
		xor edx,edx
.loop:
		lodsd
		add edx,eax
		loop .loop
		mov eax,ADV_MAGIC2
		sub eax,edx
		lea di,[si+4]			; adv1
		mov si,adv0
		mov [si+4],eax			; Store checksum
		mov cx,(ADV_LEN+12)/4
		rep movsd
		popad
		ret

;
; adv_write:	write the ADV to disk.
;
;		Location is in memory variables.
;		Assumes CS == DS == ES.
;
;		Returns CF=1 if the ADV cannot be written.
;
adv_write:
		push eax
		mov eax,[ADVSec0]
		or eax,[ADVSec0+4]
		je .bad
		mov eax,[ADVSec1]
		or eax,[ADVSec1+4]
		je .bad
		cmp byte [ADVDrive],-1
		je .bad

		call adv_cleanup
		mov ah,3			; Write
		call adv_read_write

		clc
		pop eax
		ret
.bad:						; No location for ADV set
		stc
		pop eax
		ret

;
; adv_read:	read the ADV from disk
;
;		Location is in memory variables.
;		Assumes CS == DS == ES.
;
adv_read:
		push ax
		mov ah,2			; Read
		call adv_read_write
		call adv_verify
		pop ax
		ret

;
; adv_read_write: disk I/O for the ADV
;
;		On input, AH=2 for read, AH=3 for write.
;		Assumes CS == DS == ES.
;
adv_read_write:
		mov [ADVOp],ah
		pushad

		mov si,.ebios
		mov eax,[ADVSec0]
		mov edx,[ADVSec0+4]
		mov bx,adv0
		call .doone

		mov eax,[ADVSec1]
		mov edx,[ADVSec1+4]
		mov bx,adv1
		call .doone

		popad
		ret

.doone:
		push si
		jmp si

.ebios:
		mov cx,adv_retries
.eb_retry:
		; Form DAPA on stack
		push edx
		push eax
		push es
		push bx
		push word 1			; Sector count
		push word 16			; DAPA size
		mov si,sp
		pushad
		mov dl,[ADVDrive]
		mov ax,4000h
		or ah,[ADVOp]
		push ds
		push ss
		pop ds
		int 13h
		pop ds
		popad
		lea sp,[si+16]			; Remove DAPA
		jc .eb_error
		pop si
		ret
.eb_error:
		loop .eb_retry
		stc
		pop si
		ret

;		section .data16
ADVDrive	db -1			; No ADV defined

		section .bss16
ADVOp		resb 1

		section .text16
