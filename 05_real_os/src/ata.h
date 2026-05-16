#ifndef ATA_H
#define ATA_H

#include <stdint.h>

/* 从 LBA 扇区读取 512 字节的数据 */
void ata_read_sector(uint32_t lba, uint8_t* buffer);

/* 向 LBA 扇区写入 512 字节的数据 */
void ata_write_sector(uint32_t lba, const uint8_t* buffer);

#endif