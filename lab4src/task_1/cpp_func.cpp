#include <iostream>

extern "C" void function_from_CPP() {
    std::cout << "This is a function from C++." << std::endl;
} //c++支持函数重载，extern c 用来保证函数名称不被修饰