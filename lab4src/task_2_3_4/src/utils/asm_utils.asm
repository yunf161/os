[bits 32]

global asm_hello_world
global asm_lidt
global asm_halt
global asm_unhandled_interrupt
global asm_out_port
global asm_in_port
global asm_time_interrupt_handler
global asm_enable_interrupt
extern c_time_interrupt_handler
extern c_time_interrupt_handler_v2

asm_hello_world:
    push eax
    xor eax, eax

    mov ah, 0x03 ;青色
    mov al, '2'
    mov [gs:2 * 0], ax

    mov al, '4'
    mov [gs:2 * 1], ax

    mov al, '3'
    mov [gs:2 * 2], ax

    mov al, '2'
    mov [gs:2 * 3], ax

    mov al, '5'
    mov [gs:2 * 4], ax

    mov al, '1'
    mov [gs:2 * 5], ax

    mov al, '5'
    mov [gs:2 * 6], ax

    mov al, '9'
    mov [gs:2 * 7], ax

    mov al, 'L'
    mov [gs:2 * 8], ax

    mov al, 'Y'
    mov [gs:2 * 9], ax

    mov al, 'F'
    mov [gs:2 * 10], ax

    pop eax
    ret

; void asm_lidt(uint32 start, uint16 limit)
asm_lidt:
    push ebp
    mov ebp, esp
    push eax

    mov eax, [ebp + 4 * 3]
    mov [ASM_IDTR], ax
    mov eax, [ebp + 4 * 2]
    mov [ASM_IDTR + 2], eax
    lidt [ASM_IDTR]

    pop eax
    pop ebp
    ret
    
ASM_IDTR dw 0   ;人为开辟48位空间作为寄存器，dw=define word(16 bits) ,dd=define double words
      dd 0


ASM_UNHANDLED_INTERRUPT_INFO db 'Unhandled interrupt happened, halt...'
                             db 0

; void asm_unhandled_interrupt()
asm_unhandled_interrupt:
    cli
    mov esi, ASM_UNHANDLED_INTERRUPT_INFO
    xor ebx, ebx
    mov ah, 0x03
    mov bx, 22
.output_information:
    cmp byte[esi], 0
    je .end
    mov al, byte[esi]
    mov word[gs:bx], ax
    inc esi
    add ebx, 2
    jmp .output_information
.end:
    jmp $

asm_halt:
    jmp $

; void asm_out_port(uint16 port, uint8 value)
asm_out_port:
    push ebp
    mov ebp, esp

    push edx
    push eax

    mov edx, [ebp + 4 * 2] ; port
    mov eax, [ebp + 4 * 3] ; value
    out dx, al
    
    pop eax
    pop edx
    pop ebp
    ret

; void asm_in_port(uint16 port, uint8 *value)
asm_in_port:
    push ebp
    mov ebp, esp

    push edx
    push eax
    push ebx

    xor eax, eax
    mov edx, [ebp + 4 * 2] ; port
    mov ebx, [ebp + 4 * 3] ; *value

    in al, dx
    mov [ebx], al

    pop ebx
    pop eax
    pop edx
    pop ebp
    ret

asm_time_interrupt_handler:
    pushad
    
    nop ; 否则断点打不上去
    ; 发送EOI消息，否则下一次中断不发生
    mov al, 0x20
    out 0x20, al
    out 0xa0, al
    
    call c_time_interrupt_handler_v2

    mov al, 0x20
	out 0x20, al
	out 0xa0, al
    popad
    iret

; void asm_enable_interrupt()
asm_enable_interrupt:
    sti
    ret