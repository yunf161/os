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
mov ax, 1                ; 逻辑扇区号第0~15位
mov cx, 0                ; 逻辑扇区号第16~31位
mov bx, 0x7e00           ; bootloader的加载地址
call asm_read_hard_disk  ; 读取硬盘
   
jmp 0x0000:0x7e00        ; 跳转到bootloader

jmp $ ; 死循环

asm_read_hard_disk:                           
mov ah,02H  ;功能号
mov al,5    ;读取五个扇区
mov ch ,0   ;  柱面号低8位
mov cl, 2   ;扇区号加柱面号高两位，扇区是2到6，高两位是0
mov dh ,0    ;  磁头号0
mov dl,80H   ; 读第一个硬盘
int 13H        ; 调用中断
    ret

times 510 - ($ - $$) db 0
db 0x55, 0xaa