#ifndef ATA_H
#define ATA_H
/*
    Reference: https://wiki.osdev.org/ATA_PIO_Mode
*/
#include <stdint.h>
#include <fs/buffer.h>
#include <kernel/port.h>

#define SECTOR_SIZE 512


enum hd_device {
    HDA = 0,
    HDB = 1,
    HDC = 2,
    HDD = 3
};


enum bus_number {
    PRIMARY_BUS = 0,
    SECONDARY_BUS = 1
};


enum drive_number {
    MASTER_DRIVE = 0,
    SLAVE_DRIVE = 1
};


void init_hdd();

int pio_read_lba(int, int, void*, int);

#endif /* ATA_H */