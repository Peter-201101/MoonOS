#include <core/task.h>
#include <core/panic.h>
#include <core/memory.h>
#include <lib/string.h>
#include <include/kernel.h>

/* PID counter */
static uint32_t next_pid = 0;

/* Task yang sedang running */
static task_t *current_task = NULL;

/* Head linked list semua task */
static task_t *task_list = NULL;

/*---------------------------------------------------------------------------
 * Internal helpers
 *---------------------------------------------------------------------------*/

static void task_list_add(task_t *task)
{
    task->next = NULL;

    if (!task_list) {
        task_list = task;
        return;
    }

    task_t *cur = task_list;
    while (cur->next) cur = cur->next;
    cur->next = task;
}

static void task_list_remove(task_t *task)
{
    if (!task_list) return;

    if (task_list == task) {
        task_list = task->next;
        return;
    }

    task_t *cur = task_list;
    while (cur->next && cur->next != task)
        cur = cur->next;

    if (cur->next == task)
        cur->next = task->next;
}

/*
 * Setup initial stack frame untuk task baru.
 * Waktu context switch pertama, CPU akan "return"
 * ke entry point task seolah-olah task sudah pernah
 * di-switch sebelumnya.
 */
static void task_setup_stack(task_t *task, void (*entry)(void))
{
    /*
     * Stack tumbuh ke bawah di x86.
     * esp mulai di ujung atas stack, kita push frame ke bawah.
     */
    uint32_t *sp = (uint32_t *)((uint8_t *)task->stack + task->stack_size);

    /* EFLAGS: enable interrupts (IF bit = bit 9) */
    *--sp = 0x202;

    /* EIP: entry point task */
    *--sp = (uint32_t)entry;

    /* General purpose registers (semua 0) */
    *--sp = 0; /* eax */
    *--sp = 0; /* ecx */
    *--sp = 0; /* edx */
    *--sp = 0; /* ebx */
    *--sp = 0; /* esp (dummy, akan di-overwrite) */
    *--sp = 0; /* ebp */
    *--sp = 0; /* esi */
    *--sp = 0; /* edi */

    /* Simpan ESP ke context */
    task->context.esp = (uint32_t)sp;
    task->context.eip = (uint32_t)entry;
}

/*---------------------------------------------------------------------------
 * Public API
 *---------------------------------------------------------------------------*/

void task_init(void)
{
    next_pid     = 0;
    current_task = NULL;
    task_list    = NULL;

    klog(LOG_OK "Task subsystem initialized\n");
}

task_t *task_create(const char *name, void (*entry)(void), uint32_t priority)
{
    /* Alokasi TCB */
    task_t *task = (task_t *)kmalloc(sizeof(task_t));
    if (!task) {
        klog(LOG_ERR "task_create: out of memory for TCB\n");
        return NULL;
    }

    memset(task, 0, sizeof(task_t));

    /* Alokasi kernel stack */
    task->stack = (uint32_t *)kmalloc(TASK_STACK_SIZE);
    if (!task->stack) {
        klog(LOG_ERR "task_create: out of memory for stack\n");
        kfree(task);
        return NULL;
    }

    memset(task->stack, 0, TASK_STACK_SIZE);

    /* Isi TCB */
    task->pid        = next_pid++;
    task->state      = TASK_READY;
    task->priority   = priority;
    task->time_slice = 10;   /* 10 ticks default */
    task->stack_size = TASK_STACK_SIZE;
    task->total_ticks = 0;

    strncpy(task->name, name, sizeof(task->name) - 1);
    task->name[sizeof(task->name) - 1] = '\0';

    /* Setup initial stack frame */
    task_setup_stack(task, entry);

    /* Tambah ke task list */
    task_list_add(task);

    klog(LOG_INFO "Task created: [%u] %s (priority=%u)\n",
         task->pid, task->name, task->priority);

    return task;
}

void task_destroy(task_t *task)
{
    if (!task) return;

    klog(LOG_INFO "Task destroyed: [%u] %s\n", task->pid, task->name);

    task_list_remove(task);

    if (task->stack) kfree(task->stack);
    kfree(task);
}

task_t *task_current(void)
{
    return current_task;
}

task_t *task_get(uint32_t pid)
{
    task_t *cur = task_list;
    while (cur) {
        if (cur->pid == pid) return cur;
        cur = cur->next;
    }
    return NULL;
}

void task_dump(void)
{
    static const char *state_str[] = {
        "READY", "RUNNING", "BLOCKED", "ZOMBIE", "DEAD"
    };

    klog("[TASK] === Task list ===\n");
    task_t *cur = task_list;
    while (cur) {
        klog("[TASK] [%u] %s state=%s prio=%u ticks=%u\n",
             cur->pid,
             cur->name,
             state_str[cur->state],
             cur->priority,
             cur->total_ticks);
        cur = cur->next;
    }
    klog("[TASK] ==================\n");
}