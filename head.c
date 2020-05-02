#include "head.h"

//从相对地址为0的地方开始进行初始化操作
//linux内核是从绝对地址为0的内存开始初始化

//哑中断，初始化idt
void ignore_int(){
    printf("unknown interrupt，默认为哑中断\n");
    return;
}

//init idt
void setup_idt(){
    unsigned long long * base_idt =(unsigned long long*)RAM + IDTR;
    unsigned long long call_gate = (unsigned long long)(&ignore_int);
    for(int i = 0; i < 256; i++, base_idt++){
        *base_idt = call_gate;
    }
}

//init gdt
void setup_gdt(){
    unsigned long long * base_gdt = (unsigned long long *)RAM + GDTR;
    unsigned long long code = 0x00c09a0000000fff;
    unsigned long long data = 0x00c0920000000fff;
    //第一个描述符为NULL
    *base_gdt = 0;
    //第二个为内核代码段描述符
    *(base_gdt+1) = code;
    //第三个是内核数据段描述符
    *(base_gdt+2) = data;
}

//设置一个内存页目录表和四个内存页表
void setup_paging(){
    unsigned int * base_dir = (unsigned int *)RAM + pg_dir;
    //页目录表
    //存在用户可读可写
    *(base_dir) = pg0 + 7;
    *(base_dir + 1) = pg1 + 7;
    *(base_dir + 2) = pg2 + 7;
    *(base_dir + 3) = pg3 + 7;
    //页表
    unsigned int * base_pg0 = (unsigned int *)RAM + pg0;
    unsigned int pg_des = 0xfff007;
    for(int i = (1024*4-1); i >= 0; i--){
        *(base_pg0 + i) = pg_des;
        pg_des -= 0x1000;
    }
}

void startup_32(){
    setup_idt();
    setup_gdt();
    setup_paging();
}


//设置全局描述表
void set_tss_desc(unsigned long long* base, unsigned long long addr){
    *base = addr;
}

void set_ldt_desc(unsigned long long* base, unsigned long long addr){
    *base = addr;
}