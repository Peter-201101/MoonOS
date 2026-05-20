#include <types.h>
#include <include/kernel.h>
#include <core/panic.h>
#include <core/memory.h>
#include <core/task.h>
#include <core/scheduler.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <hal/x86/boot/multiboot.h>
#include <hal/x86/serial/serial_debug.h>
#include <hal/x86/interrupt/idt.h>
#include <hal/x86/interrupt/irq.h>
#include <hal/x86/timer/pit.h>

/* Simbol dari linker script */
extern uintptr_t _kernel_start;
extern uintptr_t _kernel_end;

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

/* --------------------------------------------------------------------------
 * kmain – entry point x86
 * -------------------------------------------------------------------------- */
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

    /* VGA tidak diinisialisasi dulu – akan ditambahkan setelah heap stabil */
    klog(LOG_OK "VGA: skipped for now\n");

    /* Lanjut ke kernel umum */
    kernel_main();

    /* Aktifkan interrupt setelah scheduler? 
       Di kernel_main sudah sti, jadi tidak perlu di sini */
    PANIC("kmain returned");
}
