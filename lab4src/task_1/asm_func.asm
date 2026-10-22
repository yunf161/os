[bits 32]
global function_from_asm  ;在C/C++调用汇编函数之前，我们先需要在汇编代码中将函数声明为global
extern function_from_C    ;调用前extern声明
extern function_from_CPP     

function_from_asm:
    call function_from_C
    call function_from_CPP
    ret