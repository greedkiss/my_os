#ifndef _TIMER_H
#define _TIMER_H


#include <unistd.h>
#include <sched.h>

void do_timer(){
    while(1){
        usleep(10000);
        //进程调度
        schedule();
    }
}

#endif // !1