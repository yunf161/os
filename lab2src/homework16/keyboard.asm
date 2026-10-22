org 0x7c00
[bits 16]

start:
mov ah ,00h
int 16h
mov ah,0Eh
int 10h
jmp start

jmp $ ; 死循环

times 510 - ($ - $$) db 0
db 0x55, 0xaa
