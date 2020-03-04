#include "memory.h"

//每块内存页为4KB，判断当前地址是否在内存页中
#define CODE_SPACE(addr) ((((addr)+4095)&~4095)<current->start_code + current->end_code)

unsigned int HIGH_MEMORY = 0;


//copy 4096字节
int copy_page(char* from, char* to){
    if(from == NULL || to == NULL){
        printf("ERROR!from or to is NULL!\n");
        return 0;
    }
    int i = 0;
    while(i++ < 4096 && (*to++ = *from++));
    return 1;
}

//释放addr线性地址所在的内存页
void free_page(unsigned int addr){
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


//写时复制机制
int copy_page_tables(unsigned int from, unsigned int to, int size){
    unsigned int* from_page_table;
    unsigned int* to_page_table;
    unsigned int this_page;
    unsigned int* from_dir, *to_dir;
    unsigned int new_page;
    unsigned int nr;
    if((from & 0x3fffff) || (to & 0x3fffff))
        printf("４ＭＢ对齐\n");
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
        *to_dir = ((unsigned int) to_page_table) | 7;
        nr = (from == 0)? 0xA0 : 1024;
        for( ; nr-- > 0 ; from_page_table++, to_page_table++){
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

//线性地址为address，内存页面为page，注意是由内存address映射到page上
//也就是说是由线性地址查页目录表和页表得到的page
static unsigned int put_page(unsigned int page, unsigned int address){
    unsigned int tmp, *page_table;
    if(page < LOW_MEM || page >= HIGH_MEMORY)
        printf("不符合规范\n");
    if(mem_map[(page - LOW_MEM) >> 12] != 1)
        printf("该页还未申请\n");
    page_table = (unsigned int *) ((address >> 20) & 0xffc);
    if((*page_table) & 1)
        page_table = (unsigned int *) (0xfffff000 & *page_table);
    else{
        if(!(tmp = get_free_page()))
            return 0;
        *page_table = tmp | 7;
        page_table = (unsigned int*) tmp;
    }
    page_table[(address>>12) & 0x3ff] = page | 7;
    return page;
}

unsigned int put_dirty_page(unsigned int page, unsigned int address){
    unsigned int tmp, *page_table;
    if(page < LOW_MEM || page >= HIGH_MEMORY)
        printf("待映射的page不符合规范\n");
    if(mem_map[(page - LOW_MEM) >> 12] != 1)
        printf("page未申请\n");
    page_table = (unsigned int *) ((address >> 20) & 0xffc);
    if(*page_table & 1){
        page_table = (unsigned int *) (*page_table & 0xfffff000);
    }else{
        if(!(tmp = get_free_page()))
            return 0;
        *page_table = tmp | 7;
        page_table = (unsigned int *) tmp;
    }
    page_table[(address >> 12) & 0x3ff] = page | (PAGE_DIRTY | 7);
}

//取消页共享机制，复制到新的内存页，并置Ｒ／Ｗ位
//table_entry 参数是页表项地址
void un_wp_page (unsigned int * table_entry){
    unsigned int old_page, new_page;
    old_page = 0xfffff000 & *table_entry;
    if(old_page > LOW_MEM && mem_map[MAP_NR(old_page)] == 1){
        //表明没有共享页面
        *table_entry |= 2;
        return;
    }
    if(!(new_page = get_free_page())){
        printf("内存满\n");
        return;
    }
    if(old_page >= LOW_MEM)
        mem_map[MAP_NR(old_page)]--;
    *table_entry = new_page | 7;
    copy_page(old_page, new_page);
}

void do_wp_page(unsigned int error_code, unsigned int address){
    if(address < TASK_SIZE){
        printf("内核代码受保护\n");
        return;
    }
    if(address - current->start_code > TASK_SIZE){
        printf("address非法，所写地址不在进程空间中\n");
        return;
    }
    un_wp_page( (unsigned int*) (*((unsigned int *)((address >> 20) & 0xffc)) & 0xfffff000) + ((address >> 10) & 0xffc) );
}

