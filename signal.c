#include "signal.h"

//获取信号屏蔽位图
int sys_sgetmask(){
    return current->blocked;
}

//设置新的信号屏蔽位图
int sys_setmask(int newmask){
    int old = current->blocked;
    current->blocked = newmask & ~(1<<(SIGKILL-1)) & ~(1<<(SIGSTOP-1));
    return old;
}

