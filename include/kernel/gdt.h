#ifndef GDT_H
#define GDT_H
#include <stdint.h>

typedef struct {
    uint16_t limit;
    uint16_t base0_15;
    uint8_t  base16_23;
    uint8_t  access;
    uint8_t  flags;
    uint8_t  base24_31;
} __attribute__((packed)) gdt_gate_t;

typedef struct {
    uint16_t limit;
    uint32_t offset;
} __attribute__((packed)) gdt_descriptor_t;

uint8_t make_access(int pr, int privl, int ex, int dc, int rw);
void set_gdt_gate(int entry_id, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
void init_gdt ();

#endif