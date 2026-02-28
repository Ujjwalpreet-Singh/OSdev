#pragma once
#include <stdint.h>
#include "bootinfo.h"

void framebuffer_init(BootInfo* info);
void putpixel(int x,int y,uint32_t color);
void clear(uint32_t color);
void scroll(int font_height);

uint32_t fb_get_width();
uint32_t fb_get_height();

uint32_t make_color(uint8_t r,uint8_t g,uint8_t b);