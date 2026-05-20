#include <types.h>
#include <include/kernel.h>
#include <core/panic.h>
#include <core/memory.h>
#include <core/task.h>
#include <core/scheduler.h>

/* Deklarasi dari komponen lain */
void os_server_init(void);
void platform_init(void);

static void os_server_task(void)
{
    os_server_init();
}

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
