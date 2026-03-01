#include "timer.h"
#include "io.h"
#include "isr.h"

#define PIT_BASE_FREQ 1193182

volatile uint32_t g_ticks = 0;

static void PIT_Handler(Registers* regs)
{
    (void)regs;

    g_ticks++;

    //send EOI to PIC
    outb(0x20,0x20);
}

void PIT_init(uint32_t freq)
{
    uint32_t divisor = PIT_BASE_FREQ/freq;

    outb(0x42,0x36);
    
    outb(0x40,divisor & 0xFF);
    outb(0x40,divisor >> 8);

    ISR_RegisterHandler(32,PIT_Handler);
}

void sleep(uint32_t ms)
{
    uint32_t target = g_ticks + ms;

    while ((int32_t)(target-g_ticks) > 0)
        __asm__("hlt");
}