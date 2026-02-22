#include "stdint.h"
#include "stdio.h"

void check_protected_mode(void);

void __attribute__((cdecl)) start(uint16_t bootDrive)
{
    clrscr();
    if (sizeof(void*) == 4)
        puts("Pointer = 4 bytes\n");
    puts("We're running in 32-bit now");

    check_protected_mode();
}
uint32_t read_cr0()
{
    uint32_t value;
    __asm__ volatile ("mov %%cr0, %0" : "=r"(value));
    return value;
}
void check_protected_mode()
{
    if (read_cr0() & 1)
        puts("Protected Mode ON\n");
    else
        puts("Still in Real Mode\n");
}