#include "keyboard.h"
#include "../drivers/io.h"
#include "../drivers/isr.h"
#include "tty.h"
#include <stdint.h>

static char scancode_table[128] =
{
0,27,'1','2','3','4','5','6','7','8','9','0','-','=', '\b',
'\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
0,'a','s','d','f','g','h','j','k','l',';','\'','`',
0,'\\','z','x','c','v','b','n','m',',','.','/',
0,'*',
0,' ',   // <-- THIS IS SPACEBAR (scancode 0x39)
};
static char scancode_shift[128] =
{
0,27,'!','@','#','$','%','^','&','*','(',')','_','+', '\b',
'\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
0,'A','S','D','F','G','H','J','K','L',':','"','~',
0,'|','Z','X','C','V','B','N','M','<','>','?',
0,'*',
0,' ',
};
static int caps_on = 0;
static int shift_pressed = 0;
static void keyboard_handler(Registers* regs)
{
    (void)regs;

    uint8_t scancode = inb(0x60);

    if(scancode == 0x3A)   // caps lock press
    {
        caps_on = !caps_on;
    }
        // Left Shift
    if(scancode == 0x2A)
    {
        shift_pressed = 1;
    }
    if(scancode == 0xAA)
    {
        shift_pressed = 0;
    }

    // Right Shift
    if(scancode == 0x36)
    {
        shift_pressed = 1;
    }
    if(scancode == 0xB6)
    {
        shift_pressed = 0;
    }
    if(scancode < 128)
    {
        char c;
        if(shift_pressed)
            c = scancode_shift[scancode];
        else
            c = scancode_table[scancode];

        if(caps_on && c >= 'a' && c <= 'z')
        {
            c = c - 32;  // convert to uppercase
        }


        if(c)
            tty_keyboard_input(c);   // print directly to screen
    }

    outb(0x20,0x20); // EOI
}

void keyboard_init()
{
    ISR_RegisterHandler(33, keyboard_handler);
}