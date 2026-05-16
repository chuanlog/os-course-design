#include "ata.h"
#include "io.h"

/* ATA Primary Bus I/O 端口基地址 */
#define ATA_IO 0x1F0

/* 等待驱动器状态为非忙碌 (BSY = 0) */
static void ata_wait_bsy(void) {
    while(inb(ATA_IO + 7) & 0x80);
}

/* 等待驱动器准备好传输数据 (DRQ = 1) */
static void ata_wait_drq(void) {
    while(!(inb(ATA_IO + 7) & 0x08));
}

void ata_read_sector(uint32_t lba, uint8_t* buffer) {
    ata_wait_bsy();
    
    /* 选择主驱动器并设置 LBA 寻址模式 (0xE0) 和最高 4 位 LBA */
    outb(ATA_IO + 6, 0xE0 | ((lba >> 24) & 0x0F));
    /* 设置要读取的扇区数为 1 */
    outb(ATA_IO + 2, 1);
    /* 写入 LBA 地址的剩余 24 位 */
    outb(ATA_IO + 3, (uint8_t) lba);
    outb(ATA_IO + 4, (uint8_t)(lba >> 8));
    outb(ATA_IO + 5, (uint8_t)(lba >> 16));
    /* 发送 READ SECTORS 命令 (0x20) */
    outb(ATA_IO + 7, 0x20);

    ata_wait_bsy();
    ata_wait_drq();

    /* 从数据端口读取 256 个字 (512 字节) */
    for (int i = 0; i < 256; i++) {
        uint16_t data = inw(ATA_IO + 0);
        buffer[i * 2] = (uint8_t) data;
        buffer[i * 2 + 1] = (uint8_t) (data >> 8);
    }
}

void ata_write_sector(uint32_t lba, const uint8_t* buffer) {
    ata_wait_bsy();
    
    outb(ATA_IO + 6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_IO + 2, 1);
    outb(ATA_IO + 3, (uint8_t) lba);
    outb(ATA_IO + 4, (uint8_t)(lba >> 8));
    outb(ATA_IO + 5, (uint8_t)(lba >> 16));
    /* 发送 WRITE SECTORS 命令 (0x30) */
    outb(ATA_IO + 7, 0x30);

    ata_wait_bsy();
    ata_wait_drq();

    /* 写入 256 个字 */
    for (int i = 0; i < 256; i++) {
        uint16_t data = buffer[i * 2] | (buffer[i * 2 + 1] << 8);
        outw(ATA_IO + 0, data);
    }
    
    /* 刷新驱动器缓存 */
    outb(ATA_IO + 7, 0xE7);
    ata_wait_bsy();
}
