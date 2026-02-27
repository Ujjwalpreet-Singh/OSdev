#include "stdint.h"
#include "stdio.h"
#include "idt.h"
#include "gdt.h"
#include "isr.h"
#include "ata.h"
#include "fat.h"
#include "memdefs.h"
#include "jump.h"

#define KERNEL_ADDR ((void*)0x500000)

#define TXT_ADDR ((uint8_t*)0x90000)

#define KERNEL_STACK_TOP 0x200000

typedef void (*KernelStart)(void);

void check_protected_mode(void);
void jump_to_kernel(uint32_t entry, uint32_t stack);
void __attribute__((cdecl)) start(uint16_t bootDrive)
{
    ISR_Initialize();
    GDT_Initialize();
    IDT_Initialize();
    ata_init(bootDrive);
    clrscr();
    if (sizeof(void*) == 4)
        puts("Pointer = 4 bytes\n");
    puts("We're running in 32-bit now\n");
    check_protected_mode();

    uint8_t buffer[512];

    ata_read28(0, 1, buffer);

    printf("OEM: ");

    for (int i = 3; i < 11; i++)
        putc(buffer[i]);

    printf("\n");

        if(!fat16_init())
    {
        printf("FAT init failed\n");
        while(1);
    }

    printf("FAT ready\n");

    fat16_read_root();
    puts("reading into fat");
    if (fat16_read_fat()){
        puts("fat loaded!");
    } else {
        puts("fat not loaded");
    }

    fat16_list();



    uint16_t ss;
    __asm__("mov %%ss, %0" : "=r"(ss));
    printf("SS=%x\n", ss);
    FAT16_DirEntry file;

    if(fat16_find("KERNEL.BIN", &file))
    {
        fat16_read_file(&file, KERNEL_ADDR);

        print_buffer("kernel:", KERNEL_ADDR, 32);

        KernelStart kernelStart = (KernelStart)KERNEL_ADDR;
        kernelStart();
    
    };



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
