#include <stdint.h>
#include "input-output/bootinfo.h"
#include "input-output/framebuffer.h"
#include "input-output/psf.h"
#include "input-output/printoguri.h"

extern unsigned char font_psf[];
void kmain(uint16_t bootDrive, BootInfo* bootInfo)
{
    framebuffer_init(bootInfo);
    psf_init(font_psf);

    clear(0xF);
    printf("BootInfo height: %u\n", bootInfo->height);
    printf("BootInfo width: %u\n", bootInfo->width);
    printf("Pitch: %u\n", bootInfo->pitch);
    print("Second stage kernel running\n");
    oguriprint();
    

}