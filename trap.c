#include "trap.h"

void divide_error(){
    printf("除0中断\n");
}

void do_debug(){
    printf("debug中断\n");
}

void do_nmi(){
    printf("nmi中断\n");
}

void overflow(){
    printf("溢出\n");
}

void trap_init(){
    unsigned long long * base_idt =(unsigned long long*)RAM + IDTR;
    *(base_idt) = (unsigned long long)(&divide_error);
    *(base_idt + 1) = (unsigned long long)(&do_debug);
    *(base_idt + 2) = (unsigned long long)(&do_nmi);
    *(base_idt + 4) = (unsigned long long)(&overflow);
}







