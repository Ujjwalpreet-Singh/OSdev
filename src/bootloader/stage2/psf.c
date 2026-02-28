#include "psf.h"
#include "framebuffer.h"

extern unsigned char font_psf[];

static uint8_t* glyphs;
static uint32_t width = 8;
static uint32_t height = 16;


void psf_init(void* font)
{
    glyphs = (uint8_t*)font + 32; // skip PSF2 header
}

static uint8_t* get_glyph(char c)
{
    return glyphs + (c * height);
}

void draw_char(int x, int y, char c, uint32_t color)
{
    uint8_t* glyph = get_glyph(c);

    for(int row=0; row<height; row++)
    {
        for(int col=0; col<width; col++)
        {
            if(glyph[row] & (0x80 >> col))
                putpixel(x+col, y+row, color);
        }
    }
}

static int cursor_x = 0;
static int cursor_y = 0;

void print(const char* s)
{
    while(*s)
    {
        if(*s == '\n')
        {
            cursor_x = 0;
            cursor_y += height;
        }
        else
        {
            draw_char(cursor_x, cursor_y, *s, 0xFFFFFFFF);
            cursor_x += width;
        }
        s++;
    }
}