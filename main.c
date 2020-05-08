#include "myRAM.h"
#include "head.h"
#include "trap.h"
#include "sched.h"

void (*fun)();

int main(int argc, char** argv){
    //获取16MB内存空间
    getmyRAM();
    //设置ldt,gdt and pageing
    startup_32();
    //中断向量表初始化
    trap_init();
    //进程初始化
    sched_init();
    
    
    // unsigned long long* address =(unsigned long long*)RAM + IDTR;
    // fun = (void *)(*(address + 2));
    // (*fun)();

    return 0;
}