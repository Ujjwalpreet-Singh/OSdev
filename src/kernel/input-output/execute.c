#include <stdint.h>
#include "psf.h"
#include "../drivers/initalize.h"

int strncmp(const char* a, const char* b, uint32_t n)
{
    for(uint32_t i = 0; i < n; i++)
    {
        if(a[i] != b[i])
            return 1;

        if(a[i] == '\0')
            return 0;
    }

    return 0;
}

int strcmp(const char* a, const char* b)
{
    while(*a && *b)
    {
        if(*a != *b)
            return 1;

        a++;
        b++;
    }

    return (*a != *b);
}

void execute_command(char* cmd)
{
    if(strcmp(cmd, "help") == 0)
    {
        print("commands: help echo clear ls cat");
    }
    else if (strncmp(cmd,"echo ",5) == 0)
    {
        print(cmd+5);
    }
    else if (strcmp(cmd,"clear") == 0)
    {
        clear(0xF);
    }
    else if (strcmp(cmd,"ls") == 0)
    {
        fat16_list();
    }
    else if(strncmp(cmd, "cat ", 4) == 0)
    {
        FAT16_DirEntry file;

        if(fat16_find_path(cmd + 4, &file))
        {
            fat16_read_file(&file, (void*)0x800000);

            char* data = (char*)0x800000;

            for(uint32_t i = 0; i < file.file_size; i++)
                print_char(data[i]);
        }
    }
    else
    {
        print("Unknown command");
    }
}