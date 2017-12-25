;
; The prefix is a small structure that prefaces the actual code;
; it gives the compression program necessary information.
;

		section .prefix		nowrite progbits align=16
		; Without this ldlinux.raw wouldn't have 0x7c00 bytes at
		; the beginning.
		dd 0
		section .text16
