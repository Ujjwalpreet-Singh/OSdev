#include "../drivers/io.h"
#include "pic.h"
#include <stdint.h>

void PIC_Remap()
{
    uint8_t mask1 = inb(0x21);
    uint8_t mask2 = inb(0xA1);

    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    outb(0x21, 0x20); // master offset = 32
    outb(0xA1, 0x28); // slave offset = 40

    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    // unmask IRQ0 (timer)
    outb(0x21, mask1 & ~1);
    outb(0xA1, mask2);
}