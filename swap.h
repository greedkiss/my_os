#ifndef _SWAP_H
#define _SWAP_H

#include "memory.h"

extern void swap_free(int swap_nr);

extern void swap_in(unsigned int* table_ptr);

extern int try_to_swap_out(unsigned int * table_ptr);

extern int swap_out(void);

extern void init_swapping(void);
#endif