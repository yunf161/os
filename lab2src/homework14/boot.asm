org 0x7c00
[bits 16]


xor ax, ax ; eax = 0


; 初始化段寄存器, 段地址全部设为0
mov ds, ax
mov ss, ax
mov es, ax
mov fs, ax
mov gs, ax
mov bx, ax
mov si, ax
mov di, ax
mov cx, ax
;bx用来存x，si存y，di存偏移地址,cx表示上下方向，0下，bp存左右方向，1右

; 初始化栈指针
mov sp, 0x7c00

mov ax, 0xb800
mov gs, ax   
;段寄存器gs


mov bx, 2
mov bp, 1
carry_out:
   call delay
   cmp bx,24
   je turn_up
   cmp bx,0
   je turn_down
ca1:
   cmp si,79

   je turn_left
   cmp si,0
   je turn_right
ca2:
   ;方向修正
    mov di, bx
    mov ax, 80
    mul di        ; ax = bx * 80
    add ax, si    ; ax = bx*80 + si
    shl ax, 1     ; ax = 2*(bx*80+si)
    mov di, ax    ; di = 偏移地址
    ;计算偏移地址

    
    mov ax, si    ; 把列号放进 AX
    mov dl, 10    ; 准备除以 10
    div dl        ; 取个位
    add ah, '0'  ;变成asc
    mov dl,al
    mov al, ah ; ax低位是列号的个位
    inc dl
    mov ah, dl ;ax高位，颜色，取决于列号的十位+1
    mov [gs:di], ax
    ;填充一边
   

    mov ax, 0x18
    sub ax, bx
    mov di, ax
    mov ax, 80
     mul di        ; ax = (24-bx )* 80
    sub ax, si    
    add ax, 79      ; ax=(24-bx)*80+79-si
    shl ax, 1     ; ax = 2*((24-bx)*80+79-si)
    mov di, ax    ; di = 偏移地址
    ;计算偏移地址

    
     mov ax, si    ; 把列号放进 AX
    mov dl, 10    ; 准备除以 10
    div dl        ; 取个位
    add ah, '0'  ;变成asc
    mov dl,al
    mov al, ah ; ax低位是列号的个位
    inc dl
    mov ah, dl ;ax高位，颜色，取决于列号的十位+1
    mov [gs:di], ax
    ;填充另一边.

    

    cmp cx,1
    je go_up
    jne go_down
    ;上下移动
shifting:
    cmp bp, 1
    je go_right
    jne go_left
    ;左右移动
   

    
     

go_up:
    sub bx,1
    jmp shifting
go_down:
    add bx,1
    jmp shifting
go_right:
    add si,1
    jmp carry_out
go_left:
    sub si,1
    jmp carry_out
  
turn_up:
    mov cx ,1
 
    jmp ca1
turn_down:
    mov cx ,0
 
    jmp ca1
turn_left:
    mov bp ,0
  
    jmp ca2
turn_right:
    mov bp ,1
 
    jmp ca2

delay:
    push dx
    push cx         ; 保存两个寄存器
    
    mov cx, 0x03FF  ; 外层计数器
    
delay_outer:
    mov dx, 0xFFFF  ; 内层计数器
    
delay_inner:
    sub dx, 1
    cmp dx, 0
    jne delay_inner
    
    sub cx, 1
    cmp cx, 0
    jne delay_outer
    
    pop cx
    pop dx
    ret


jmp $ ; 死循环

times 510 - ($ - $$) db 0
db 0x55, 0xaa



