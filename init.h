#ifndef _INIT_H
#define _INIT_H
#include "fs.h"

#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>




#define DISK1 "/home/flipped/code/my_os/0ext2"
#define DISK2 "/home/flipped/code/my_os/1ext2"

extern void initFileSys(void);

#endif 