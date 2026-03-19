#include "framebuffer.h"
#include "bootinfo.h"
#include <stdint.h>

static uint32_t* fb;
static uint32_t pitch;   // pixels per row
static uint32_t width;
static uint32_t height;

#define FB_SIZE (1920*1080)

static uint32_t framebuffer_backup[FB_SIZE];

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

void clear_driver(uint32_t color)
{
    for(int y=0;y<height;y++)
        for(int x=0;x<width;x++)
            fb[y*pitch+x]=color;
}

void scroll(int font_height,uint32_t bg)
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
            fb[y*pitch+x] = bg;
        }
    }
}

uint32_t fb_get_width(){ return width; }
uint32_t fb_get_height(){ return height; }

void fb_save()
{
    uint32_t total = pitch * height;

    for(uint32_t i = 0; i < total; i++)
        framebuffer_backup[i] = fb[i];
}

void fb_restore()
{
    uint32_t total = pitch * height;

    for(uint32_t i = 0; i < total; i++)
        fb[i] = framebuffer_backup[i];
}