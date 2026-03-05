#include "fat.h"
#include "ata.h"
#include "../input-output/psf.h"

#define ROOT_ADDR 0x70000
#define FAT_ADDR  0x80000

static FAT16_BPB bpb;

static uint32_t first_fat_sector;
static uint32_t first_root_sector;
static uint32_t first_data_sector;
static uint32_t root_dir_sectors;

static FAT16_DirEntry* root = (FAT16_DirEntry*)ROOT_ADDR;
static uint16_t* fat = (uint16_t*)FAT_ADDR;


bool fat16_init()
{
    uint8_t buffer[512];

    if(!ata_read28(0,1,buffer))
        return false;

    bpb = *(FAT16_BPB*)buffer;

    root_dir_sectors =
        ((bpb.root_entry_count * 32) +
        (bpb.bytes_per_sector - 1))
        / bpb.bytes_per_sector;

    first_fat_sector =
        bpb.reserved_sectors;

    first_root_sector =
        first_fat_sector +
        (bpb.fat_count * bpb.sectors_per_fat);

    first_data_sector =
        first_root_sector +
        root_dir_sectors;

    printf("FAT16 initialized\n");

    printf("reserved: %u\n", bpb.reserved_sectors);
    printf("fat_count: %u\n", bpb.fat_count);
    printf("sectors_per_fat: %u\n", bpb.sectors_per_fat);
    printf("root_entries: %u\n", bpb.root_entry_count);
    printf("root_dir_sectors: %u\n", root_dir_sectors);
    printf("sectors_per_cluster = %u\n", bpb.sectors_per_cluster);
    printf("first_fat_sector: %u\n", first_fat_sector);
    printf("first_root_sector: %u\n", first_root_sector);
    printf("first_data_sector: %u\n", first_data_sector);
    return true;
}

bool fat16_read_root()
{
    uint32_t sectors = root_dir_sectors;

    if(!ata_read28(first_root_sector,
                   sectors,
                   root))
        return false;

    printf("Root directory loaded\n");

    return true;
}

void fat16_list()
{
    printf("\nFilesystem tree:\n");

    fat16_list_dir_recursive(root, bpb.root_entry_count,0);
}

static void fat16_format_name(const char* input, char* output)
{
    // fill with spaces
    for(int i=0;i<11;i++)
        output[i] = ' ';

    int i = 0;
    int j = 0;

    // name
    while(input[i] && input[i] != '.' && j < 8)
    {
        output[j++] = input[i++];
    }

    // extension
    if(input[i] == '.')
    {
        i++;
        j = 8;

        int k = 0;
        while(input[i] && k < 3)
        {
            output[j++] = input[i++];
            k++;
        }
    }
}

bool fat16_find(const char* name, FAT16_DirEntry* out)
{
    char fat_name[11];

    fat16_format_name(name, fat_name);

    for(int i=0;i<bpb.root_entry_count;i++)
    {
        if(root[i].name[0] == 0)
            return false;

        if(root[i].name[0] == 0xE5)
            continue;

        bool match = true;

        for(int j=0;j<11;j++)
        {
            char c;

            if(j < 8)
                c = root[i].name[j];
            else
                c = root[i].ext[j-8];

            if(c != fat_name[j])
            {
                match = false;
                break;
            }
        }

        if(match)
        {
            *out = root[i];
            print_buffer("dir entry:", &root[i], 32);
            printf("Found file: %s\n", name);
            printf("Cluster: %u\n", root[i].first_cluster);
            printf("Size: %u bytes\n", root[i].file_size);

            return true;
        }
    }

    return false;
}

bool fat16_read_fat()
{
    if(!ata_read28(first_fat_sector,
                   bpb.sectors_per_fat,
                   fat))
        return false;

    printf("FAT loaded\n");

    return true;
}

static uint32_t fat16_cluster_to_lba(uint16_t cluster)

{
    return first_data_sector +
          (cluster - 2) *
          bpb.sectors_per_cluster;
}

bool fat16_read_file(FAT16_DirEntry* file, void* buffer)
{
    uint16_t cluster = file->first_cluster;
    uint8_t* ptr = buffer;

    printf("BUFFER=%x\n", ptr);

    while(cluster >= 2 && cluster < 0xFFF8)
    {
        uint32_t lba = fat16_cluster_to_lba(cluster);

        printf("READ LBA=%u INTO %x\n", lba, ptr);

        ata_read28(lba, bpb.sectors_per_cluster, ptr);

        print_buffer("sector:", ptr, 16);

        ptr += bpb.sectors_per_cluster * bpb.bytes_per_sector;

        cluster = fat[cluster];
    }

    return true;
}

bool fat16_read_directory(uint16_t cluster, FAT16_DirEntry* buffer)
{
    uint8_t* ptr = (uint8_t*)buffer;

    while (cluster >= 2 && cluster < 0xFFF8)
    {
        uint32_t lba = fat16_cluster_to_lba(cluster);

        ata_read28(lba, bpb.sectors_per_cluster, ptr);

        ptr += bpb.sectors_per_cluster * bpb.bytes_per_sector;

        cluster = fat[cluster];
    }

    return true;
}

bool fat16_find_in_dir(
    FAT16_DirEntry* dir,
    int max_entries,
    const char* name,
    FAT16_DirEntry* out)
{
    char fat_name[11];

    fat16_format_name(name, fat_name);

    for(int i = 0; i < max_entries; i++)
    {
        if(dir[i].name[0] == 0)
            return false;

        if(dir[i].name[0] == 0xE5)
            continue;

        bool match = true;

        for(int j = 0; j < 11; j++)
        {
            char c;

            if(j < 8)
                c = dir[i].name[j];
            else
                c = dir[i].ext[j-8];

            if(c != fat_name[j])
            {
                match = false;
                break;
            }
        }

        if(match)
        {
            *out = dir[i];
            return true;
        }
    }

    return false;
}

const char* next_path_part(const char* path, char* out)
{
    int i = 0;

    while(path[i] && path[i] != '/')
    {
        out[i] = path[i];
        i++;
    }

    out[i] = 0;

    if(path[i] == '/')
        return path + i + 1;

    return path + i;
}

bool fat16_find_path(const char* path, FAT16_DirEntry* out)
{
    FAT16_DirEntry dir_buffer[128];
    FAT16_DirEntry entry;

    FAT16_DirEntry* current = root;
    int max_entries = bpb.root_entry_count;

    if(path[0] == '/')
        path++;

    while(1)
    {
        char part[12];

        path = next_path_part(path, part);

        if(!fat16_find_in_dir(current, max_entries, part, &entry))
            return false;

        if(*path == 0)
        {
            *out = entry;
            return true;
        }

        if(!(entry.attr & 0x10))
            return false;

        fat16_read_directory(entry.first_cluster, dir_buffer);

        current = dir_buffer;
        max_entries = 128;
    }
}

void fat16_list_dir(FAT16_DirEntry* dir, int max_entries)
{
    for(int i=0;i<max_entries;i++)
    {
        if(dir[i].name[0] == 0)
            break;

        if(dir[i].name[0] == 0xE5)
            continue;

        if(dir[i].attr == 0x0F) // skip LFN
            continue;

        for(int j=0;j<8;j++)
            print_char(dir[i].name[j]);

        print_char('.');

        for(int j=0;j<3;j++)
            print_char(dir[i].ext[j]);

        if(dir[i].attr & 0x10)
            printf(" <DIR>");

        printf("\n");
    }
}

void fat16_list_dir_recursive(FAT16_DirEntry* dir, int max_entries,int depth)
{
    FAT16_DirEntry dir_buffer[128];

    for(int i=0;i<max_entries;i++)
    {
        if(dir[i].name[0] == 0)
            break;

        if(dir[i].name[0] == 0xE5)
            continue;

        if(dir[i].attr == 0x0F)
            continue;

        if(dir[i].name[0] == '.')
            continue;


        for(int d = 0; d < depth; d++)
            printf(" |   ");

        printf(" |--> ");

        for(int j=0;j<8;j++)
            print_char(dir[i].name[j]);

        print_char('.');

        for(int j=0;j<3;j++)
            print_char(dir[i].ext[j]);

        if(dir[i].attr & 0x10)
            printf(" <DIR>");

        printf("\n");

        if(dir[i].attr & 0x10)
        {
            fat16_read_directory(dir[i].first_cluster, dir_buffer);

            fat16_list_dir_recursive(dir_buffer, 128,depth+1);
        }
    }
}