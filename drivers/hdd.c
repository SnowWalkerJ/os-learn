#include <stdint.h>


struct drive_info {
    uint8_t enabled;
    uint8_t drive_type;
    uint16_t cylinders;
    uint8_t sectors_per_track;
    uint8_t sides;
    uint8_t drivers;
} __attribute__((packed));

struct drive_info drive_infos[3];