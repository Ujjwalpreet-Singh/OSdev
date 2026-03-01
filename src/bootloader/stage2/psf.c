#include "psf.h"
#include "framebuffer.h"
#include <stdarg.h>
#include <stdint.h>

extern unsigned char font_psf[];

static uint8_t* glyphs;
static uint32_t font_width = 8;
static uint32_t font_height = 16;


void psf_init(void* font)
{
    glyphs = (uint8_t*)font + 32; // skip PSF2 header
}

static uint8_t* get_glyph(char c)
{
    return glyphs + (c * font_height);
}

void draw_char(int x, int y, char c, uint32_t color)
{
    uint8_t* glyph = get_glyph(c);

    for(int row=0; row<font_height; row++)
    {
        for(int col=0; col<font_width; col++)
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
    while (*s)
    {
        if (*s == '\n')
        {
            cursor_x = 0;
            cursor_y += font_height;
        }
        else
        {
            // wrap horizontally first
            if (cursor_x + font_width > fb_get_width())
            {
                cursor_x = 0;
                cursor_y += font_height;
            }

            // scroll BEFORE drawing if needed
            if (cursor_y + font_height > fb_get_height())
            {
                scroll(font_height);
                cursor_y -= font_height;
            }

            draw_char(cursor_x, cursor_y, *s, 0xFFFFFFFF);
            cursor_x += font_width;
        }

        s++;
    }
}


static void print_char(char c)
{
    char s[2];
    s[0] = c;
    s[1] = 0;
    print(s);
}

static void print_string(const char* s)
{
    if (!s) s = "(null)";
    print(s);
}

static void print_uint(uint32_t value)
{
    char buf[11];
    int i = 0;

    if (value == 0)
    {
        print_char('0');
        return;
    }

    while (value)
    {
        buf[i++] = '0' + (value % 10);
        value /= 10;
    }

    while (i--)
        print_char(buf[i]);
}

static void print_int(int32_t value)
{
    if (value < 0)
    {
        print_char('-');
        value = -value;
    }
    print_uint((uint32_t)value);
}

static void print_hex(uint32_t value)
{
    char buf[8];
    int i = 0;

    if (value == 0)
    {
        print_char('0');
        return;
    }

    while (value)
    {
        uint32_t digit = value & 0xF;
        buf[i++] = digit < 10 ? '0' + digit : 'A' + digit - 10;
        value >>= 4;
    }

    print_string("0x");

    while (i--)
        print_char(buf[i]);
}

void printfv(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    while (*fmt)
    {
        if (*fmt == '%')
        {
            fmt++;

            switch (*fmt)
            {
                case 'd':
                    print_int(va_arg(args, int32_t));
                    break;

                case 'u':
                    print_uint(va_arg(args, uint32_t));
                    break;

                case 'x':
                    print_hex(va_arg(args, uint32_t));
                    break;

                case 'c':
                    print_char((char)va_arg(args, int));
                    break;

                case 's':
                    print_string(va_arg(args, const char*));
                    break;

                case '%':
                    print_char('%');
                    break;

                default:
                    print_char('%');
                    print_char(*fmt);
                    break;
            }
        }
        else
        {
            print_char(*fmt);
        }

        fmt++;
    }

    va_end(args);
}