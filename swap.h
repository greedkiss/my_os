#ifndef _SWAP_H
#define _SWAP_H

#include "memory.h"

#ifndef NULL
#define NULL ((void*) 0); 

#endif

#define SWAP_BITS (4096 << 3)


//交换设备号

#define SWAP_DISK "/home/flipped/code/my_os/swap"

//0~64mb为内核空间，不会被交换出去
//0.12逻辑地址空间为４ＧＢ
#define FIRST_VM_PAGE (TASK_SIZE >> 12)
#define LAST_VM_PAGE (1024*1024)
#define VM_PAGES (LAST_VM_PAGE - FIRST_VM_PAGE)


extern void swap_free(int swap_nr);

extern void swap_in(unsigned int* table_ptr);

extern int try_to_swap_out(unsigned int * table_ptr);

extern int swap_out(void);

extern void init_swapping(void);

#endif