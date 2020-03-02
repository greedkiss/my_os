#include <memory.h>

//每块内存页为4KB，判断当前地址是否在内存页中
#define CODE_SPACE(addr) ((((addr)+4095)&~4095)<current->start_code + current->end_code)

unsigned long HIGH_MEMORY = 0;

//copy 4096字节
int copy_page(char* from,char* to){
    if(from == NULL || to == NULL){
        printf("ERROR!from or to is NULL!");
        return 0;
    }
    int i = 0;
    while(i++ < 4096 && (*to++ = *from++));
    return 1;
}

