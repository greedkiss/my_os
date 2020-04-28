#include "myRAM.h"
#include "head.h"

void (*fun)();

int main(int argc, char** argv){
    //获取16MB内存空间
    getmyRAM();

    setup_idt();
    unsigned long long* address =(unsigned long long*)RAM + IDTR;
    fun = (void *)(*(address + 3));
    (*fun)();

    return 0;
}