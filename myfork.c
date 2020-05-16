#include "myfork.h"

//最新进程号，全局变量
int last_pid = 0;

int find_empty_process(void){
    int i;
    repeat: 
    if((++last_pid) < 0)
        last_pid = 1;
    for(i = 0; i < NR_TASKS; i++){
        if(task[i] && ((task[i]->pid == last_pid) || task[i]->pgrp == last_pid)){
            goto repeat;
        }
    }
    for(i = 1; i< NR_TASKS; i++){
        if(!task[i])
            return i;
    }
    printf("最多64个进程，超限\n");
    return -1;
}

//写时验证，可调用写时复制函数
//addr逻辑地址
void verify_area(void * addr, int size){
    unsigned int start;
    start = (unsigned int) addr;
    size += start & 0xfff;
    start &= 0xfffff000;
    start += current->ldt[2];
    while(size>0){
        size-=4096;
        write_verify(start);
        start+=4096;
    }
}

//复制内存页表
int copy_mem(int nr, struct task_struct * p){
    unsigned int old_data_base, new_data_base, data_limit;
    unsigned int old_code_base, new_code_base, code_limit;
    old_code_base = current->ldt[1];
    old_data_base = current->ldt[2];
    if(old_code_base != old_data_base)
        printf("数据段和代码段位置必须相同\n");
    new_data_base = new_code_base = nr * TASK_SIZE;
    p->start_code = new_code_base;
    p->ldt[1] = new_code_base;
    p->ldt[2] = new_data_base;
    if(copy_page_tables(old_data_base, new_data_base, TASK_SIZE)){
        free_page_tables(new_data_base, TASK_SIZE);
    }
    return 0;
}


//复制进程
void copy_process(){
    struct task_struct * p;
    int i, nr;
    //文件结构体
    struct file * f;
    p = (struct task_struct *)get_free_page();
    if(!p)
        printf("申请pcb空间失败\n");
    nr = find_empty_process();
    task[nr] = p;
    //复制进程结构
    *p = *current; 
    p->state = TASK_UNINTERRUPTIBLE;
    p->pid = last_pid;
    p->signal = 0;
    p->alarm = 0;
    // p->start_time = jiffies;
    p->tss.ldt = _LDT(nr);
    if(copy_mem(nr, p)){
       task[nr] = NULL;
       free_page((unsigned int)p);
       printf("复制页表出错\n");
    }
    for(i = 0; i < NR_OPEN; i++){
        if(f = p->filp[i])
            f->f_count++;
    }
    if(current->pwd)
        current->pwd->i_count++;
    if(current->root)
        current->root->i_count++;
    if(current->executable)
        current->executable->i_count++;
    if(current->library)
        current->library->i_count++;
    p->p_pptr = current;
    p->p_cptr = 0;
    p->p_ysptr = 0;
    p->p_osptr = current->p_cptr;
    if(p->p_osptr)
        p->p_osptr->p_ysptr = p;
    current->p_cptr = p;
    p->state = TASK_RUNNING;
    unsigned long long * base_gdt = (unsigned long long *)RAM + GDTR;
    set_tss_desc(base_gdt+(nr<<1) + FIRST_TSS_ENTRY, (unsigned long long)&(p->tss));
    set_ldt_desc(base_gdt+(nr<<1) + FIRST_LDT_ENTRY, (unsigned long long)&(p->ldt));
    return last_pid;
}