#include <libs/assert.h>
#include <kernel/page.h>
#include <kernel/kmemory.h>
#include <drivers/screen.h>
#include <stddef.h>


char* test_paging() {
    void *virt_addr = (void*)0x5000000;
    void *phys_addr = kalloc_page();
    char *result = NULL;
    disable_paging();
    *(char*)phys_addr = 'X';
    bind_page(virt_addr, phys_addr);
    enable_paging();
    if (*(char*)virt_addr != 'X') {
        result = "Can't Access memory via page";
    } else {
        unbind_page(virt_addr);
        // if (*(char*)virt_addr == 'X') {
        //     result = "Can still access memory after unbinding page";
        // }
    }
    kfree_page(phys_addr);
    return result;
}