org 0x7c00
[bits 16]
xor ax, ax ; eax = 0
; 初始化段寄存器, 段地址全部设为0
mov ds, ax
mov ss, ax
mov es, ax
mov fs, ax
mov gs, ax

; 初始化栈指针
mov sp, 0x7c00
mov ax, 0xb800
mov gs, ax

mov ah, 02h
mov bh, 0
mov dh, 10
mov dl, 20
int 10h
;设置光标在10行20列

mov ah, 03h
mov bx, 0
int 10h

;读取光标 AX=0，CH=行扫描开始，CL=行扫描结束，DH=行，DL=列

mov ah, 09h
mov al, 'A'
mov bh, 0
mov bl, 07h
mov cx, 1
int 10h
;在光标处写a

jmp $ ; 死循环
times 510 - ($ - $$) db 0
db 0x55, 0xaa

