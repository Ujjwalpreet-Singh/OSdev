#include "stdint.h"
#include "stdio.h"

void check_protected_mode(void);

void __attribute__((cdecl)) start(uint16_t bootDrive)
{
    clrscr();
    if (sizeof(void*) == 4)
        puts("Pointer = 4 bytes");
    puts("We're running in 32-bit now");

    while (1);
}
