#ifndef _RWBLOCK_H
#define _RWBLOCK_H

#include "init.h"
#include <stdio.h>
#include "swap.h"

extern void ll_read_block(struct buffer_head* bh);

extern void ll_write_block(struct buffer_head* bh);
#endif 