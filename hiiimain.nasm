; by pts@fazekas.hu at Mon Dec 25 04:22:48 CET 2017
; 
; $ nasm -DLOAD_ADDR=0x8000 -f bin -o hiiimain.bin hiiimain.nasm
;

bits 16
__org equ LOAD_ADDR
org __org

msg_pre:
db 13, 10, 'Message in the very beginning.' , 13, 10, 0
times 44-($-$$) db 'X'
db 0xeb, 0x22
times 16 dw 0x5b53  ; This will be destroyed by compression.
db 0xfa, 0xf4

; At LOAD_ADDR + 0x50.
start: 

pop ax  ; !! as pushed by liigboot_boot.nasm before the jump
mov ah, 0x0e
xor bx, bx
int 0x10

jmp near start2
times 0x123-($-$$) db 0
start2:
mov ax, 0x0e00 + '#'
xor bx, bx
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
xor bx, bx
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
xor bx, bx
int 0x10
%endif

cli
hlt

; Print message at DS:SI.
print:
; Teletype output	AH=0Eh	AL = Character, BH = Page Number, BL = Color (only in graphic mode)
mov ah, 0x0e
xor bx, bx
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

%if 1
times 0x12000-__org-($-$$) db 0
msg db 13, 10, 'Mello, World!', 13, 10, 0
%endif

;incbin "random.bin"
