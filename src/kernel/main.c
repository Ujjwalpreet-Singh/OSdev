#include "stdio.h"
#include <stdint.h>

__attribute__((naked, section(".entry")))
void kmain(void)
{
    clrscr();
    puts("Kernel started\n");

    printf("hello");


    while (1);
}