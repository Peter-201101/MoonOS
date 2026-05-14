#ifndef CONFIG_H
#define CONFIG_H

#ifdef ARCH_X86
#define KERNEL_LOAD_ADDR    0x100000
#define KERNEL_HEAP_START ((uintptr_t)&_end)
#define KERNEL_HEAP_END 0x08000000  // 128 MB
#define HEAP_SIZE         (1024 * 1024) // 1 MB heap default
#define KERNEL_STACK_SIZE   0x4000
#endif

#ifdef ARCH_ARM
#define KERNEL_LOAD_ADDR    0x80000
#define KERNEL_HEAP_START ((uintptr_t)&_end)
#define KERNEL_HEAP_END 0x40000000  // contoh: 1 GiB
#define HEAP_SIZE         (1024 * 1024) // 1 MB heap default
#define KERNEL_STACK_SIZE   0x4000
#endif

#define PAGE_SIZE           4096
#define MAX_TASKS           256
#define TASK_STACK_SIZE     0x2000

#endif