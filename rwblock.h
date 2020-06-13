#ifndef _RWBLOCK_H
#define _RWBLOCK_H

#include "init.h"
#include <stdio.h>
#include "swap.h"

//请求项
struct request{
    int cmd;
    int sector;
    struct buffer_head * bh;
};

//

extern void ll_read_block(struct buffer_head* bh);

extern void ll_write_block(struct buffer_head* bh);

extern void read_swap_page(int nr, char * buffer);

extern void write_swap_page(int nr, char * buffer);

#endif 