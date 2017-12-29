; by pts@fazekas.hu at Mon Dec 25 04:22:48 CET 2017
;
; $ nasm -DLOAD_ADDR=0x8000 -f bin -o hiiimain.bin hiiimain.nasm
;
; Expected output in QEMU:
;
;   SeaBIOS (...)
;   Booting from Hard Disk...
;   Li~#
;   Message in the very beginning.
;
;   HI
;   9M_
;
; Cursor blinking at _ above.
;

bits 16
__org equ LOAD_ADDR
org __org

msg_pre:
db 13, 10, 'Message in the very beginning.' , 13, 10, 0

; Align to 12-byte boundary for bmcompress. Use 0x2c because
; liigboot_boot.nasm jumps here.
times 44-($-$$) db 'X'

%if 0  ; Short
; bmcompress SHORT_SIGNATURE1.
;db 0xeb, 0x22
;times 16 dw 0x5b53  ; This will be destroyed by compression.
;db 0xfa, 0xf4
%else
; !! Add automatic alignment.
; bmcompress LONG_SIGNATURE
db 0xeb, '?_SBARERP_COMPRESSION_WILL_BE_APPLIED_AFTER_THE_SLASH______LZMA/'
%endif

; At LOAD_ADDR + 0x50 (short) or LOAD_ADDR + 0x60 (long).
start:
mov bx, 7  ; Color for int 0x10 below. QEMU seems to be ignoring it.

pop ax  ; As pushed by liigboot_boot.nasm before the jump: '~'
mov ah, 0x0e
int 0x10

jmp near start2
times 0x123-($-$$) db 0
start2:
mov ax, 0x0e00 + '#'
int 0x10

mov si, msg_pre
call print

mov si, msg_hi
call print

mov ax, 0x900  ; segment of msg9
mov ds, ax
xor si, si  ; offset of msg9
;call print
lodsb  ; 13
lodsb  ; 10
lodsb  ; '9'
mov ah, 0x0e
int 0x10

%if 1
mov ax, 0x1200  ; segment of msg
mov ds, ax
xor si, si  ; offset of msg
;call print
lodsb  ; 13
lodsb  ; 10
lodsb  ; 'M'
mov ah, 0x0e
int 0x10
%endif

cli
hlt

; Print message at DS:SI.
print:
; Teletype output	AH=0Eh	AL = Character, BH = Page Number, BL = Color (only in graphic mode)
mov ah, 0x0e
.next:
lodsb
and al, al
jz .done
int 0x10
jmp short .next
.done:
ret

msg_hi db 13, 10, 'HI', 13, 10

times 0x9000-__org-($-$$) db 0

msg9 db 13, 10, '9ello, World!', 13, 10, 0

times 0x12000-__org-($-$$) db 0
msg db 13, 10, 'Mello, World!', 13, 10, 0

;incbin "random.bin"
