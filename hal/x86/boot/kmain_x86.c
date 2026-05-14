#include <types.h>
#include <include/kernel.h>
#include <core/panic.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <hal/x86/boot/multiboot.h>
#include <hal/x86/serial/serial_debug.h>
#include <hal/x86/interrupt/idt.h>
#include <hal/x86/interrupt/irq.h>
#include <hal/x86/timer/pit.h>
#include <display/display.h>
#include <display/vga.h>

extern uintptr_t _kernel_start;
extern uintptr_t _kernel_end;
extern void multiboot_parse_mmap(multiboot_info_t *mbi);
extern void kernel_main(void);

/* x86 entry point — dipanggil dari boot/entry.asm */
void kmain(uint32_t magic, multiboot_info_t *mbi)
{
    /* 1. Serial debug — output pertama */
    serial_debug_init();
    serial_puts("\n=== " KERNEL_NAME " " KERNEL_VERSION " (x86) ===\n");

    /* 2. Validasi multiboot */
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
        PANIC("Invalid multiboot magic");
    if (!(mbi->flags & MULTIBOOT_INFO_MMAP))
        PANIC("No memory map");

    /* 3. PMM */
    uint32_t kstart = (uint32_t)&_kernel_start;
    uint32_t kend   = (uint32_t)&_kernel_end;
    pmm_init(mbi, kstart, kend);
    multiboot_parse_mmap(mbi);
    pmm_reserve(0x0, MB(1));
    pmm_reserve(kstart, kend - kstart);
    pmm_stats_t s = pmm_get_stats();
    klog(LOG_OK "PMM: %u MB free\n", (s.free_frames * PAGE_SIZE) / MB(1));

    /* 4. VMM */
    vmm_init();
    klog(LOG_OK "VMM: paging enabled\n");

    /* 5. IDT + IRQ + PIT */
    idt_init();
    irq_init();
    pit_init();
    klog(LOG_OK "IDT + PIT: ready\n");

    /* 6. VGA */
    vga_module.init();
    display_register(&vga_ops);
    display_set_color(COLOR_LIGHT_CYAN, COLOR_BLACK);
    display_puts("================================\n");
    display_puts("  MoonOS " KERNEL_VERSION " (x86)\n");
    display_puts("================================\n");
    display_set_color(COLOR_LIGHT_GREY, COLOR_BLACK);
    klog(LOG_OK "VGA: ready\n");

    /* 7. Common kernel init */
    kernel_main();

    /* 8. Enable interrupts */
    __asm__ volatile("sti");

    PANIC("kernel_main returned");
}