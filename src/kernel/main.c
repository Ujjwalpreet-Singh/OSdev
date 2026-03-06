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
    print("reading into fat");
    if (fat16_read_fat()){
        print("fat loaded!");
    } else {
        print("fat not loaded");
    }

    fat16_list();

    FAT16_DirEntry file;


    if(fat16_find_path("HOME/USR/TEST.TXT", &file)){
        print("TEST.TXT FOUND!");
        fat16_read_file(&file,(void*)0x800000);

        char* data = (char*)0x800000;

        for(uint32_t i = 0; i < file.file_size; i++)
        {
            print_char(data[i]);
        }
    }
    print("shell is alive..");
    shell();

}

