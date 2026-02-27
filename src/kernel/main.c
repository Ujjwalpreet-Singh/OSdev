#include "stdio.h"



void __attribute__((section(".entry"))) kmain(void)
{
    

    clrscr();
    puts("Kernel started\n");

    while(1);
}