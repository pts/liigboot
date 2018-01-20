;
; grub4dos_fd0_tab_patch.nasm: `cat (fd0', <Tab> binary patch for GRUB4DOS
; by pts@fazekas.hu at Sat Jan 20 12:12:40 CET 2018
;
; After this binary patch, `cat (fd0', <Tab> in GRUB4DOS will autocomplete
; to ',' (and show each partition) rather than ')'.
;
; This file is not used when building Liigboot.
; grub4dos_fd0_tab_patch.new.bin has been incorporated to patch_grldr.py
; (search for 0x1b78e).
;
; $ nasm -O0          -f bin -o grub4dos_fd0_tab_patch.old.bin grub4dos_fd0_tab_patch.nasm
; $ nasm -O0 -DIS_NEW -f bin -o grub4dos_fd0_tab_patch.new.bin grub4dos_fd0_tab_patch.nasm
;

bits 32
ss_grub_strcpy equ 0x1874c
ss_grub_strlen equ 0x18520
ss_unique equ 0x3be98
ss_unique_string equ 0x3be9c
after_code1 equ 0x1b825
after_code2 equ 0x1b96e
our_org equ 0x1b78e

; --- grub4dos-0.4.4-src/stage2/disk_io.c.orig	2018-01-16 23:42:20.813893042 +0100
; +++ grub4dos-0.4.4-src/stage2/disk_io.c	2018-01-20 11:36:00.415007915 +0100
; @@ -1623,16 +1620,16 @@
;  		  ptr++;
;  		  grub_strcpy (ptr, unique_string);
;  		  if (unique == 1)
;  		    {
;  		      ptr += grub_strlen (ptr);
; -		      if (*unique_string == 'h')
; +		      if (*unique_string == 'f' || *unique_string == 'h')
;  			{
;  			  *ptr++ = ',';
;  			  *ptr = 0;
;  			}
;  		      else
;  			{
;  			  *ptr++ = ')';
;  			  *ptr = 0;
;  			}
;  		    }
;
; * On input, eax contains the (unincremented) ptr.
; * On input, ecx contains the unique_string local variable.
; * On output, registers eax, ebx, ecx can be destroyed.
; * On output, edx should contain the final value of `ptr - 1'.
; * On output, execution must continue at after_code1.
; * We don't have to execute `*ptr = 0;', after_code1 does it.
; * There are no incoming jumps to this code.
;
; We are able to gain a few bytes because in the original code gcc has
; aligned function call arguments to 16-byte boundary.
;

org our_org
%ifndef IS_NEW  ; 2 useless bytes.
push ecx  ; Why? Could be removed.
push ecx  ; Why? Could be removed.
%endif
push edx  ; unique_string.
lea ebx, [eax+1]
push ebx  ; ptr.
call dword ss_grub_strcpy
%ifdef IS_NEW  ; 2 bytes.
pop ecx
pop ecx
%else  ; 3 bytes.
add esp, byte +0x10
%endif
cmp dword [dword ss_unique], byte 1
jnz strict after_code2
%ifndef IS_NEW  ; 3 useless bytes.
sub esp, byte 0xc
%endif
push ebx  ; ptr.
call dword ss_grub_strlen
%ifdef IS_NEW  ; 1 byte.
pop ecx
%else  ; 3 bytes.
add esp, byte 0x10
%endif
lea edx, [ebx+eax]
mov eax, [ss_unique_string]
%ifdef IS_NEW
mov byte [edx], ')'
cmp byte [eax], 'f'
je short set_to_comma
cmp byte [eax], 'h'
jne short after_code1
set_to_comma:
mov byte [edx], ','
%else
cmp byte [eax], 'h'
jnz short set_to_paren
mov byte [edx], ','
jmp short after_code1
set_to_paren:
mov byte [edx], ')'
%endif
jmp short after_code1
; Pad to original size. IS_NEW gains 5 bytes.
times (0x1b7cd - our_org) - ($ - $$) db 0x90
