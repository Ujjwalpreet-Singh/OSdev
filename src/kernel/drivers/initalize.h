#pragma once
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
#include <stdint.h>


// INIT
void ALL_INIT(uint16_t bootDrive, BootInfo* bootInfo);


//GDT
void GDT_Initialize();

//IDT
void IDT_Initialize();
void IDT_DisableGate(int interrupt);
void IDT_EnableGate(int interrupt);
void IDT_SetGate(int interrupt, void* base, uint16_t segmentDescriptor, uint8_t flags);

//ISR
void ISR_Initialize();
void ISR_RegisterHandler(int interrupt, ISRHandler handler);


//FRAMEBUFFER
void framebuffer_init(BootInfo* info);
void putpixel(int x,int y,uint32_t color);
void clear(uint32_t color);
void scroll(int font_height);

uint32_t fb_get_width();
uint32_t fb_get_height();

uint32_t make_color(uint8_t r,uint8_t g,uint8_t b);

//PIC
void PIC_Remap();


//TIMER
void PIT_init(uint32_t freq);
void sleep(uint32_t ms);


// IO
void psf_init(void* font);
void draw_char(int x, int y, char c, uint32_t color);
void print(const char* s);
void print_char(char c);
void printf(const char* fmt, ...);
void print_buffer(const char* label, const void* buffer, uint32_t size);
void print_hex(uint32_t value);
void print_int(int32_t value);
void print_uint(uint32_t value);
void print_string(const char* s);