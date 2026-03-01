#pragma once
#include <stdint.h>

void PIT_init(uint32_t freq);
void sleep(uint32_t ms);

extern volatile uint32_t g_ticks;