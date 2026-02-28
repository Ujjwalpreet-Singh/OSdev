#pragma once
#include <stdint.h>

void psf_init(void* font);
void draw_char(int x, int y, char c, uint32_t color);
void print(const char* s);
void print_hex(uint32_t value);
void print_int(int value);