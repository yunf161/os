#include <iostream>

 //c++支持函数重载，extern c 用来保证函数名称不被修饰
extern "C" void function_from_asm();

int main() {
    std::cout << "Call function from assembly." << std::endl;
    function_from_asm();
    std::cout << "Done by 24325159 LYF ."<<std::endl;
}