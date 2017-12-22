;
; liigboot_boot.nasm: MBR bootloader to load ldlinux.sys and give control to it.
; by pts@fazekas.hu at Wed Dec 20 01:33:02 CET 2017
;
; To compile just the boot sectors:
;
;   $ nasm -f bin -o liigresc_bs.bin -DLIIGRESC liigboot_boot.nasm
;   $ nasm -f bin -o liigboot_bs.bin -DLIIGBOOT liigboot_boot.nasm
;
; To compile the entire FAT filesystem with ldlinux.sys in the reserved
; sectores:
;
;   $ nasm -f bin -o liigresc_empty.img -DLIIGRESC -DEMPTYFS liigboot_boot.nasm
;   $ nasm -f bin -o liigboot_empty.img -DLIIGBOOT -DEMPTYFS liigboot_boot.nasm
;
; When compiled without the -DEMPTYFS flag, the output file will be an 512-byte
; boot sector containing:
;
; * a BIOS MBR partition table with 1 entry
; * bootloader code to load ldlinux.sys from Syslinux 4.06, 39 KiB starting
;   at sector 2 (byte offset 1024)
; * filesystem headers of a FAT12 or FAT16 filesystem (BIOS parameter block)
;
; When compiled with the -DEMPTYFS flag, the output file will be longer (1
; MiB or 128 MiB), and it will contain a bootable FAT12 or FAT16 filesystem
; image with no files on it, and the ldlinux.sys code in its reserved
; sectors (39 KiB starting at byte offset 1024).
;
; TODO(pts): Make it bootable from a partition.
;
; Docs:
;
; * http://wiki.osdev.org/MBR_(x86)
; * http://wiki.osdev.org/Boot_Sequence

bits 16
org 0x7c00  ; The BIOS loads the boot sector.

; 62 bytes of FAT16 filesystem headers (mkfs.vfat creates that many bytes in front of
; the boot code), including the BPB (BIOS parameter block).
; (FAT32 may need 90 bytes.) Copied from the beginning of ldlinuxmbr.bin .
entry1:
jmp strict short entry2  ; db 235,60
nop  ; db 144

%ifdef LIIGRESC
%ifdef LIIGBOOT
%error Do not specify both -DLIIGRESC and -DLIIGBOOT.
%endif
%define VOLUME_LABEL 'liigresc   '
%else
%ifndef LIIGBOOT
%error Specify at least -DLIIGRESC and -DLIIGBOOT.
%endif
%define VOLUME_LABEL 'liigboot   '
%endif

; FAT filesystem headers (BPB, BIOS Parameter Block).

db 'SYSLINUX'  ; OEM ID.

%ifdef LIIGRESC
; These headers describe a FAT16 filesystem of 128 MiB in size, with 80
; reserved sectors, 1 FATs, with volume label 'prescue', volume UUID
; 33A8-16C5, 512 root directory entries, 2048 bytes per cluster, 512 bytes
; per sector. It was created with:
;
;   $ mkfs.vfat -f 1 -F 16 -R 80 -n prescue   -i 33A816C5 -r 512 -s 4 -S 512 -C prescue.img 131072
;
dw 512   ; Sector size in bytes.
db 4     ; Sectors per cluster.
dw 80    ; Number of reserved sectors.
db 1     ; Number of FATs.
dw 512   ; Number of root directory entries.
dw 0     ; Number of sectors or 0.
db 248   ; Media descriptor.
dw 256   ; Sectors per FAT.
dw 32    ; Sectors per track (CHS).
dw 64    ; Heads (CHS).
dd 0     ; Hidden.
dd 0x40000 ; Number of sectors if the field above is 0.
dw 0x80  ; Physical drive number.
db 0x29  ; B4 BPB signature.
dd 0x33A816C5    ; UUID (serial number).
db VOLUME_LABEL  ; 11 bytes.
db 'FAT16   '    ; Filesytem type.
%endif

%ifdef LIIGBOOT
; These headers describe a FAT12 filesystem of 1 MiB in size, with 80
; reserved sectors, 1 FATs, with volume label 'prescue1m', volume UUID
; 33A8-16C6, 32 root directory entries, 512 bytes per cluster, 512 bytes
; per sector. It was created by:
;
;   $ mkfs.vfat -f 1 -F 12 -R 80 -n prescue1m -i 33A816C6 -r  32 -s 1 -S 512 -C prescue1m.img 1024
;
dw 512   ; Sector size in bytes.
db 1     ; Sectors per cluster.
dw 80    ; Number of reserved sectors.
db 1     ; Number of FATs.
dw 32    ; Number of root directory entries.
dw 2048  ; Number of sectors or 0.
db 248   ; Media descriptor.
dw 6     ; Sectors per FAT.
dw 32    ; Sectors per track (CHS).
dw 64    ; Heads (CHS).
dd 0     ; Hidden.
dd 0     ; Number of sectors if the field above is 0.
dw 0x80  ; Physical drive number.
db 0x29  ; B4 BPB signature.
dd 0x33A816C6    ; UUID (serial number).
db VOLUME_LABEL  ; 11 bytes.
db 'FAT12   '    ; Filesytem type.
%endif

; mkfs.vfat creates 52 bytes of headers for FAT12 and FAT16 filesystems.
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

; TODO(pts): Do these addresses change if we relink ldlinux.sys?
mov [0x80f5], al  ; Use LBA mode (getlinsec_ebios).
sti
jmp 0:0x8970  ; all_read, the entry point of syslinux_liigboot.ldlinux.sys (Syslinux 4.07).

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
dq 2  ; Read from sector 2 (0x400), that's where ldlinux.sys starts.

; UUIDs and the parition table of a modern standard Master Boot Record (MBR).
; https://en.wikipedia.org/wiki/Master_boot_record
times 0x1b8-($-$$) db 0
dd 0x37574cf8  ; Disk signature (UUID)
dw 0
;dq 0, 0  ; Partition entry 1. Empty.

%ifdef LIIGRESC
; Partition entry 1. 1 MiB long, starting at 128 MiB (right after prescue).
; Bootable bit unset. CHS values are garbage.
db 0, 0x51, 2, 0x10, 0x83, 0x71, 0x21, 0x10
dd 128 * 1024 * 1024 / 512  ; LBA start offset.
dd 1024 * 1024 / 512  ; LBA size.
%endif

%ifdef LIIGBOOT
; Partition entry 1. 1 MiB long, starting at 1 MiB (right after prescue).
; Bootable bit unset. CHS values are garbage.
db 0, 0x51, 2, 0x10, 0x83, 0x71, 0x21, 0x10
dd 1024 * 1024 / 512  ; LBA start offset.
dd 1024 * 1024 / 512  ; LBA size.
%endif

dq 0, 0  ; Partition entry 2. Empty.
dq 0, 0  ; Partition entry 3. Empty.
dq 0, 0  ; Partition entry 4. Empty.
dw 0xaa55  ; Boot signature.

times 0x200-($-$$) db 0  ; Doesn't add any additional bytes.

%ifdef EMPTYFS

times 0x400-($-$$) db 0
; $ cmp -l syslinux_liigboot.ldlinux.sys syslinux-4.07.ldlinux.sys
; at 0x8028: CheckSum dd ...
; at 0x81ee: SectorPtrs dq 3; dw 70  ; Instructs the loader to load 70 * 512 bytes from 0x600 on disk to 0x8600 in memory. ldlinux.sys is 71 * 512 bytes from 0x400 on disk.
; at 0xb798: ADVSec0 dq 74 (sector number relative the partition, += Hidden) 
; at 0xb7a0: ADVSec1 dq 75 (ADVSec0 + 1)
incbin "syslinux_liigboot.ldlinux.sys"
times 0xa000-($-$$) db 0

%ifdef LIIGRESC
dw 0xfff8, 0xffff  ; Empty FAT16 FAT.
times 0x2a000-($-$$) db 0
db VOLUME_LABEL
; Rest of the directory entry for the volume label.
db 8, 0, 0, 0x41, 9, 0x94, 0x4b, 0x94, 0x4b, 0, 0, 0x51, 9, 0x94, 0x4b, 0, 0, 0, 0, 0, 0
times 0x8000000-($-$$) db 0  ; This is slow (it takes >10 seconds to compile.)
%endif

%ifdef LIIGBOOT
db 0xf8, 0xff, 0xff  ; Empty FAT12 FAT.

times 0xac00-($-$$) db 0
db VOLUME_LABEL
; Rest of the directory entry for the volume label.
db 8, 0, 0, 0x41, 4, 0x90, 0x4b, 0x90, 0x4b, 0, 0, 0x41, 4, 0x90, 0x4b, 0, 0, 0, 0, 0, 0
times 0x100000-($-$$) db 0
%endif

%endif  ; EMPTYFS

; __END__
