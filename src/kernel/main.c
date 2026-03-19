#include <stdint.h>
#include "input-output/bootinfo.h"
#include "drivers/initalize.h"
#include "input-output/printoguri.h"
#include "input-output/shell.h"

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
    print("\nreading into fat");
    if (fat16_read_fat()){
        print("\nfat loaded!\n");
    } else {
        print("\nfat not loaded\n");
    }


    print("Booting up shell...");
    sleep(2500);
    clear(0xF);
    shell();

}

