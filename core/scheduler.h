#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <core/task.h>

/*
 * Round-robin scheduler dengan priority.
 * Priority 0 = highest. Task dengan priority sama
 * di-round-robin. Task priority tinggi selalu
 * didahulukan dari yang rendah.
 */

/* Init scheduler */
void scheduler_init(void);

/* Tambah task ke scheduler */
void scheduler_add(task_t *task);

/* Hapus task dari scheduler */
void scheduler_remove(task_t *task);

/* Pilih task berikutnya (dipanggil dari timer IRQ) */
task_t *scheduler_next(void);

/* Trigger context switch — dipanggil dari timer IRQ handler */
void scheduler_tick(void);

/* Yield: task sengaja melepas CPU */
void scheduler_yield(void);

/* Block task saat ini */
void scheduler_block(task_t *task);

/* Unblock task */
void scheduler_unblock(task_t *task);

/* Enable/disable scheduling */
void scheduler_enable(void);
void scheduler_disable(void);

/* Context switch — implemented di assembly */
extern void switch_context(cpu_context_t *old_ctx, cpu_context_t *new_ctx);

extern volatile uint8_t sched_need_reschedule;

#endif