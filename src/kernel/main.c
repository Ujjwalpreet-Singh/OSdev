#include <stdint.h>
#include "input-output/bootinfo.h"
#include "drivers/initalize.h"
#include "input-output/printoguri.h"

extern unsigned char font_psf[];
void kmain(uint16_t bootDrive, BootInfo* bootInfo)
{
    ALL_INIT(bootDrive,bootInfo);
    clear(0xF);
    printf("BootInfo height: %u\n", bootInfo->height);
    printf("BootInfo width: %u\n", bootInfo->width);
    printf("Pitch: %u\n", bootInfo->pitch);
    print("Second stage kernel running\n");
    oguriprint();
    
    fat16_read_root();
    print("reading into fat");
    if (fat16_read_fat()){
        print("fat loaded!");
    } else {
        print("fat not loaded");
    }

    fat16_list();

        FAT16_DirEntry file;

    if(fat16_find_path("KERNEL.BIN", &file)){
        print("KERNEL.BIN FOUND!");
    }
    if(fat16_find_path("USR/TEST.TXT", &file)){
        print("TEST.TXT FOUND!");
    }
}

