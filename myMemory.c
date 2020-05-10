#include "myMemory.h"

char* b_start;
char* b_end;

//1mb~4mb
void getBuffer(){
    b_start = RAM + 1*1024*1024;
    b_end = b_start + 3*1024*1024;
}