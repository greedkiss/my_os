#ifndef _HEAD_H
#define _HEAD_H

#include "myRAM.h"

extern const char* RAM;

//idt中断描述符的基地址
#define IDTR 0x6000;

//gdt全局描述符表的基地址
#define GDTR 0x6800;

//页表位置
#define pg0 0x1000;
#define pg1 0x2000;
#define pg2 0x3000;
#define pg3 0x4000;
//页目录表
#define pg_dir 0x0;

extern void startup_32();

#endif 