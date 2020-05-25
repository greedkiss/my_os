#include "myRAM.h"

char* RAM;
void getmyRAM(){
    RAM = (char*)malloc(16*1024*1024);
}
