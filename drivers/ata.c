/*
    Reference: https://wiki.osdev.org/ATA_PIO_Mode
*/
#include <drivers/ata.h>



enum COMMANDS {
    READ_SECTORS = 0x20,
    WRITE_SECTORS = 0x30,
    FLUSH_CACHE = 0xE7,
    IDENTIFY = 0xEC
};


static enum drive_number selected[2];

static int exists[4];


#define PRIMARY_IO_BASE        0x1F0
#define PRIMARY_CONTROL_BASE   0x3F6

#define SECONDARY_IO_BASE      0x170
#define SECONDARY_CONTROL_BASE 0x376

#define DRIVER_NUMER(dev) ((dev) & 1)
#define BUS_NUMBER(dev) ((dev) & 2)

#define IO_BASE(bus) ((bus) ? SECONDARY_IO_BASE : PRIMARY_IO_BASE)
#define CONTROL_BASE(bus) ((bus) ? SECONDARY_CONTROL_BASE : PRIMARY_CONTROL_BASE)

#define data_io(bus) (IO_BASE(bus) + 0)
#define error_io(bus) (IO_BASE(bus) + 1)
#define features_io(bus) (IO_BASE(bus) + 1)
#define sector_count_io(bus) (IO_BASE(bus) + 2)
#define lba_lo_io(bus) (IO_BASE(bus) + 3)
#define lba_mi_io(bus) (IO_BASE(bus) + 4)
#define lba_hi_io(bus) (IO_BASE(bus) + 5)
#define drive_head_io(bus) (IO_BASE(bus) + 6)
#define command_io(bus) (IO_BASE(bus) + 7)
#define status_io(bus) (IO_BASE(bus) + 7)

#define alt_status_control(bus) (CONTROL_BASE(bus) + 0)
#define device_control(bus) (CONTROL_BASE(bus) + 1)
#define drive_addr_control(bus) (CONTROL_BASE(bus) + 1)


struct error_register {
    uint8_t amnf  : 1;      // Address mark not found
    uint8_t tkznf : 1;      // Track zero not found
    uint8_t abrt  : 1;      // Aborted command
    uint8_t mcr   : 1;      // Media change request
    uint8_t idnf  : 1;      // ID not found
    uint8_t mc    : 1;      // Media changed
    uint8_t unc   : 1;      // Uncorrectable data error
    uint8_t bbk   : 1;      // Bad Block detected
};


struct status_register {
    uint8_t err  : 1;        // Indicates an error occurred. Send a new command to clear it (or nuke it with a Software Reset)
    uint8_t idx  : 1;        // Index. Always set to zero
    uint8_t corr : 1;        // Corrected data. Always set to zero
    uint8_t drq  : 1;        // Set when the drive has PIO data to transfer, or is ready to accept PIO data
    uint8_t srv  : 1;        // Overlapped Mode Service Request
    uint8_t df   : 1;        // Drive Fault Error (does not set ERR)
    uint8_t rdy  : 1;        // Bit is clear when drive is spun down, or after an error. Set otherwise
    uint8_t bsy  : 1;        // Indicates the drive is preparing to send/receive data (wait for it to clear). In case of 'hang' (it never clears), do a software reset
};


void delay_400ns(enum bus_number);

void select_drive(enum bus_number, enum drive_number);

struct status_register identify(enum hd_device);

struct status_register poll_status(enum bus_number);


void init_hdd() {
    for (int i = 0; i < 4; i++) {
        struct status_register status = identify(i);
        exists[i] = *(uint8_t*)&status != 0;
    }

    select_drive(PRIMARY_BUS, MASTER_DRIVE);
    select_drive(SECONDARY_BUS, MASTER_DRIVE);
}


void delay_400ns(enum bus_number bus) {
    uint16_t port = bus == PRIMARY_BUS ? PRIMARY_IO_BASE : SECONDARY_IO_BASE;
    portByteIn(port);
    portByteIn(port);
    portByteIn(port);
    portByteIn(port);
}


void select_drive(enum bus_number bus, enum drive_number drive) {
    if (selected[bus] == drive)
        return;
    portByteOut(drive_head_io(bus), ((drive << 4) | 0xE0));
    delay_400ns(bus);
    selected[bus] = drive;
}


struct status_register identify(enum hd_device dev) {
    uint8_t bus = BUS_NUMBER(dev);
    uint8_t drive = DRIVER_NUMER(dev);
    portByteOut(sector_count_io(bus), 0);
    portByteOut(lba_hi_io(bus), 0);
    portByteOut(lba_mi_io(bus), 0);
    portByteOut(lba_lo_io(bus), 0);
    select_drive(bus, drive);
    portByteOut(command_io(bus), IDENTIFY);
    return poll_status(bus);
}


struct status_register poll_status(enum bus_number bus) {
    uint8_t status_byte;
    struct status_register* status;
    do {
        status_byte = portByteIn(status_io(bus));
        status = (struct status_register*)&status_byte;
    } while (!status_byte && status->bsy);
    return *status;
}


int read_sector(struct buffer_head* bh) {
    uint8_t drive_number = DRIVER_NUMER(bh->dev);
    uint8_t bus_number      = BUS_NUMBER(bh->dev);
    portByteOut(drive_head_io(bus_number), (uint8_t)((drive_number << 4) | 0xE0 | ((bh->block >> 24) & 0x0F)));
    portByteOut(sector_count_io(bus_number), 1);
    portByteOut(lba_lo_io(bus_number), (uint8_t)(bh->block & 0xFF));
    portByteOut(lba_mi_io(bus_number), (uint8_t)((bh->block >> 8) & 0xFF));
    portByteOut(lba_hi_io(bus_number), (uint8_t)((bh->block >> 16) & 0xFF));
    portByteOut(command_io(bus_number), READ_SECTORS);
    int i;
    for (i = 0; i < 8; i++) {                           // 8 * 512 = 4K
        struct status_register status;
        do {
            status = poll_status(bus_number);
        } while (status.err || status.drq);
        if (status.drq) {
            insw(data_io(drive_number), (uint16_t*)bh->data, 256);
            delay_400ns(bus_number);
        } else if (status.err) {
            i--;
            break;
        }
    }
    return i + 1;
}