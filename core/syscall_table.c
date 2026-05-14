#include <core/syscall.h>
#include <core/syscall_table.h>
#include <core/task.h>
#include <core/scheduler.h>
#include <core/memory.h>
#include <core/panic.h>
#include <include/kernel.h>
#include <hal/x86/serial/serial_debug.h>

/*---------------------------------------------------------------------------
 * Handler implementations
 *---------------------------------------------------------------------------*/

static int32_t sys_exit(syscall_regs_t *regs)
{
    int32_t exit_code = (int32_t)regs->ebx;
    task_t *cur = task_current();

    klog(LOG_INFO "sys_exit: task [%u] %s exit code=%d\n",
         cur ? cur->pid : 0,
         cur ? cur->name : "?",
         exit_code);

    if (cur) {
        cur->state = TASK_ZOMBIE;
        scheduler_remove(cur);
        scheduler_yield();
    }

    return SYS_OK;
}

static int32_t sys_write(syscall_regs_t *regs)
{
    uint32_t    fd  = regs->ebx;
    const char *buf = (const char *)regs->ecx;
    uint32_t    len = regs->edx;

    if (!buf)             return SYS_EINVAL;
    if (len == 0)         return SYS_OK;
    if (fd != 1 && fd != 2) return SYS_EBADF;

    for (uint32_t i = 0; i < len; i++)
        serial_putchar(buf[i]);

    return (int32_t)len;
}

static int32_t sys_read(syscall_regs_t *regs)
{
    (void)regs;
    /* TODO: implement setelah keyboard driver ada */
    return SYS_ERR;
}

static int32_t sys_open(syscall_regs_t *regs)
{
    (void)regs;
    /* TODO: implement setelah VFS ada */
    return SYS_ERR;
}

static int32_t sys_close(syscall_regs_t *regs)
{
    (void)regs;
    /* TODO: implement setelah VFS ada */
    return SYS_ERR;
}

static int32_t sys_sleep(syscall_regs_t *regs)
{
    uint32_t ticks = regs->ebx;
    task_t  *cur   = task_current();

    if (!cur) return SYS_ERR;

    klog(LOG_INFO "sys_sleep: task [%u] sleeping %u ticks\n",
         cur->pid, ticks);

    /* TODO: timer-based sleep setelah PIT ready */
    (void)ticks;
    scheduler_yield();

    return SYS_OK;
}

static int32_t sys_getpid(syscall_regs_t *regs)
{
    (void)regs;
    task_t *cur = task_current();
    return cur ? (int32_t)cur->pid : SYS_ERR;
}

static int32_t sys_yield(syscall_regs_t *regs)
{
    (void)regs;
    scheduler_yield();
    return SYS_OK;
}

static int32_t sys_malloc(syscall_regs_t *regs)
{
    size_t size = (size_t)regs->ebx;
    if (!size) return SYS_EINVAL;

    void *ptr = kmalloc(size);
    if (!ptr) return SYS_ENOMEM;

    return (int32_t)ptr;
}

static int32_t sys_free(syscall_regs_t *regs)
{
    void *ptr = (void *)regs->ebx;
    if (!ptr) return SYS_EINVAL;

    kfree(ptr);
    return SYS_OK;
}

/*---------------------------------------------------------------------------
 * Syscall table
 *---------------------------------------------------------------------------*/

static syscall_fn_t syscall_table[SYS_MAX] = {
    [SYS_EXIT]   = sys_exit,
    [SYS_WRITE]  = sys_write,
    [SYS_READ]   = sys_read,
    [SYS_OPEN]   = sys_open,
    [SYS_CLOSE]  = sys_close,
    [SYS_SLEEP]  = sys_sleep,
    [SYS_GETPID] = sys_getpid,
    [SYS_YIELD]  = sys_yield,
    [SYS_MALLOC] = sys_malloc,
    [SYS_FREE]   = sys_free,
};

void syscall_table_init(void)
{
    klog(LOG_OK "Syscall table: %u syscalls registered\n", SYS_MAX);
}

syscall_fn_t syscall_table_get(uint32_t num)
{
    if (num >= SYS_MAX) return NULL;
    return syscall_table[num];
}