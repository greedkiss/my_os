#ifndef _MYRAM_H
#define _MYRAM_H

#include <stdio.h>
#include <stdlib.h>
//物理地址起始位置
extern const char* RAM;

//申请16MB物理内存空间
extern void getmyRAM();

#endif