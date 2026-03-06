#include "tty.h"
#include "psf.h"

static TTY tty;

void tty_init()
{
    tty.head = 0;
    tty.tail = 0;
}

void tty_keyboard_input(char c)
{

    uint32_t next = (tty.head + 1) % TTY_BUFFER_SIZE;

    if(next != tty.tail) // buffer not full
    {
        tty.buffer[tty.head] = c;
        tty.head = next;
    }

    // echo character
    tty_putchar(c);
}

void tty_putchar(char c)
{
    print_char(c); // your framebuffer/VGA putchar
}

char tty_getchar()
{
    while(tty.head == tty.tail)
        __asm__("hlt");

    char c = tty.buffer[tty.tail];
    tty.tail = (tty.tail + 1) % TTY_BUFFER_SIZE;

    return c;
}