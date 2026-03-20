#include "tty.h"
#include "editor.h"
#include "../drivers/initalize.h"
#define EDIT_BUFFER 8192

static char buffer[EDIT_BUFFER];
static int length = 0;

void editor(char* arg)
{
    length = 0;
    for(int i = 0; i < EDIT_BUFFER; i++)
        buffer[i] = 0;
    printf("----EDIOTR!----\n");
    printf(":s to save,:q to quit\n\n");

    while(1){
        char c = tty_getchar();

        if (c == ':'){
            char cmd = tty_getchar();
            if(cmd == 'q')
            {
                printf("\nQuit\n");
                return;
            } else if (cmd == 's'){
                if(length == 0)
                {
                    printf("\nNothing to save\n");
                }
                else if(fat16_create_file(arg, (uint8_t*)buffer, length))
                {
                    printf("\nSaved\n");
                }
                else
                {
                    printf("\nSave failed\n");
                }

                return;
            } else {
                printf("INVALID!!");
            }
        }
        else if(c == '\b')
        {
            if (length > 0)
            {
                length--;
            }
        }
        else
        {
            if(length<EDIT_BUFFER-1)
            {
                buffer[length++] = c;
            }
        }
    }
}

