#include <kernel/gdt.h>
#include <kernel/defs.h>

#define GDT_ENTRIES (4 + NR_TASKS * 2)
gdt_gate_t gdt_gates[GDT_ENTRIES];
descriptor_t gdt_descriptor;

extern void gdt_flush();

void init_gdt () {
    /* reset the gdt descriptors and gates so that they can be controlled by the kernel */
    set_gdt_gate(0, 0, 0, 0, 0);                                         // NULL segment
    set_gdt_gate(1, 0, 0xfffff, make_access(1, 0, 1, 0, 1), 0b1100);     // ring0 code segment
    set_gdt_gate(2, 0, 0xfffff, make_access(1, 0, 0, 0, 1), 0b1100);     // ring0 data segment
    // set_gdt_gate(3, 0, 0xfffff, make_access(1, 3, 1, 0, 1), 0b1100);     // ring3 code segment
    // set_gdt_gate(4, 0, 0xfffff, make_access(1, 3, 0, 0, 1), 0b1100);     // ring3 data segment
    gdt_descriptor.offset = (uint32_t)gdt_gates;
    gdt_descriptor.limit = (uint16_t)(GDT_ENTRIES * sizeof(gdt_gate_t));
    gdt_flush();
}

void set_gdt_gate(int entry_id, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    gdt_gates[entry_id].base0_15 = (uint16_t)(base >> 16);
    gdt_gates[entry_id].base16_23 = (uint8_t)((base >> 8) & 0xff);
    gdt_gates[entry_id].base24_31 = (uint8_t)(base & 0xff);
    gdt_gates[entry_id].access = access;
    gdt_gates[entry_id].flags = (flags << 4) | (limit & 0xf);
    gdt_gates[entry_id].limit = (uint16_t) ((limit >> 4) & 0xffff);
}

