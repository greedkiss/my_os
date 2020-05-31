#include "sched.h"

//信号编号是1-32
#define _S(nr) (1 << ((nr) - 1))

//除了这两个信号外其他都是可阻塞的信号
#define _BLOCKABLE (~(_S(SIGKILL) | _S(SIGSTOP)))

//进程信息调试
void show_task(int nr, struct task_struct * p){
    int i, j = 4096 - sizeof(struct task_struct);
    printf("%d: pid = %d, state = %d, father = %d, child = %d, ",nr, p->pid, p->state, p->p_pptr->pid, p->p_cptr? p->p_cptr->pid : -1);
    i=0;
    while(i<j && !((char *)(p + 1))[i])
        i++;
    printf("内核栈可用字节：%d/%d\n", i, j);
    if(p->p_ysptr || p->p_osptr)
        printf("younger sib=%d, older  sib=%d\n", p->p_ysptr ? p->p_ysptr->pid : -1, p->p_osptr ? p->p_osptr->pid : -1);
    else
        printf("\n");
}

void show_state(void){
    int i;
    printf("任务信息：\n");
    for(i = 0; i < NR_TASKS; i++){
        if(task[i])
            show_task(i, task[i]);
    }
}

// //时钟中断处理程序
// extern int time_interrupt(void);
//系统调用中断处理程序
// extern int system_call(void);

union task_union {
    struct task_struct task;
    char stack[PAGE_SIZE];
};

static union task_union init_task = {INIT_TASK};

//从开机算起的时间滴答值
unsigned int jiffies = 0;

unsigned int startup_time = 0;

int jiffies_offset = 0;

//switch_to切换的进程号
int next;

struct task_struct * current = &(init_task.task);
// struct task_struct * last_task_used_math ;

struct task_struct * task[NR_TASKS] = {&(init_task.task), };

//任务0的内核栈，初始化后作为任务0和任务1的用户栈
int user_stack [PAGE_SIZE >> 2];

struct {
    int* a;
    short b;
}stack_start = {&user_stack [PAGE_SIZE>>2], 0x10};

void exec_switch_to(unsigned long long* ret_addr){
    *ret_addr = task[next]->tss.entry;
    current = task[next];
}

void switch_to(){
    __asm__("lea 8(%%rbp), %%rdi\n\t"
        "pushq %%rdi\n\t"
        "call exec_switch_to\n\t"
        "leave\n\t"
        "ret\n\t"
        :::);
}

void schedule(void){
    int i, c;
    struct task_struct ** p;
    for(p = &LAST_TASK ; p > &FIRST_TASK ; --p){
        if(*p){
            if((*p)->timeout && (*p)->timeout < jiffies){
                (*p)->timeout = 0;
                if((*p)->state == TASK_INTERRUPTIBLE){
                    (*p)->state = TASK_RUNNING;
                }
            }
        }
        // SIGALRM的操作是终止进程
        if((*p)->alarm && (*p)->alarm < jiffies){
            (*p)->signal |= (1<<(SIGALRM-1));
            (*p)->alarm = 0;
        }
        if(((*p)->signal & ~(_BLOCKABLE & (*p)->blocked)) && (*p)->state == TASK_INTERRUPTIBLE)
            (*p)->state = TASK_RUNNING;
    }
    while(1){
        c =-1;
        next = 0;
        i = NR_TASKS;
        p = &task[NR_TASKS];
        while(--i){
            if(!*--p)
                continue;
            if((*p)->state == TASK_RUNNING && (*p)->counter > c){
                c = (*p)->counter;
                next = i;
            }
        }
        if(c) break;
        for(p = &LAST_TASK ; p > &FIRST_TASK; --p){
            if(*p){
                (*p)->counter = ((*p)->counter >> 1) + (*p)->priority;
            }
        }
    }
    switch_to();
}

//pause系统调用，转换当前任务位可终端等待状态
int sys_pause(void){
    current->state = TASK_INTERRUPTIBLE;
    schedule();
    return 0;
}

//进程睡眠但是
static inline void __sleep_on(struct task_struct** p, int state){
    struct task_struct * tmp;
    if(!p){
        return;
    }
    if(current == &(init_task.task)){
        printf("不可能为0号进程");
        return;
    }
    tmp = *p;
    *p = current;
    current->state = state;
    schedule();
}


//唤醒可中断等待状态和不可中断等待状态
void wake_up(struct task_struct **p){
    if(p && *p){
        if((**p).state == TASK_STOPPED){
            printf("唤醒停止状态的进程，错误\n");
            return;
        }
        if((**p).state == TASK_ZOMBIE){
            printf("唤醒僵尸状态的进程,错误\n");
            return;
        }
        (**p).state = 0;
    }
}

//可中断等待
void interruptible_sleep_on(struct task_struct **p){
    __sleep_on(p, TASK_INTERRUPTIBLE);
}

//不可中断等待
void sleep_on(struct task_struct **p){
    __sleep_on(p, TASK_UNINTERRUPTIBLE);
}


int sys_getpid(void){
    return current->pid;
}

int sys_getppid(void){
    return current->p_pptr->pid;
}

int sys_getuid(void){
    return current->uid;
}

int sys_geteuid(void){
    return current->euid;
}

int sys_getgid(void){
    return current->gid;
}

int sys_getegid(void){
    return current->egid;
}

void first_proc(){
    printf("0号进程初始化成功\n");
    checkuser();
    while(1){
        char cmd[50];
        printf("%s$ ", loginuser);
        scanf("%s", cmd);
        if(!strcmp(cmd, "ls")){
            list();
            continue;
        }
        if(!strcmp(cmd, "mkdir")){
            mkdir();
            continue;
        }
        if(!strcmp(cmd, "touch")){
            touch();
            continue;
        }
        if(!strcmp(cmd, "exit")){
            return 0;
            continue;
        }
        if(!strcmp(cmd, "write")){
            shell_write();
            continue;
        }
        if(!strcmp(cmd, "read")){
            shell_read();
            continue;
        }
        if(!strcmp(cmd, "ln")){
            ln();
            continue;
        }
        if(!strcmp(cmd, "cd")){
            cd();
            continue;
        }
        if(!strcmp(cmd, "rm")){
            rm();
            continue;
        }
        if(!strcmp(cmd, "rmdir")){
            rmdir();
            continue;
        }
        if(!strcmp(cmd, "help")){
            help();
            continue;
        }
        //新增
        if(cmd[0] == '.' && cmd[1] == '/'){
            char buff[50];
            memset(buff, 0, sizeof(buff));
            int fd;
            fd = hsc_open(cmd+2, O_RDWR, 0777);
            lseek(fd, 0, SEEK_SET);
            read(fd, buff, 50);
            memset(cmd, 0, sizeof(cmd));
            if(buff[0] == '#' && buff[1] == '!'){
                //#!shell/touch text/ls
                int i = 8, j = 0;
                while(buff[i]){
                    if(buff[i] != '/'){
                        cmd[j] = buff[i];
                        j++;
                        i++;
                        continue;
                    }
                    execcmd(cmd);
                    memset(cmd, 0, sizeof(cmd));
                    j = 0;
                    i++;
                }
            }else{
                printf("该文件不可执行\n");
            }
        } 
        else{
            printf("commond not found : %s\n", cmd);
            printf("you can try help for help\n");
        }
    }
}

//调度程序初始化
void sched_init(void){
    int i;
    unsigned long long * p;
    unsigned long long * base_gdt = (unsigned long long *)RAM + GDTR;
    *(base_gdt + FIRST_TSS_ENTRY) = &init_task.task.tss;
    *(base_gdt + FIRST_LDT_ENTRY) = &init_task.task.ldt;
    p = base_gdt + 2 + FIRST_TSS_ENTRY;
    for(i = 1; i<NR_TASKS; i++){
        task[i] = NULL;
        p = 0;
        p++;
        p = 0;
    }
    task[0] = current;
    current->pwd = current->root;
    task[0]->tss.entry = (unsigned long long *) &first_proc;
    switch_to();
}