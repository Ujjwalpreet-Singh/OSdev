#ifndef FAT_H
#define FAT_H

#include <stdint.h>
#include <stdbool.h>

typedef struct __attribute__((packed))
{
    uint8_t  jump[3];
    char     oem[8];

    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t  fat_count;
    uint16_t root_entry_count;
    uint16_t total_sectors16;
    uint8_t  media_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t head_count;
    uint32_t hidden_sectors;
    uint32_t total_sectors32;

} FAT16_BPB;

typedef struct __attribute__((packed))
{
    char name[8];
    char ext[3];
    uint8_t attr;
    uint8_t reserved;
    uint8_t creation_time_tenths;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t access_date;
    uint16_t high_cluster;
    uint16_t mod_time;
    uint16_t mod_date;
    uint16_t first_cluster;
    uint32_t file_size;

} FAT16_DirEntry;

bool fat16_init();
bool fat16_read_root();
void fat16_list();
bool fat16_find(const char* name, FAT16_DirEntry* out);
bool fat16_read_file(FAT16_DirEntry* file, void* buffer);
bool fat16_read_fat();

#endif