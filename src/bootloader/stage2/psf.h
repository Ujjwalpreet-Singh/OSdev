#pragma once
#include <stdint.h>

void psf_init(void* font);
void draw_char(int x, int y, char c, uint32_t color);
void print(const char* s);
static void print_char(char c);
void printfv(const char* fmt, ...);