#ifndef GDT_H
#define GDT_H
#include <stdint.h>

typedef struct {
    uint16_t limit;
    uint16_t base0_15;
    uint8_t base16_23;
    uint8_t access;
    uint8_t flags;
    uint8_t base24_31;
} __attribute__((packed)) gdt_gate_t;

typedef struct {
    uint16_t limit;
    uint32_t offset;
} __attribute__((packed)) descriptor_t;

uint8_t make_access(int pr, int privl, int ex, int dc, int rw);
void set_gdt_gate(int entry_id, uint32_t base, uint32_t limit, uint8_t access,
                  uint8_t flags);
void init_gdt();

extern gdt_gate_t gdt_gates[];

/* compose access byte from flag bits
    pr: present
    privl: priviledge ring0 - ring3
    ex: executable
    dc
    rw: readable for code / writable for data */
#define make_access(pr, privl, ex, dc, rw) \
    (uint8_t) (((pr) << 7) | ((privl) << 5) | \
    (1 << 4) | ((ex) << 3) | ((dc) << 2) | ((rw) << 1))

#endif