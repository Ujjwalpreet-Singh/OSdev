#ifndef ATA_H
#define ATA_H

#include <stdint.h>
#include <stdbool.h>

//
// ATA primary bus I/O ports
//
#define ATA_PRIMARY_IO       0x1F0
#define ATA_PRIMARY_CTRL     0x3F6

#define ATA_REG_DATA         0x1F0
#define ATA_REG_ERROR        0x1F1
#define ATA_REG_SECCOUNT     0x1F2
#define ATA_REG_LBA_LOW      0x1F3
#define ATA_REG_LBA_MID      0x1F4
#define ATA_REG_LBA_HIGH     0x1F5
#define ATA_REG_DRIVE        0x1F6
#define ATA_REG_STATUS       0x1F7
#define ATA_REG_COMMAND      0x1F7

//
// ATA commands
//
#define ATA_CMD_READ_SECTORS 0x20

//
// Status flags
//
#define ATA_SR_BSY     0x80
#define ATA_SR_DRDY    0x40
#define ATA_SR_DF      0x20
#define ATA_SR_DRQ     0x08
#define ATA_SR_ERR     0x01

//
// Drive select values
//
#define ATA_MASTER     0xE0
#define ATA_SLAVE      0xF0


//
// Public API
//

//
// Initialize ATA using BIOS boot drive number
// bootDrive = value originally in DL (0x80, 0x81, etc)
//
void ata_init(uint8_t bootDrive);


//
// Read sectors using LBA-28
//
// Parameters:
// lba    = starting sector
// count  = number of sectors
// buffer = destination memory
//
bool ata_read28(uint32_t lba, uint8_t count, void* buffer);


//
// Optional generic disk interface (recommended)
// lets FAT driver remain hardware-independent
//
bool disk_read(uint32_t lba, uint8_t count, void* buffer);


#endif