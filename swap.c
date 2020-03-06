#include "swap.h"

#define SWAP_BITS (4096 << 3)

static char * swap_bitmap = NULL

//交换设备号
int SWAP_DEV = 0;

//0~64mb为内核空间，不会被交换出去
//0.12逻辑地址空间为４ＧＢ
#define FIRST_VM_PAGE (TASK_SIZE >> 12)
#define LAST_VM_PAGE (1024*1024)
#define VM_PAGES (LAST_VM_PAGE - FIRST_VM_PAGE)

//检查bit位
int bit(char * a, int b){
    return !!((*a) & ((1ULL) << (b)));
}

//清bit位
int clrbit(char * a , int b){
    int res = bit(a, 7 - b);
    b = 7 -b;
    *a &= ~((1ULL) << (b));
    return res;
}

//置bit位
int setbit(char * a, int b){
    int res = bit(a, 7 - b);
    b = 7 - b;
    *a |= ((1ULL) << (b));
    return res;
}

static int get_swap_page(void){
    int nr;
    if(!swap_bitmap)
        return 0;
    for(nr = 1; nr < 32768 ; nr++){
        if(clrbit(swap_bitmap, nr))
            return nr;
    }
    return 0;
}

void swap_free(int swap_nr){
    if(!swap_nr)
        return;
    if(swap_bitmap && swap_nr < SWAP_BITS){
        if(!setbit(swap_bitmap, swap_nr))
            return;
    }
    printf("该交换分区页未被占用\n");
    return;
}