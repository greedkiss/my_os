#ifndef _SCHED_H
#define _SCHED_H

    #include "head.h"
    #include "signal.h"
    #include "fs.h"
    
    #ifndef PAGE_SIZE
    #define PAGE_SIZE 4096 
    
    #endif // !1
    //寄存器
    unsigned int TR;
    //系统中最多进程数,总进程数×每个任务长度=4GB
    #define NR_TASKS 64
    //每个任务长度逻辑地址 64MB
    #define TASK_SIZE 0x04000000
    //动态加载库长度
    #define LIBRARY_SIZE 0x00400000
    //进程逻辑地址空间中，动态库被加载的位置
    #define LIBRARY_OFFSET (TASK_SIZE - LIBRARY_SIZE)

    extern struct task_struct * task[NR_TASKS];

    #define FIRST_TASK task[0]

    #define LAST_TASK task[NR_TASKS - 1]

    #define TASK_RUNNING 0 // 进程正在运行或者已经准备就绪
    #define TASK_INTERRUPTIBLE 1 //可中断等待状态
    #define TASK_UNINTERRUPTIBLE 2//不可中断等待状态
    #define TASK_ZOMBIE 3//僵死进程
    #define TASK_STOPPED 4 //进程停止

    #ifndef NULL
        #define NULL ((void *) 0)
    #endif
    // extern void schedule(void);

    extern void schedule(void);

    typedef int (*fn_ptr)();

    //本系统不考虑协处理器,存储各寄存器值
    struct tss_struct {
        unsigned int ldt;
        unsigned long long * entry;
    };
    // struct tss_struct {
    //     int back_link;
    //     int esp0;
    //     int ss0;
    //     int esp1;
    //     int ss1;
    //     int esp2;
    //     int ss2;
    //     int cr3;
    //     int eip;
    //     int eflags;
    //     int eax, ecx, edx, ebx;
    //     int esp;
    //     int ebp;
    //     int esi;
    //     int edi;
    //     int es;
    //     int cs;
    //     int ss;
    //     int ds;
    //     int fs;
    //     int gs;
    //     int ldt;
    //     int trace_bitmap;
    // };
    struct my_sigaction{
        void (*sa_handler) (int);
        unsigned int sa_mask;
        int sa_flags;
        void (*sa_restorer) (void);
    };

    //pcb
    struct task_struct{
        int state; //运行状态
        int counter; //运行计数
        int priority; //进程优先级
        int signal; //信号32位
        struct my_sigaction sigaction[32]; //信号入口
        int blocked;//信号屏蔽码

        int exit_code; //退出码,由父进程获取
        unsigned int start_code, end_code, end_data, brk, start_stack;//逻辑地址空间
        int pid, pgrp, session, leader; //进程id, 组id， 会话id, 首领进程id
        // int groups[NGROUP]; //一个进程可以属于多个组，进程所属组号

        struct task_struct *p_pptr, *p_cptr, *p_ysptr, *p_osptr;//父进程、最新子进程、比自己后创建的相邻进程、比自己早创建的相邻进程指针

        unsigned short uid, euid, suid;//用户Id, 有效的用户id， 保存的用户id
        unsigned short gid, egid, sgid;//组id ， 有效组id, 保存的组id
        unsigned int timeout, alarm;//内核定时超时、报警定时值
        // int utime, stime, cutime, cstime, start_time;//用户态运行时间， 内核态运行时间， 进程开始运行时间
        // struct rlimit rlim[RLIM_NLIMITS];//进程资源使用统计数组
        unsigned int flag;//各个进程的标识

        //文件系统部分先注释调试
        // int tty;//进程使用的tty终端子设备号
        unsigned short umask;
        struct m_inode * pwd;
        struct m_inode * root;
        struct m_inode * executable;
        struct m_inode * library;
        unsigned int close_on_exec;
        struct file * filp[NR_OPEN];

        unsigned int ldt[3];

        struct tss_struct tss;
    };

    //手动创建任务0
    // #define INIT_TASK \
    // { 0, 15, 15, \
    // 0, {{},}, 0, \
    // 0,0,0,0,0,0,\
    // 0,0,0,0,\
    // {NOGROUP, },\
    // &init_task.task,0,0,0,\
    // 0,0,0,0,0,0,\
    // 0,0,0,0,0,0,0,\
    // {{0x7fffffff, 0x7fffffff},{0x7fffffff, 0x7fffffff},\
    // {0x7fffffff, 0x7fffffff}, {0x7fffffff, 0x7fffffff}\
    // {0x7fffffff, 0x7fffffff},{0x7fffffff, 0x7fffffff}},\
    // 0,\
    // -1, 0022, NULL, NULL, NULL, NULL, 0,\
    // {NULL,},\
    // {\
    //     {0,0},\
    //     {0x9f, 0xc0fa00},\
    //     {0x9f, 0xc0f200},\
    // },\
    // {0, PAGE_SIZE + (int)&init_task, 0x10, 0,0,0,0,(int)&pg_dir,\
    // 0,0,0,0,0,0,0,0,\
    // 0,0,0x17,0x17,0x17,0x17,0x17,0x17,\
    // _LDT(0),0x80000000,\
    // {}\
    // },\
    // }

    #define INIT_TASK \
    {0, 15, 15,\
    0,{{},},0,\
    0,0,0,0,0,0,\
    0,0,0,0,\
    &init_task.task,0,0,0,\
    0,0,0,0,0,0,\
    0,0,\
 /*flag*/   0,\
    0022,NULL,NULL,NULL,NULL,0,\
    {NULL,},\
    {0,0,0},\
    {_LDT(0), 0}},

    extern struct task_struct *current;
    extern unsigned int jiffies;
    extern unsigned int startup_time;
    extern int jiffies_offset;

    //全局表地址 null, cs, ds, systemcall, tss0, ldt0,tss1, ldt1
    #define FIRST_TSS_ENTRY 4
    #define FIRST_LDT_ENTRY (FIRST_TSS_ENTRY + 1)
    #define _TSS(n) ((((unsigned int )n) << 4) + (FIRST_TSS_ENTRY) << 3 )
    #define _LDT(n) ((((unsigned int )n) << 4) + (FIRST_LDT_ENTRY) << 3 )

    //把第n个任务的tss加载到TR
    // #define ltr(n) (TR = (unsigned int)(* ))


    //task任务数组
    extern struct task_struct * task[NR_TASKS];

    extern void sched_init(void);

    extern void switch_to();




#endif