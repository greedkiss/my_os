#include "exit.h"

//释放任务槽
void release(struct task_struct * p){
    int i;
    if(!p)
        return;
    if(p == current){
    }
    for(i = 1; i<NR_TASKS ; i++){
        if(task[i] == p){
            task[i] = NULL;
            free_page((unsigned int)p);
            schedule();
            return;
        }
    }
}

//检测任务结构指针
int bad_task_ptr(struct task_struct * p){
    int i;
    if(!p){
        return 0;
    }
    for(i=0; i<NR_TASKS; i++){
        if(task[i] == p)
            return 0;
    }
    return 1;
}

//发送信号
int send_sig(int sig, struct task_struct *p){
    if(!p)
        return 0;
    if((sig == SIGKILL)){
        if(p->state == TASK_STOPPED)
            p->state = TASK_RUNNING;
        p->exit_code = 0;
        p->signal &= ~(1<<(SIGSTOP-1)); 
    }
    if((int) p->sigaction[sig-1].sa_handler == 1)
        return 0;
    p->signal |= (1<<(sig-1));
    return 0;
}

//kill掉进程组
int kill_pg(int pgrp, int sig){
    struct task_struct ** p;
    int found = 0;
    if(sig<1 || sig>32 || pgrp<=0)
        return 0;
    for(p = &LAST_TASK ; p>&FIRST_TASK; --p){
        if((*p)->pgrp == pgrp){
            send_sig(sig, *p);
        }
        found++;
    }
    return found;
}

//终止进程
int kill_proc(int pid, int sig){
    struct task_struct **p;
    if(sig<1||sig>32)
        return 0;
    for(p = &LAST_TASK ; p>&FIRST_TASK; --p){
        if((*p)->pid == pid)
            return send_sig(sig,*p);
    }
}

//系统盘调用
//pid>0，kill某进程
//pid=0, kill当前进程组所有进程
//pid=-1, kill除第一个进程外的所有进程
//pid<-1, 发给进程组-pid的所有进程
int sys_kill(int pid, int sig){
    struct task_struct **p = NR_TASKS + task;
    if(!pid)
        return kill_pg(current->pid, sig);
    if(pid == -1){
        while(--p > &FIRST_TASK){
            return send_sig(sig, *p);
        }
    }
    if(pid < 0)
        return kill_pg(-pid, sig);
    return kill_proc(pid, sig);
}

void do_exit(int code){
    struct task_struct *p;
    int i;
    free_page_tables(current->ldt[1], TASK_SIZE);
    free_page_tables(current->ldt[2], TASK_SIZE);
    for(i=0; i<NR_OPEN; i++){
        if(current->filp[i])
            hsc_close(i);
    }
    iput(current->pwd);
    current->pwd =NULL;
    current->state = TASK_ZOMBIE;
    iput(current->root);
    current->root=NULL;
    iput(current->executable);
    current->executable =NULL;
    release(current);
    schedule();
}


