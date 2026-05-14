#include <hal/x86/boot/multiboot.h>
#include <mm/pmm.h>
#include <hal/x86/serial/serial_debug.h>

void multiboot_parse_mmap(multiboot_info_t *mbi)
{
    if (!(mbi->flags & MULTIBOOT_INFO_MMAP)) {
        serial_puts("[MMAP] No memory map\n");
        return;
    }

    serial_printf("[MMAP] Lower: %u KB | Upper: %u KB\n",
        mbi->mem_lower, mbi->mem_upper);

    multiboot_mmap_t *mmap =
        (multiboot_mmap_t *)mbi->mmap_addr;
    multiboot_mmap_t *end =
        (multiboot_mmap_t *)(mbi->mmap_addr + mbi->mmap_length);

    while (mmap < end) {
        const char *type_str =
            (mmap->type == MULTIBOOT_MMAP_AVAILABLE) ? "AVAILABLE" : "RESERVED";

        serial_printf("[MMAP] %x | len: %u KB | %s\n",
            (uint32_t)mmap->base_addr,
            (uint32_t)(mmap->length / 1024),
            type_str);

        if (mmap->type == MULTIBOOT_MMAP_AVAILABLE
            && mmap->base_addr >= PAGE_SIZE
            && mmap->base_addr < 0x100000000ULL)
        {
            uint32_t base = (uint32_t)mmap->base_addr;
            uint32_t len  = (mmap->base_addr + mmap->length > 0x100000000ULL)
                            ? (uint32_t)(0x100000000ULL - mmap->base_addr)
                            : (uint32_t)mmap->length;
            pmm_unreserve(base, len);
        }

        mmap = (multiboot_mmap_t *)
            ((uint32_t)mmap + mmap->size + sizeof(mmap->size));
    }
}