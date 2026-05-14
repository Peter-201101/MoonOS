#ifndef PMM_H
#define PMM_H

#include <types.h>

#define PAGE_SIZE       4096
#define PAGE_SHIFT      12
#define PMM_BITMAP_SIZE (0x100000 / 32)

#define ADDR_TO_FRAME(addr)  ((addr) >> PAGE_SHIFT)
#define FRAME_TO_ADDR(frame) ((frame) << PAGE_SHIFT)

typedef struct {
    uint32_t total_frames;
    uint32_t used_frames;
    uint32_t free_frames;
    uint32_t reserved_frames;
} pmm_stats_t;

/* x86 init */
#ifdef ARCH_X86
#include <hal/x86/boot/multiboot.h>
void pmm_init(multiboot_info_t *mbi, uint32_t kstart, uint32_t kend);
#endif

/* ARM init */
#ifdef ARCH_ARM
void pmm_init_simple(uint32_t mem_start, uint32_t mem_size,
                     uint32_t kstart, uint32_t kend);
#endif

/* Common API */
uint32_t    pmm_alloc(void);
uint32_t    pmm_alloc_n(uint32_t n);
void        pmm_free(uint32_t addr);
void        pmm_free_n(uint32_t addr, uint32_t n);
void        pmm_reserve(uint32_t addr, uint32_t size);
void        pmm_unreserve(uint32_t addr, uint32_t size);
pmm_stats_t pmm_get_stats(void);

#endif