; If you meet compile error, try 'sudo apt install gcc-multilib g++-multilib' first

%include "head.include"
jmp your_if
continue:
jmp your_while
end_continue:
jmp student_function_end

your_if:
; put your implementation here
push eax
push ebx
push ecx
mov eax ,[a1]
cmp eax, 0xC
jl break1
cmp eax , 0x18
jl break2
jmp break3



break1:
mov edx, 0
mov ebx ,0x2
idiv ebx
add eax ,1 
mov [if_flag],eax
jmp if_out
break2:
mov ebx ,0x18
sub ebx ,eax
imul eax ,ebx
mov [if_flag],eax
jmp if_out
break3:
shl eax, 0x4
mov [if_flag],eax
jmp if_out

if_out:
pop ecx
pop ebx
pop eax
jmp continue



your_while:
; put your implementation here
push eax
push ebx
push ecx
mov ebx , [a2]
while_fun:
cmp  ebx , 0xc
jl end_while
jmp carry_while


carry_while:
     call my_random
     mov  ecx, ebx
     sub  ecx , 0xc
     add ecx , [while_flag]
     mov [ecx],al
     sub ebx,1
     mov [a2], ebx
     jmp while_fun


end_while:
pop ecx
pop ebx
pop eax
jmp end_continue

%include "end.include"

your_function:
; put your implementation here
push eax
push ebx
push ecx
mov ecx ,0
mov eax , 0
mov ebx, [your_string]
for_begin:
cmp byte [ebx+eax],0
jne for_carry
je fun_out


for_carry:
pushad 
push dword  [ebx+eax]
call  print_a_char
add esp,4
popad
inc eax
jmp for_begin


fun_out:
pop ecx
pop ebx
pop eax
ret




