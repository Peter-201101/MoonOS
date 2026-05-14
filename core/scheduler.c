#include <core/scheduler.h>
#include <core/panic.h>
#include <include/kernel.h>
#include <lib/string.h>

/* Run queue — linked list task yang READY */
static task_t  *run_queue    = NULL;
static task_t  *current      = NULL;
static uint8_t  sched_active = 0;

volatile uint8_t sched_need_reschedule = 0;

/*---------------------------------------------------------------------------
 * Internal
 *---------------------------------------------------------------------------*/

static void run_queue_add(task_t *task)
{
    task->next = NULL;

    if (!run_queue) {
        run_queue = task;
        return;
    }

    /*
     * Insert berdasarkan priority (0 = highest).
     * Task priority rendah masuk di belakang task priority sama/lebih tinggi.
     */
    if (task->priority < run_queue->priority) {
        task->next = run_queue;
        run_queue  = task;
        return;
    }

    task_t *cur = run_queue;
    while (cur->next && cur->next->priority <= task->priority)
        cur = cur->next;

    task->next = cur->next;
    cur->next  = task;
}

static void run_queue_remove(task_t *task)
{
    if (!run_queue) return;

    if (run_queue == task) {
        run_queue = task->next;
        task->next = NULL;
        return;
    }

    task_t *cur = run_queue;
    while (cur->next && cur->next != task)
        cur = cur->next;

    if (cur->next == task) {
        cur->next  = task->next;
        task->next = NULL;
    }
}

/*---------------------------------------------------------------------------
 * Public API
 *---------------------------------------------------------------------------*/

void scheduler_init(void)
{
    run_queue    = NULL;
    current      = NULL;
    sched_active = 0;

    klog(LOG_OK "Scheduler initialized\n");
}

void scheduler_add(task_t *task)
{
    if (!task) return;
    task->state = TASK_READY;
    run_queue_add(task);
}

void scheduler_remove(task_t *task)
{
    if (!task) return;
    run_queue_remove(task);
}

task_t *scheduler_next(void)
{
    if (!run_queue) return current; /* tidak ada task lain */

    /* Ambil task pertama dari run queue */
    task_t *next = run_queue;
    run_queue_remove(next);
    return next;
}

void scheduler_tick(void)
{
    if (!sched_active || !current) return;
    current->total_ticks++;
    if (current->time_slice > 0) current->time_slice--;
    if (current->time_slice == 0) {
        current->time_slice = 10;
        sched_need_reschedule = 1;  /* set flag, switch di assembly */
    }
}

void scheduler_yield(void)
{
    if (!sched_active) return;

    task_t *prev = current;
    task_t *next = scheduler_next();

    if (!next || next == prev) return; /* tidak ada task lain */

    /* Kembalikan current ke run queue kalau masih RUNNING */
    if (prev && prev->state == TASK_RUNNING) {
        prev->state = TASK_READY;
        run_queue_add(prev);
    }

    next->state = TASK_RUNNING;
    current     = next;

    klog(LOG_INFO "Switch: [%u]%s -> [%u]%s\n",
         prev ? prev->pid : 0,
         prev ? prev->name : "none",
         next->pid, next->name);

    /* Context switch — loncat ke assembly */
    if (prev)
        switch_context(&prev->context, &next->context);
}

void scheduler_block(task_t *task)
{
    if (!task) return;
    task->state = TASK_BLOCKED;
    run_queue_remove(task);

    /* Kalau yang di-block adalah current task, yield */
    if (task == current)
        scheduler_yield();
}

void scheduler_unblock(task_t *task)
{
    if (!task) return;
    task->state = TASK_READY;
    run_queue_add(task);
}

void scheduler_enable(void)
{
    sched_active = 1;

    /* Jalankan task pertama */
    if (run_queue) {
        current             = run_queue;
        run_queue_remove(current);
        current->state      = TASK_RUNNING;

        klog(LOG_INFO "Scheduler started, first task: [%u] %s\n",
             current->pid, current->name);

        /* Jump ke context task pertama */
        switch_context(NULL, &current->context);
    }
}

void scheduler_disable(void)
{
    sched_active = 0;
}