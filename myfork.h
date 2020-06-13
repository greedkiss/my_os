#ifndef _MYFORK_H
#define _MYFORK_H

#include "sched.h"
#include "memory.h"
#include "head.h"

extern int last_pid;

extern int copy_process();

extern int find_empty_process();


#endif 