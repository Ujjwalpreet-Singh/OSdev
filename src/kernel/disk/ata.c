#include "ata.h"
#include <stdint.h>

#define ATA_DATA        0x1F0
#define ATA_SECCOUNT    0x1F2
#define ATA_LBA_LOW     0x1F3
#define ATA_LBA_MID     0x1F4
#define ATA_LBA_HIGH    0x1F5
#define ATA_DRIVE       0x1F6
#define ATA_STATUS      0x1F7
#define ATA_COMMAND     0x1F7

#define ATA_SR_BSY  0x80
#define ATAT_SR_DRQ 0x08

static uint8_t driveSelect = 0xE0;

static inline void outb(uint16_t port,uint8_t val)
{
    __asm__ volatile("outb %0,%1"::"a"(val),"Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ volatile("inb %1,%0":"=a"(ret):"Nd"(port));
    return ret;
}

static inline uint16_t inw(uint16_t port)
{
    uint16_t ret;
    __asm__ volatile("inw %1,%0":"=a"(ret):"Nd"(port));
    return ret;
}

void ata_init(uint8_t bootDrive)
{
    if (bootDrive & 1)
        driveSelect = 0xF0;
    else
        driveSelect = 0xE0;
}

static void ata_wait(){
    while(inb(ATA_STATUS) & ATA_SR_BSY);
}

bool ata_read28(uint32_t lba,uint8_t count, void* buffer)
{
    volatile uint16_t* ptr = (volatile uint16_t*)buffer;

    ata_wait();

    outb(ATA_DRIVE,driveSelect | ((lba >> 24) & 0x0F));

    outb(ATA_SECCOUNT,count);
    outb(ATA_LBA_LOW,lba&0xFF);
    outb(ATA_LBA_MID,(lba>>8)&0xFF);
    outb(ATA_LBA_HIGH,(lba>>16)&0xFF);

    outb(ATA_COMMAND,0x20);

    for(int s=0;s<count;s++)
    {
        uint8_t status;

        do {
            status = inb(ATA_STATUS);
        } while(status & ATA_SR_BSY);

        while(!(status & ATAT_SR_DRQ))
            status = inb(ATA_STATUS);

        for(int i=0;i<256;i++)
            ptr[i] = inw(ATA_DATA);

        ptr += 256;
    }

    return true;
}