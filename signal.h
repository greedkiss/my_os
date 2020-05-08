#ifndef _SIGNAL_H
#define _SIGNAL_H

#include "sched.h"

//挂断控制终端或进程
#define SIGHUB 1
//键盘中断
#define SIGINT 2
//强迫进程终止
#define SIGKILL 9
//定时器报警
#define SIGALRM 14
//栈出错
#define SIGSTAKFLT 16
//停止进程
#define SIGSTOP 19

//默认信号
#define SIG_DFL ((void (*) (int))0)

//忽略信号
#define SIG_IGN ((void (*) (int))1)

//信号处理返回错误
#define SIG_ERR ((void (*) (int))-1)



#endif // !1