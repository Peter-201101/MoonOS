#include <types.h>
#include <include/kernel.h>
#include <core/panic.h>
#include <core/memory.h>
#include <core/task.h>
#include <core/scheduler.h>
#include <mm/pmm.h>

void os_server_init(void);
void platform_init(void);
void platform_dump(void);

static void os_server_task(void)
{
    os_server_init();
}

/*
 * kernel_main — dipanggil dari kmain_x86.c atau kmain_arm.c
 * setelah arch-specific init selesai
 */
void kmain(void)
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
}