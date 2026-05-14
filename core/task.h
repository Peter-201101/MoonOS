#ifndef TASK_H
#define TASK_H

#include <types.h>
#include <include/config.h>

/*
 * Task states
 */
typedef enum {
    TASK_READY,      /* siap jalan, menunggu scheduler */
    TASK_RUNNING,    /* sedang jalan di CPU */
    TASK_BLOCKED,    /* menunggu sesuatu (I/O, sleep, dll) */
    TASK_ZOMBIE,     /* sudah selesai, belum di-cleanup */
    TASK_DEAD        /* siap dihapus */
} task_state_t;

/*
 * CPU registers context — disimpan waktu context switch
 * Urutan ini HARUS sama dengan urutan push di switch_context.asm
 */
typedef struct {
    uint32_t edi, esi, ebp;
    uint32_t esp;   /* stack pointer task */
    uint32_t ebx, edx, ecx, eax;
    uint32_t eip;   /* instruction pointer */
    uint32_t eflags;
} cpu_context_t;

/*
 * Task Control Block (TCB)
 */
typedef struct task {
    uint32_t        pid;            /* process ID */
    char            name[32];       /* nama task */
    task_state_t    state;          /* state saat ini */
    cpu_context_t   context;        /* saved CPU state */
    uint32_t       *stack;          /* base of kernel stack */
    uint32_t        stack_size;     /* ukuran stack */
    uint32_t        priority;       /* 0 = highest */
    uint32_t        time_slice;     /* sisa time slice (ticks) */
    uint32_t        total_ticks;    /* total ticks yang sudah dipakai */
    struct task    *next;           /* linked list */
} task_t;

/* Init task subsystem */
void task_init(void);

/* Buat task baru */
task_t *task_create(const char *name, void (*entry)(void), uint32_t priority);

/* Hapus task */
void task_destroy(task_t *task);

/* Ambil task yang sedang running */
task_t *task_current(void);

/* Ambil task by PID */
task_t *task_get(uint32_t pid);

/* Print semua task (debug) */
void task_dump(void);

#endif