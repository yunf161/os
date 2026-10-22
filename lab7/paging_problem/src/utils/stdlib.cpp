#include "os_type.h"

template<typename T>
void swap(T &x, T &y) {
    T z = x;
    x = y;
    y = z;
}
void itos(char *numstr,uint32 num,uint32 base) {
  int length =0;
  while(num){
    int temp = num % base;
    num/=base;
    if(temp>9){
        numstr[length++] = temp - 10 + 'A';
    }
    else{
        numstr[length++] = temp + '0';
    }
  }
    
    if(!length){
        numstr[length++] = '0';
    }
    for(int i=0;i<length/2;++i){
        swap(numstr[i],numstr[length-1-i]);
}
numstr[length] = '\0';
}

void memset(void *memory, char value, int length)
{
    for (int i = 0; i < length; ++i)
    {
        ((char *)memory)[i] = value;
    }
}
int ceil(const int dividend, const int divisor) {
    return (dividend + divisor - 1) / divisor;
}