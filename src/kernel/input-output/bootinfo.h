#pragma once
#include <stdint.h>

typedef struct
{
    uint32_t framebuffer;
    uint16_t pitch;
    uint16_t width;
    uint16_t height;
    uint8_t  bpp;
} BootInfo;