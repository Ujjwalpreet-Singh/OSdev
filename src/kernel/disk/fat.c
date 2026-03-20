#include "fat.h"
#include "ata.h"
#include "../input-output/psf.h"
#include <stddef.h>

#define ROOT_ADDR 0x70000
#define FAT_ADDR  0x80000

static FAT16_BPB bpb;

static uint32_t first_fat_sector;
static uint32_t first_root_sector;
static uint32_t first_data_sector;
static uint32_t root_dir_sectors;

static FAT16_DirEntry* root = (FAT16_DirEntry*)ROOT_ADDR;
static uint16_t* fat = (uint16_t*)FAT_ADDR;
void split_path(const char* path, char* dir, char* file);

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
            printf(" <DIR>  ");

        print_int(dir[i].file_size);

        printf("\n");

        if(dir[i].attr & 0x10)
        {
            fat16_read_directory(dir[i].first_cluster, dir_buffer);

            fat16_list_dir_recursive(dir_buffer, 128,depth+1);
        }
    }
}

int fat16_find_free_cluster()
{
    uint32_t total_data_sectors =
    bpb.total_sectors16 -
    (bpb.reserved_sectors +
     bpb.fat_count * bpb.sectors_per_fat +
     root_dir_sectors);

    uint32_t total_clusters =
        total_data_sectors / bpb.sectors_per_cluster;
    for (int i = 2; i < total_clusters+2; i++)
    {
        if (fat[i] == 0x0000)
            return i;
    }

    return -1; // no space
}

bool fat16_allocate_clusters(int count, uint16_t* first_cluster_out)
{
    uint16_t prev = 0;
    uint16_t first = 0;

    for(int i = 0; i < count; i++)
    {
        int cluster = fat16_find_free_cluster();
        if(cluster < 0)
            return false;
        fat[cluster] = 0xFFFF;
        if(i == 0)
            first = cluster;
        else
            fat[prev] = cluster;

        prev = cluster;
    }

    fat[prev] = 0xFFF8; // end of chain

    *first_cluster_out = first;
    return true;
}

bool fat16_write_file_data(uint16_t start_cluster,
                           uint8_t* buffer,
                           uint32_t size)
{
    uint16_t cluster = start_cluster;
    uint32_t cluster_size =
        bpb.sectors_per_cluster * bpb.bytes_per_sector;

    uint8_t temp[cluster_size]; // assume max cluster size <= 4KB

    while(cluster >= 2 && cluster < 0xFFF8)
    {
        uint32_t lba = fat16_cluster_to_lba(cluster);

        uint32_t to_write =
            (size > cluster_size) ? cluster_size : size;

        // zero buffer (important for last cluster)
        for(uint32_t i = 0; i < cluster_size; i++)
            temp[i] = 0;

        // copy actual data
        for(uint32_t i = 0; i < to_write; i++)
            temp[i] = buffer[i];

        ata_write28(lba,
                    bpb.sectors_per_cluster,
                    (uint16_t*)temp);

        buffer += to_write;
        size   -= to_write;

        if(size == 0)
            break;

        cluster = fat[cluster];
    }

    return true;
}

void strcpy(char* dest, const char* src)
{
    while(*src)
    {
        *dest++ = *src++;
    }
    *dest = 0;
}

bool fat16_flush_fat()
{
    for(int i = 0; i < bpb.fat_count; i++)
    {
        ata_write28(first_fat_sector + i * bpb.sectors_per_fat,
                    bpb.sectors_per_fat,
                    fat);
    }
    return true;
}

bool fat16_create_file(const char* name,
                       uint8_t* data,
                       uint32_t size)
{
    fat16_read_fat();
    char dir_path[128];
    char filename[32];

    split_path(name, dir_path, filename);

    FAT16_DirEntry dir_entry;
    FAT16_DirEntry dir_buffer[128];

    FAT16_DirEntry* target_dir = root;
    int max_entries = bpb.root_entry_count;

    if(dir_path[0] != 0)
    {
        if(!fat16_find_path(dir_path, &dir_entry))
        {
            print("Directory not found");
            return false;
        }

        if(!(dir_entry.attr & 0x10))
        {
            print("Not a directory");
            return false;
        }

        fat16_read_directory(dir_entry.first_cluster, dir_buffer);

        target_dir = dir_buffer;
        max_entries = 128;
    }

    char upper[12];
    int i = 0;

    while(filename[i] && i < 11)
    {
        char c = filename[i];

        if(c >= 'a' && c <= 'z')
            c -= 32;

        upper[i] = c;
        i++;
    }

    upper[i] = 0;

    uint32_t cluster_size =
        bpb.sectors_per_cluster * bpb.bytes_per_sector;

    int cluster_count =
        (size + cluster_size - 1) / cluster_size;

    uint16_t first_cluster;

    if(!fat16_allocate_clusters(cluster_count, &first_cluster))
        return false;

    fat16_flush_fat();
    fat16_write_file_data(first_cluster, data, size);


    // create dir entry
    FAT16_DirEntry* entry = NULL;

    for(int i = 0; i < max_entries; i++)
    {
        if(target_dir[i].name[0] == 0x00 || target_dir[i].name[0] == 0xE5)
        {
            entry = &target_dir[i];
            break;
        }
    }

    if(!entry) return false;


    char fat_name[11];
    fat16_format_name(upper, fat_name);

    for(int i = 0; i < 8; i++)
        entry->name[i] = fat_name[i];

    for(int i = 0; i < 3; i++)
        entry->ext[i] = fat_name[8 + i];

    entry->attr = 0x20;
    entry->first_cluster = first_cluster;
    entry->file_size = size;

    if(target_dir == root)
    {
        ata_write28(first_root_sector,
                    root_dir_sectors,
                    (uint16_t*)root);
    }
    else
    {
        uint16_t cluster = dir_entry.first_cluster;
        uint32_t lba = fat16_cluster_to_lba(cluster);

        ata_write28(lba,
                    bpb.sectors_per_cluster,
                    (uint16_t*)dir_buffer);
    }

    return true;
}

void fat16_dump_fat(int max)
{
    
    printf("\nFAT TABLE DUMP:\n");

    for(int i = 2; i < max; i++)
    {
        printf("%d: %x\n", i, fat[i]);
    }
}

void fat16_dump_chain(uint16_t cluster)
{
    printf("Cluster chain: ");

    int count = 0;

    while(cluster >= 2 && cluster < 0xFFF8)
    {
        printf("%u -> ", cluster);

        cluster = fat[cluster];

        // prevent infinite loops
        if(count++ > 100)
        {
            printf("LOOP DETECTED\n");
            return;
        }
    }

    printf("EOF\n");
}

void split_path(const char* path, char* dir, char* file)
{
    int len = 0;
    while(path[len]) len++;

    int last_slash = -1;

    for(int i = 0; i < len; i++)
    {
        if(path[i] == '/')
            last_slash = i;
    }

    if(last_slash == -1)
    {
        // no directory → root
        dir[0] = 0;
        strcpy(file, path);
        return;
    }

    // directory part
    for(int i = 0; i < last_slash; i++)
        dir[i] = path[i];
    dir[last_slash] = 0;

    // file part
    strcpy(file, path + last_slash + 1);
}