#include "initalize.h"
#include "idt.h"
#include "gdt.h"
#include "isr.h"
#include "../disk/ata.h"
#include "../disk/fat.h"
#include "../input-output/bootinfo.h"
#include "../input-output/framebuffer.h"
#include "../input-output/psf.h"
#include "../input-output/timer.h"
#include "../input-output/pic.h"
#include "../input-output/keyboard.h"

extern unsigned char font_psf[];

void ALL_INIT(uint16_t bootDrive, BootInfo* bootInfo){
    psf_init(font_psf);
    framebuffer_init(bootInfo);
    clear(0xF);
    ISR_Initialize();
    GDT_Initialize();
    IDT_Initialize();
    ata_init(bootDrive);
    PIC_Remap();
    PIT_init(1000);
    keyboard_init();

    __asm__("sti");
    
    if(!fat16_init())
    {
        printf("FAT init failed\n");
        while(1);
    }


}

