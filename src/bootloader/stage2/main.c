#include "stdint.h"
#include "stdio.h"
#include "idt.h"
#include "gdt.h"
#include "isr.h"

void check_protected_mode(void);
void __attribute__((cdecl)) start(uint16_t bootDrive)
{
    ISR_Initialize();
    GDT_Initialize();
    IDT_Initialize();
    clrscr();
    if (sizeof(void*) == 4)
        puts("Pointer = 4 bytes\n");
    puts("We're running in 32-bit now\n");
    check_protected_mode();

    while (1);
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
