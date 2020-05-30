#include "myRAM.h"
#include "head.h"
#include "trap.h"
#include "sched.h"
#include "swap.h"
#include "myMemory.h"
#include "buffer.h"
#include "super.h"
#include "namei.h"

void (*fun)();

int main(int argc, char** argv){
    //获取16MB内存空间
    getmyRAM();
    //设置ldt,gdt and pageing
    startup_32();
    //中断向量表初始化
    trap_init();
    //初始化文件为ext2文件系统
    initFileSys();
    //初始化交换分区
    init_swapping();
    //初始化buffer
    getBuffer();
    initBuffer();
    //挂载根文件系统
    mount_root();
    //手动创建根目录
    sys_mkdir("/",0777);
   //创建目录
    sys_mkdir("/bin", 0777 | S_IFDIR);
    sys_mkdir("/boot", 0777 | S_IFDIR);
    sys_mkdir("/dev", 0777 | S_IFDIR);
    sys_mkdir("/etc", 0777 | S_IFDIR);
    sys_mkdir("/home", 0777 | S_IFDIR);
    sys_mkdir("/home/root", 0777 | S_IFDIR);
    //进程初始化
    sched_init();
    
    // unsigned long long* address =(unsigned long long*)RAM + IDTR;
    // fun = (void *)(*(address + 2));
    // (*fun)();

    return 0;
}