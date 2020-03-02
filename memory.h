#ifndef _MEMORY_H
#define _MEMORY_H

#include "myRAM.h"

#define PAGE_SIZE 4096

//内存页交换设备号
extern int SWAP_DEV;

//从交换分区读写内存页
#define read_swap_page(nr, buffer) ll_rw_page(READ, SWAP_DEV, (nr), (buffer))
#define write_swap_page(nr, buffer) ll_rw_page(WRITE, SWAP_DEV, (nr), (buffer))

extern unsigned int get_free_page(void);

extern unsigned int put_dirty_page(unsigned int page, unsigned int address);

extern void free_page(unsigned int addr);
void swap_free(int page_nr);
void swap_in(unsigned int* table_ptr);

extern unsigned int HIGH_MEMORY;
//15MB的可用内存空间
#define PAGING_MEMORY (15*1024*1024)
//分页后可用页数
#define PAGING_PAGES ((PAGING_MEMORY) >> 12)
#define MAP_NR(addr) (((addr) - LOW_MEM) >> 12)
#define USED 100

extern unsigned char mem_map [ PAGING_PAGES ];

//页目录表和页表的标志位
#define PAGE_DIRTY 0x40
#define PAGE_ACCESED 0x20
#define PAGE_USED 0x04
#define PAGE_RW 0x02
#define PAGE_PRESENT 0x01

#endif