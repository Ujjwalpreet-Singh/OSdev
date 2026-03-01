#include "framebuffer.h"
#include "bootinfo.h"
#include <stdint.h>

static uint32_t* fb;
static uint32_t pitch;   // pixels per row
static uint32_t width;
static uint32_t height;

static BootInfo* bootinfo;

void framebuffer_init(BootInfo* info)
{
    bootinfo = info;

    fb = (uint32_t*)info->framebuffer;
    pitch = info->pitch/4;
    width = info->width;
    height = info->height;
}



void putpixel(int x,int y,uint32_t color)
{
    fb[y*pitch + x] = color;
}

void clear(uint32_t color)
{
    for(int y=0;y<height;y++)
        for(int x=0;x<width;x++)
            fb[y*pitch+x]=color;
}

void scroll(int font_height)
{
    int scroll_rows = font_height;

    /* move screen up */
    for(int y=0; y<height-scroll_rows; y++)
    {
        for(int x=0; x<width; x++)
        {
            fb[y*pitch+x] =
            fb[(y+scroll_rows)*pitch+x];
        }
    }

    /* clear bottom */
    for(int y=height-scroll_rows; y<height; y++)
    {
        for(int x=0; x<width; x++)
        {
            fb[y*pitch+x] = 0x000000;
        }
    }
}

uint32_t fb_get_width(){ return width; }
uint32_t fb_get_height(){ return height; }