#ifndef _MEMORY_H
#define _MEMORY_H


#include "myRAM.h"
#include "swap.h"
#include "head.h"
#include "sched.h"
#include "rwblock.h"

#define PAGE_SIZE 4096

extern unsigned long long * get_free_page(void);

extern unsigned int put_dirty_page(unsigned int page, unsigned int address);

extern void free_page(unsigned int addr);
void swap_free(int page_nr);
void swap_in(unsigned int* table_ptr);

extern unsigned int HIGH_MEMORY;

//物理内存最低端，RAM是申请的１６MB内存空间地址
#define LOW_MEM 0x100000

//15MB的可用内存空间
#define PAGING_MEMORY (15*1024*1024)
//分页后可用页数
#define PAGING_PAGES ((PAGING_MEMORY) >> 12)
#define MAP_NR(addr) (((addr) - LOW_MEM) >> 12)
#define USED 100

unsigned char mem_map [ PAGING_PAGES ];

//复制４kb页
extern int copy_page(char* from, char* to);

extern void free_page(unsigned int addr);

extern int free_page_tables(unsigned int from, unsigned int size);

extern int copy_page_tables(unsigned int from, unsigned int to, int size);

extern unsigned int put_page(unsigned int page, unsigned int address);

extern unsigned int put_dirty_page(unsigned int page, unsigned int address);

extern void un_wp_page (unsigned int * table_entry);

extern void do_wp_page(unsigned int error_code, unsigned int address);

extern void write_verify(unsigned int address);

extern void get_empty_page(unsigned int address);

extern void do_no_page(unsigned int error_code, unsigned int address);

extern void mem_init(int start_mem, int end_mem);

extern void show_mem(void);


//页目录表和页表的标志位
#define PAGE_DIRTY 0x40
#define PAGE_ACCESED 0x20
#define PAGE_USED 0x04
#define PAGE_RW 0x02
#define PAGE_PRESENT 0x01

#endif