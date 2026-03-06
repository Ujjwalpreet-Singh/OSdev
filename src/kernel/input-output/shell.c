#include "psf.h"
#include "tty.h"
#include "execute.h"
#define CMD_BUF 256

char cmd[CMD_BUF];
int index = 0;

void shell()
{
    printf("\n> ");

    while(1)
    {
        char c = tty_getchar();

        if(c == '\n')
        {
            cmd[index] = 0;

            execute_command(cmd);

            index = 0;
            printf("\n> ");
        }

        else if(c == '\b')
        {
            if(index > 0)
                index--;
        }

        else
        {
            if(index < CMD_BUF - 1)
                cmd[index++] = c;
        }
    }
}