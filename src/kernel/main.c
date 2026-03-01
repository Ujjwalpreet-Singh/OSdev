#include "stdio.h"
#include <stdint.h>
#include "bootinfo.h"
#include "framebuffer.h"
#include "psf.h"
#include "printoguri.h"

extern unsigned char font_psf[];
void kmain(uint16_t bootDrive, BootInfo* bootInfo)
{
    framebuffer_init(bootInfo);
    psf_init(font_psf);

    clear(0xF);
    print("Second stage kernel running\n");
    oguriprint();

}