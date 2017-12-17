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
org 0x7c00  ; The BIOS loads the boot sector.


; 62 bytes of FAT16 filesystem headers (mkfs.vfat creates that many bytes in front of
; the boot code), including the BPB (BIOS parameter block).
; (FAT32 may need 90 bytes.) Copied from the beginning of ldlinuxmbr.bin .
entry1:
jmp strict short entry2  ; db 235,60
; These headers describe a FAT16 filesystem of 128 MiB in size, with 80
; reserved sectors, 1 FATs, with volume label 'prescue', volume UUID
; 33A8-16C5, 512 root directory entries, 2048 bytes per cluster, 512 bytes
; per sector. It was created by:
;
;   $ mkfs.vfat -f 1 -F 16 -R 80 -n prescue -i 33A816C5 -r 512 -s 4 -S 512 -C prescue.img 131072
;
db 144,83,89,83,76,73,78,85,88,0,2,4,80,0,1,0,2,0,0,248,0,1,32,0,64,0,0,0,0,0,0,0,4,0,128,0,41,197,22,168,51,112,114,101,115,99,117,101,32,32,32,32,70,65,84,49,54,32,32,32
; mkfs.vfat creates 52 bytes of headers
times 0x3e-($-$$) db 0  ; Doesn't add any additional bytes.

; Code starts at mem=0x7c58 disk=0x58=90.
entry2:
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
;   PartInfo.mbr == 0x7b78
;   PartInfo.gptlen == 0x7b88
;   PartInfo.gpt == 0x7b8c
; FloppyTable == 0x7bc4  (16 bytes)
; TODO(pts): Should we copy the partinfo? It doesn't seem to be needed.

mov es, ax
; No need to set CS, the `jmp 0:...' below will set it properly.
mov ds, ax

; ASSERT: sp == 0x7b60
;cmp sp, 0x7b60
;jne .spok

; Teletype output	AH=0Eh	AL = Character, BH = Page Number, BL = Color (only in graphic mode)
mov ax, 0x0e00 + 'M'
xor bx, bx
int 0x10

;.spok:

; Detect EBIOS (EDD, MBR) first.
mov bx, 0x55aa
mov ah, 0x41
int 0x13
jc  strict short no_ebios
cmp bx, 0xaa55
jne strict short no_ebios
test cl, 1
jz  strict short no_ebios

; https://en.wikipedia.org/wiki/INT_13H#INT_13h_AH.3D42h:_Extended_Read_Sectors_From_Drive
; Read ldlinux.sys in one go from disk.
mov ah, 0x42
; dl already contains the drive index.
mov si, dap
int 0x13
jnc strict short read_ok

read_error:
no_ebios:
mov ax, 0x0e00 + 'E'
xor bx, bx
int 0x10
hlt

read_ok:
mov ax, 0x0e00 + 'R'
xor bx, bx
int 0x10

; Register values copied from gdb3.reg.
; No need to reset es, all_read resets it.
; Also no need to set esi, all_read sets si immediately.
; Also no need to set edi, all_read sets di soon.
; Also no need to set edi, all_read sets cx soon.
; Also no need to set ebx, writestr_early called by all_read sets bx soon.
; Also no need to set eax, writestr_early called by all_read sets ax soon.
;mov ax, 0xf000
;mov fs, ax
;xor eax, eax
;mov gs, ax
;xor ecx, ecx
;mov ebx, 0x10e00
;mov edi, 0x7bc4
; TODO(pts) Do we need to set edx, ebp?
;mov edx, 0x1000
;mov ebp, 0x8c00

mov [0x80f5], al  ; Use LBA mode (getlinsec_ebios).
sti
jmp 0:0x8970  ; all_read, the entry point of ldlinux.sys in syslinux-4.07.

; We have 40 bytes free for new code, but we don't need it.
; The region 0xda ... 0xe0 is reserved for the modern standard MBR.
; We could add more code starting at file offset 0xe0.
times 0xda-($-$$) db 0

times 0x1b8-16-($-$$) db 0
; https://en.wikipedia.org/wiki/INT_13H#INT_13h_AH.3D42h:_Extended_Read_Sectors_From_Drive
; Disk Address Packet, for reading ldlinux.sys from disk. 16 bytes.
dap:
dw 0x10
dw 80 - 2  ; Number of sectors to load.
dw 0, 0x800  ; segment:offset of 0x8000
dq 2  ; Read from sector 2 (0x400), that's wher ldlinux.sys starts.

; UUIDs and the parition table of a modern standard Master Boot Record (MBR).
; https://en.wikipedia.org/wiki/Master_boot_record
times 0x1b8-($-$$) db 0
dd 0x37574cf8  ; Disk signature (UUID)
dw 0
;dq 0, 0  ; Partition entry 1. Empty.
; Partition entry 1. 1 MiB long, starting at 128 MiB (right after prescue).
; Bootable bit unset.
;db 0, 0x51, 2, 0x10, 0x83, 0x71, 0x21, 0x10, 0, 0, 4, 0, 0, 8, 0, 0
; Partition entry 1. 1 MiB long, starting at 64 MiB (right after prescue).
; Bootable bit unset.
db 0, 0x51, 2, 0x10, 0x83, 0x71, 0x21, 0x10, 0, 0, 4, 0, 0, 0, 2, 0
dq 0, 0  ; Partition entry 2. Empty.
dq 0, 0  ; Partition entry 3. Empty.
dq 0, 0  ; Partition entry 4. Empty.
dw 0xaa55  ; Boot signature.

times 0x200-($-$$) db 0  ; Doesn't add any additional bytes.

; __END__
