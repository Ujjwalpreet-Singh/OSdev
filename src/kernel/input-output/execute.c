#include <stdint.h>
#include "psf.h"
#include "../drivers/initalize.h"
#include "printoguri.h"
#include "editor.h"

void run_uma(const char* filename);

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

int ends_with(const char* str, const char* suffix)
{
    int len1 = 0;
    int len2 = 0;

    while(str[len1]) len1++;
    while(suffix[len2]) len2++;

    if(len2 > len1)
        return 0;

    for(int i = 0; i < len2; i++)
    {
        if(str[len1 - len2 + i] != suffix[i])
            return 0;
    }

    return 1;
}

void execute_command(char* cmd)
{
    if(strcmp(cmd, "help") == 0)
    {
        print("commands: help echo clear ls cat edit write");
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
    else if (strcmp(cmd,"oguri cap") == 0)
    {
        oguriprint();
    }
    else if (strncmp(cmd, "write ", 6) == 0)
    {
        char* args = cmd + 6;

        char filename[32];
        char content[256];

        int i = 0;

        // extract filename
        while(args[i] && args[i] != ' ')
        {
            filename[i] = args[i];
            i++;
        }
        filename[i] = 0;

        // skip space
        if(args[i] == ' ')
            i++;

        int j = 0;

        // extract content
        while(args[i])
        {
            content[j++] = args[i++];
        }
        content[j] = 0;

        if(j == 0)
        {
            print("No content provided");
            return;
        }

        if(fat16_create_file(filename, (uint8_t*)content, j))
            print("File written");
        else
            print("Write failed");
    }
    else if (strncmp(cmd, "fatdbg", 6) == 0)
    {

        char* arg = cmd + 6;

        // skip spaces
        while(*arg == ' ')
            arg++;

        int max = 50; // default

        if(*arg != 0)
            max = atoi(arg);

        fat16_dump_fat(max);
    }
    else if (strncmp(cmd, "chain ", 6) == 0)
    {
        char* arg = cmd + 6;

        // skip spaces
        while(*arg == ' ')
            arg++;

        if(*arg == 0)
        {
            print("Usage: chain <cluster>");
            return;
        }

        uint16_t cluster = atoi_u16(arg);

        if(cluster < 2)
        {
            print("Invalid cluster");
            return;
        }

        fat16_dump_chain(cluster);
    }
    else if (strncmp(cmd,"edit ",5) == 0){
        char* arg = cmd+5;
        fb_save();
        cursor_save();
        clear(0xFF);
        editor(arg);
        clear(0xF);
        fb_restore();
        cursor_restore();
    }
    else if (strncmp(cmd, "run ", 4) == 0)
    {
        char* filename = cmd + 4;

        // skip spaces
        while(*filename == ' ')
            filename++;

        if(!ends_with(filename, ".UMA"))
        {
            print("Error: only .uma files can be ran");
            return;
        }

        run_uma(filename);
    }
    else
    {
        print("Unknown command");
    }
}

void run_uma(const char* filename)
{
    FAT16_DirEntry file;

    if (!fat16_find_path(filename, &file))
    {
        print("Script not found");
        return;
    }

    fat16_read_file(&file, (void*)0x800000);

    char* data = (char*)0x800000;
    uint32_t i = 0;

    char line[128];
    int line_pos = 0;

    while (i < file.file_size)
    {
        char c = data[i++];

        if (c == '\n' || c == '\r')
        {
            line[line_pos] = 0;

            if (line_pos > 0)
                execute_command(line);

            line_pos = 0;
        }
        else
        {
            line[line_pos++] = c;
        }
    }

    // last line (if no newline at end)
    if (line_pos > 0)
    {
        line[line_pos] = 0;
        execute_command(line);
    }
}