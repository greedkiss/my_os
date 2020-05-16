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

//获取空闲页
unsigned int get_free_page(){
    int i = 0;
    for( ; i < PAGING_PAGES; i++ ){
        if(mem_map[i] == 0)
            break;
    }
    return LOW_MEM + i<<12;
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
unsigned int put_page(unsigned int page, unsigned int address){
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

//和put_page功能相同，只不过最后置脏位
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

//写时出错，页面共享
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

//写页面验证
void write_verify(unsigned int address){
    unsigned int page;
    if(!((page = *((unsigned int *) ((address >> 20) & 0xffc)) ) & 1))
        return;
    page &=  0xfffff000;
    page += ((address >> 10) & 0xffc);
    //判断r/w位和p位
    if((3 & *(unsigned int *) page) == 1)
        un_wp_page((unsigned int *) page);
    return;
}

//申请空闲页并将address映射到空闲页
void get_empty_page(unsigned int address){
    unsigned int tmp;
    if(!(tmp = get_free_page()) || !put_page(tmp, address)){
        free_page(tmp);
        printf("内存溢出\n");
    }
}

//页面共享，p进程共享页面给current，并且页面都置为只读
static int try_to_share (unsigned int address, struct task_struct * p){
    unsigned int from;
    unsigned int to;
    unsigned int from_page;
    unsigned int to_page;
    unsigned int phys_addr;
    //进程逻辑地址空间都是64MB对齐，所以不会有零头，不用担心右移２０位后低位被舍去
    from_page = to_page =((address >> 20) & 0xffc);
    from_page += ((p->start_code >> 20) & 0xffc);
    to_page += ((current->start_code >> 20) & 0xffc);
    from = *(unsigned int *) from_page;
    if(!(from & 1))
        return 0;
    from &= 0xfffff000;
    from_page = from + ((address >> 10) & 0xffc);
    phys_addr = *(unsigned int *)from_page;
    //0x41是dirty位和p位，如果干净且存在则继续下去
    if((phys_addr & 0x41) != 0x01)
        return 0;
    phys_addr &= 0xfffff000;
    if(phys_addr >= HIGH_MEMORY || phys_addr < LOW_MEM)
        return 0;
    to = *(unsigned int *) to_page;
    if(!(to & 1)){
        if(to = get_free_page())
            *(unsigned int *) to_page = to | 7;
        else{
            printf("内存不够\n");
            return 0;
        }
    }
    to &= 0xfffff000;
    to_page = to + ((address >> 10) & 0xffc);
    if(1 & *(unsigned int *) to_page){
        printf("内核出错，页面已经存在\n");
        return 0;
    }
    *(unsigned int *) from_page &= ~2;
    *(unsigned int *) to_page = *(unsigned int *) from_page;
    phys_addr -= LOW_MEM;
    phys_addr >>= 12;
    mem_map[phys_addr]++;
    return 1;
}

//共享页面，当出现缺页异常时候，与运行同一个执行文件的其他进程进行页面共享处理
static int share_page(struct m_inode * inode , unsigned int address){
    struct task_struct ** p;
    //inode->i_count < 2 表示只有一个进程在运行该文件，所有找不到其他进程
    if(inode->i_count < 2 || !inode)
        return 0;
    for(p = &LAST_TASK ; p > &FIRST_TASK ; --p){
        if(!*p)
            continue;
        if(current == *p)
            continue;
        if(address < LIBRARY_OFFSET){
            if(inode != (*p)->executable)
                continue;
        }else{
            if(inode != (*p)->library)
                continue;
        }
        if(try_to_share(address, *p))
            return 1;
    }
    return 0;
}

//缺页处理函数
void do_no_page(unsigned int error_code, unsigned int address){
    int nr[4];
    unsigned int tmp;
    unsigned int page;
    int block, i;
    struct m_inode * inode;
    //先判断address合法性
    if(address < TASK_SIZE){
        printf("内核代码不可能缺页\n");
        return;
    }
    if(address - current->start_code > TASK_SIZE){
        printf("address超出限度\n");
        return;
    }
    page = *(unsigned int*) ((address >> 20) & 0xffc);
    if(page & 1){
        page &= 0xfffff000;
        page += (address >> 10) & 0xffc;
        //页表项内容
        tmp = *(unsigned int*) page;
        if(tmp && !(1 & tmp)){
            swap_in((unsigned int*) page);
            return;
        }
    }
    //算线性地址相对于执行文件映像的偏移值
    address &= 0xfffff000;
    tmp = address - current->start_code;
    if(tmp >= LIBRARY_OFFSET){
        inode = current->library;
        block = 1 + tmp / BLOCK_SIZE;
    }else if(tmp < current->end_data){
        inode = current->executable;
        block = 1 + tmp / BLOCK_SIZE;
    }else{
        inode = NULL;
        block = 0;
    }
    //如果进程是访问动态申请的页面或者存放栈信息而引起的缺页，inode = NULL
    if(!inode){
        get_empty_page(address);
        return;
    }
    if(share_page(inode, tmp))
        return;
    if(!(page = get_free_page())){
        printf("内存不够\n");
        return;
    }
    for(i = 0; i<4; block++,i++){
        nr[i] = bmap(inode, block);
    }
    bread_page(page, inode->i_dev, nr);
    //将多出end_data部分数据清零
    i = tmp + 4096 - current->end_data;
    if(i > 4095)
        i = 0;
    tmp = page + 4096;
    while(i-- > 0){
        tmp--;
        *(char * )tmp = 0;
    }
    if(put_page(page, address))
        return;
    free_page(page);
}

//内存初始化函数
//start_mem是主存开始的位置，end_mem一般是16mb
//将1-16MB空间的内存页先置１００后将主存内存页清零
void mem_init(int start_mem, int end_mem){
    int i;
    HIGH_MEMORY = end_mem;
    for(i = 0 ; i < PAGING_PAGES ; i++){
        mem_map[i] = USED;
    }
    i = MAP_NR(start_mem);
    end_mem -= start_mem;
    end_mem >> 12;
    while(end_mem-- > 0){
        mem_map[i++] = 0;
    }
}


//内核调试可用于键盘中断处理
void show_mem(void){
    int i,j,k,free=0,total=0;
    int shared=0;
    unsigned int * pg_tbl;
    unsigned long long * base_dir = (unsigned long long *)RAM + pg_dir;
    printf("内存信息：\n");
    for(i =0; i<PAGING_PAGES; i++){
        if(mem_map[i] == USED){
            continue;
        }
        total++;
        if(!mem_map[i])
            free++;
        else
            shared += mem_map[i] - 1;
    }
    printf("空闲页数量:%d;内存共有%d页\n", free, total);
    printf("共享页数量:%d", shared);
    k = 0;
    for(i = 4; i<1024; ){
        if(1&base_dir[i]){
            if(base_dir[i] > HIGH_MEMORY){
                printf("第%d页目录项不正常\n", i);
                i++;
                continue;
            }
            if(base_dir[i] > LOW_MEM)
                free++,k++;
            pg_tbl = (unsigned int *) (0xfffff000 & base_dir[i]);
            for(j=0; j<1024; j++){
                if(pg_tbl[j]&1 && pg_tbl[j] > LOW_MEM){
                    if(pg_tbl[j] > HIGH_MEMORY){
                        printf("第%d页表项不正常\n", j);
                    }else{
                        k++,free++;
                    }
                }
            }
        }
        i++;
        if(!(i&15) && k){
            k++, free++;
            printf("进程%d: %d页", (i>>4)-1 , k);
        }
    }
    printf("内存中正在使用的页面数: %d (%d)\n", free - shared, total);
}