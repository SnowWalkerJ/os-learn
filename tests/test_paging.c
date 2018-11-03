#include <libs/assert.h>
#include <kernel/page.h>
#include <drivers/screen.h>
#include <stddef.h>


char* test_paging() {
    void *phys_addr = (void*)0x1000000,
         *virt_addr = (void*)0x5000000;
    disable_paging();
    *(char*)phys_addr = 'X';
    bind_page(virt_addr, phys_addr);
    enable_paging();
    if (*(char*)virt_addr != 'X') return "Can't Access memory via page";
    unbind_page(virt_addr);
    if (*(char*)virt_addr == 'X') return "Can still access memory after unbinding page";
    return NULL;
}