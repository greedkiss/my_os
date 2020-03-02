#include "memory.h"

//每块内存页为4KB，判断当前地址是否在内存页中
#define CODE_SPACE(addr) ((((addr)+4095)&~4095)<current->start_code + current->end_code)

unsigned int HIGH_MEMORY = 0;


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

//释放addr线性地址所在的内存页
void free_page(unsigned int addr){
    //物理内存最低端，RAM是申请的１６MB内存空间地址
    unsigned int LOW_MEM = (unsigned int)RAM + 0x100000;
    if(addr < LOW_MEM) return;
    if(addr > HIGH_MEMORY)
        printf("trying to free non_exit page!\n");
    addr -= LOW_MEM;
    addr >>= 12;
    if(mem_map[addr]--) return;
    mem_map[addr] = 0;
    printf("trying to free free page!\n");
}

//释放页目录项，一个页目录项映射一个内存页表，然后一个内存页表对应1024个物理内存页
//３２位地址空间，页目录表和页表最低位都是Ｐ存在位，页目录项最低位为１表示存在页目录项到页表的映射，页表项P为１表示页面在内存
//否则在交换分区中
int free_page_tables(unsigned int from, unsigned int size){
    unsigned int* pg_table;
    unsigned int* dir, nr;
    if(from & 0x3fffff)
        printf("释放的空间需要4MB地址对齐\n");
    if(!from)
        printf("释放的空间不可为内核空间\n");
    size = (size + 0x3fffff) >> 22;
    dir = (unsigned int*) ((from >> 20)& 0xffc);
    for( ; size-- ; dir++){
        if(!(1 & *dir))
            continue;
        pg_table = (unsigned int*) (0xfffff000 & *dir);
        for(nr=0 ; nr<1024 ; nr++){
            if(*pg_table){
                if(1 & *pg_table)
                    free_page(0xfffff000 & *pg_table);
                else
                    swap_free(*pg_table >> 1);
            }
            pg_table++;
        }
        free_page(0xfffff000 & *dir);
        *dir = 0;
    }
    return 0;
}

int copy_page_tables(unsigned int from, unsigned int to, int size){
    unsigned int* from_page_table;
    unsigned int* to_page_table;
    unsigned int this_page;
    unsigned int* from_dir, *to_dir;
    unsigned int new_page;
    unsigned int nr;
    if((from & 0x3fffff) || (to & 0x3fffff))
        printf("４ＭＢ对齐");
    from_dir = (unsigned int*) ((from >> 20) & 0xffc);
    to_dir = (unsigned int*) ((to >> 20) & 0xffc);
    size = ((unsigned) (size + 0x3fffff)) >> 22;
    for( ; size-->0 ; from_dir++, to_dir++){
        if(1 & *to_dir){
            printf("该目录项已经被占用\n");
            return 1;
        }
        if(!(1 & *from_dir))
            continue;
        from_page_table = (unsigned int*) (0xfffff000 & *from_dir);
        if(!(to_page_table = (unsigned int *) get_free_page()))
            return -1;
        *to_dir = ((unsigned int*) to_page_table) | 7;
        nr = (from == 0)? 0xA0 : 1024;
        for( ; nr-- > 0 ; from_page_table++,to_page_table++){
            this_page = *from_page_table;
            if(!this_page)
                continue;
            if(!(1 & this_page)) {
                if(!(new_page = get_free_page()))
                    return -1;
                read_swap_page(this_page>>1, (char *) new_page);
                *to_page_table = this_page;
                *from_page_table = new_page | (PAGE_DIRTY | 7);
                continue;
            }
            this_page &= ~2;
            *to_page_table = this_page;
            if(this_page > LOW_MEM){
                *from_page_table = this_page;
                this_page -= LOW_MEM;
                this_page >>= 12;
                mem_map[this_page]++;
            }
        }
    }
    return 0;
}