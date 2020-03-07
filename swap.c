#include "swap.h"

#define SWAP_BITS (4096 << 3)

static char * swap_bitmap = NULL;

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

//获取空闲的交换页面号并复位为０，表示被占用
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

//释放交换分区中的指定页
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

//从交换分区中读页进内存,置位dirty,u/s,r/w.p
void swap_in(unsigned int* table_ptr){
    int swap_nr;
    unsigned long page;
    if(!swap_bitmap){
        printf("没有交换位图\n");
        return;
    }
    if(1 & *table_ptr){
        printf("该内存页已经被占用\n");
        return;
    }
    swap_nr = *table_ptr >> 1;
    if(!swap_nr){
        printf("该交换页不存在\n");
        return;
    }
    if(!(page = get_free_page())){
        printf("内存溢出\n");
        return;
    }
    read_swap_page(swap_nr, (char *)page);
    if(setbit(swap_bitmap, swap_nr)){
        printf("该内存页已经被换入\n");
    }
    *table_ptr = page | (PAGE_DIRTY | 7);
}

//判断dirty位，若已被修改且无其他进程共享则交换出去，否则直接释放
int try_to_swap_out(unsigned int * table_ptr){
    unsigned int page;
    unsigned int swap_nr;
    page = *table_ptr;
    if(!(PAGE_PRESENT & page)){
        printf("将要交换出去的内存页不存在\n");
        return 0;
    }
    if(page - LOW_MEM > PAGING_MEMORY){
        printf("将要换出去的内存页大于最高端地址\n");
        return 0;
    }
    if(PAGE_DIRTY & page){
        page &= 0xfffff000;
        if(mem_map[MAP_NR(page)] != 1)
            return 0;
        if(!(swap_nr = get_swap_page()))
            return 0;
        *table_ptr = swap_nr << 1;
        write_swap_page(swap_nr, (char *)page);
        free_page(page);
        return 1;
    }
    *table_ptr = 0;
    free_page(page);
    return 1;
}

//当内存满的时候调用，将内存页交换出去
int swap_out(void){
    static int dir_entry = FIRST_VM_PAGE >> 10;
    static int page_entry = -1;
    int counter = VM_PAGES;
    int pg_table;
    while(counter > 0){
        pg_table = pg_dir[dir_entry];
        if(pg_table & 1)
            break;
        counter -= 1024;
        dir_entry ++;
        //重新找
        if(dir_entry >= 1024)
            dir_entry = FIRST_VM_PAGE >> 10;
    }
    pg_table &= 0xfffff000;
    while(counter-- > 0){
        page_entry++;
        if(page_entry >= 1024){
            page_entry = 0;
        repeat:
            dir_entry++;
            if(dir_entry >= 1024)
                dir_entry = FIRST_VM_PAGE >> 10;
            pg_table = pg_dir[dir_entry];
            if(!(pg_table&1)){
                if((counter -= 1024) > 0)
                    goto repeat;
            }else{
                break;
            }
            pg_table &= 0xfffff000;
        }
        //相当于数组项
        if(try_to_swap_out(page_entry + (unsigned int *) pg_table))
            return 1;
    } 
    printf("交换分区满\n");
    return 0;
}

void init_swapping(void){
    
}