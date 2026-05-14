#ifndef SYSCALL_H
#define SYSCALL_H

#include <types.h>

#define SYS_EXIT        0
#define SYS_WRITE       1
#define SYS_READ        2
#define SYS_OPEN        3
#define SYS_CLOSE       4
#define SYS_SLEEP       5
#define SYS_GETPID      6
#define SYS_YIELD       7
#define SYS_MALLOC      8
#define SYS_FREE        9
#define SYS_MAX         10

#define SYS_OK          0
#define SYS_ERR        -1
#define SYS_EPERM      -2
#define SYS_EINVAL     -3
#define SYS_ENOMEM     -4
#define SYS_EBADF      -5

typedef struct {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t eip;
    uint32_t esp;
    uint32_t eflags;
} syscall_regs_t;

typedef int32_t (*syscall_fn_t)(syscall_regs_t *regs);

void syscall_init(void);
void syscall_dispatch(syscall_regs_t *regs);

#endif