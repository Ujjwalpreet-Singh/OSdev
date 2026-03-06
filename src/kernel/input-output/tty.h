#ifndef TTY_H
#define TTY_H

#include <stdint.h>

#define TTY_BUFFER_SIZE 256

typedef struct
{
    char buffer[TTY_BUFFER_SIZE];
    volatile uint32_t head;
    volatile uint32_t tail;
} TTY;

void tty_init();
void tty_putchar(char c);
char tty_getchar();
void tty_keyboard_input(char c);

#endif