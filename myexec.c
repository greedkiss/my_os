#include "myexec.h"

int sys_uselib(char * library){
    struct m_inode * inode;
    unsigned int base;


}


//创建环境和参数指针表
unsigned int * creat_tables(char * p, int argc, int envc){
    unsigned int * argv, * envp;
    unsigned int * sp;
    sp = (unsigned int *) (0xfffffffc & (unsigned int) p);
    sp -= envc + 1;
    envp = sp;
    sp -= argc + 1;
    argv = sp;
    --sp;
    *sp =(unsigned int) envp;
    --sp;
    *sp =(unsigned int) argv;
    --sp;
    *sp =(unsigned int) argc;
    while(argc-->0){
        *argv = (unsigned int) p;
        p++;
        argv++;
    }
    *argv = 0;
    while(envc-->0){
        *envp = (unsigned int) p;
        p++;
        envp++;
    }
    *envp = 0;
    return sp;
}


int count(char ** argv){
    int i=0;
    unsigned int * tmp = (unsigned int *) *argv;
    while(*tmp++){
        i++;
    }
    return i;
}

int do_execve(unsigned long * eip, long tmp, char * filename){
    struct m_inode * inode;
    struct buffer_head * bh;
    Elf64_Ehdr ex;
    unsigned int page[MAX_ARG_PAGES];
    int i, argc, envc;
    //是否是脚本运行程序
    int sh_bang = 0;
    unsigned int p = PAGE_SIZE*MAX_ARG_PAGES - 4;
    for (i = 0; i<MAX_ARG_PAGES; i++)
        page[i] = 0;
    if(!(inode = namei(filename)))
        printf("获取不到可执行文件\n");
    //省略权限判断
    //脚本文件解释程序
    if(!(bh = bread(inode->i_dev, inode->i_zone[0])))
        printf("读不到可执行文件\n");
    ex = *((Elf64_Ehdr *) bh->b_data);
    //脚本文件处理
    if((bh->b_data[0] == '#') && (bh->b_data[i] == '!') && (!sh_bang)){
        printf("脚本文件\n");
        
        
    }
    current->executable = inode;
    current->signal = 0;
    for(i = 0; i<32; i++){
        current->sigaction[i].sa_mask = 0;
        current->sigaction[i].sa_flags = 0;
        if(current->sigaction[i].sa_handler != SIG_IGN)
            current->sigaction[i].sa_handler = NULL;
    }
    
    current->tss.entry = (unsigned long long*)ex.e_entry;

}


