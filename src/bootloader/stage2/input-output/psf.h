#pragma once
#include <stdint.h>

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