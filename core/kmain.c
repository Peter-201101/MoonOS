#include <types.h>
#include <module.h>
#include <hal/x86/boot/multiboot.h>
#include <hal/x86/serial/serial_debug.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <hal/x86/interrupt/idt.h>
#include <hal/x86/interrupt/irq.h>
#include <hal/x86/timer/pit.h>
#include <core/panic.h>
#include <include/kernel.h>   // untuk LOG_*, klog
#include <display/display.h> // optional
#include <types.h>
#include <include/kernel.h>
#include <core/panic.h>
#include <core/memory.h>
#include <core/task.h>

extern module_t vga_module;
extern uintptr_t _kernel_start, _kernel_end;

/* Deklarasi fungsi dari komponen lain */
void os_server_init(void);
void platform_init(void);

static void os_server_task(void)
{
    os_server_init();
}

/* --------------------------------------------------------------------------
 * kernel_main – inisialisasi umum setelah arsitektur siap
 * -------------------------------------------------------------------------- */
void kernel_main(void)
{
    /* Heap */
    heap_init();
    klog(LOG_OK "Heap: ready\n");

    /* Platform — devices, drivers, VFS */
    platform_init();
    klog(LOG_OK "Platform: ready\n");

    /* Task + Scheduler */
    task_init();
    scheduler_init();

    task_t *server = task_create("os-server", os_server_task, 1);
    scheduler_add(server);

    klog(LOG_OK "Starting MoonOS...\n");
    scheduler_enable();

    /* Seharusnya tidak kembali */
    PANIC("kernel_main returned");
}

void kmain(uint32_t magic, multiboot_info_t *mbi)
{
    serial_debug_init();
    serial_puts("\n=== " KERNEL_NAME " " KERNEL_VERSION " (x86) ===\n");

    if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
        PANIC("Invalid multiboot magic");
    if (!(mbi->flags & MULTIBOOT_INFO_MMAP))
        PANIC("No memory map");

    /* PMM */
    uint32_t kstart = (uint32_t)&_kernel_start;
    uint32_t kend   = (uint32_t)&_kernel_end;
    pmm_init(mbi, kstart, kend);
    pmm_reserve(0x0, MB(1));
    pmm_reserve(kstart, kend - kstart);
    pmm_stats_t s = pmm_get_stats();
    klog(LOG_OK "PMM: %u MB free\n", (s.free_frames * PAGE_SIZE) / MB(1));

    /* VMM */
    vmm_init();
    klog(LOG_OK "VMM: paging enabled\n");

    /* IDT + IRQ + PIT */
    idt_init();
    irq_init();
    pit_init();
    klog(LOG_OK "IDT + PIT: ready\n");

    /* VGA module init (akan mendaftarkan display_ops) */
    extern module_t vga_module;
    vga_module.init();
uint16_t *vga_test = (uint16_t*)0xB8000;
for (int i = 0; i < 80; i++) {
    vga_test[i] = 0x0F << 8 | ('A' + (i % 26));
}
    klog(LOG_OK "VGA: ready\n");

    /* Di sini klog sudah mengirim ke layar VGA dan serial */

    /* Lanjut ke kernel_main jika ada */
    kernel_main();

    /* Jika kernel_main tidak pernah kembali, jangan panik dulu */
    /*for(;;);*/
}
