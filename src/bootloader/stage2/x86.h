#pragma once
#include "stdint.h"

void __attribute__((cdecl)) x86_Video_WriteCharTeletype(char c, uint8_t page);
void __attribute__((cdecl)) x86_Video_ClearScreen(void);
