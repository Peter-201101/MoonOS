#include "pmm.h"

#ifdef ARCH_X86
#include <hal/x86/boot/multiboot.h>
#endif

/*---------------------------------------------------------------------------
 * Static variables — shared semua arch
 *---------------------------------------------------------------------------*/
static uint32_t pmm_bitmap[PMM_BITMAP_SIZE];
static uint32_t pmm_total_frames = 0;
static uint32_t pmm_used_frames  = 0;
static uint32_t pmm_max_frame    = 0;

/*---------------------------------------------------------------------------
 * Internal bitmap ops
 *---------------------------------------------------------------------------*/
static inline void bitmap_set(uint32_t f)   { pmm_bitmap[f/32] |=  (1u << (f%32)); }
static inline void bitmap_clear(uint32_t f) { pmm_bitmap[f/32] &= ~(1u << (f%32)); }
static inline bool bitmap_test(uint32_t f)  { return (pmm_bitmap[f/32] >> (f%32)) & 1u; }

static uint32_t bitmap_find_free(void)
{
    for (uint32_t i = 0; i < (pmm_max_frame+31)/32; i++) {
        if (pmm_bitmap[i] == 0xFFFFFFFF) continue;
        for (uint32_t b = 0; b < 32; b++) {
            uint32_t f = i*32 + b;
            if (f >= pmm_max_frame) return (uint32_t)-1;
            if (!bitmap_test(f)) return f;
        }
    }
    return (uint32_t)-1;
}

static uint32_t bitmap_find_free_n(uint32_t n)
{
    uint32_t start = 0, count = 0;
    for (uint32_t f = 0; f < pmm_max_frame; f++) {
        if (!bitmap_test(f)) {
            if (!count) start = f;
            if (++count == n) return start;
        } else {
            count = 0;
        }
    }
    return (uint32_t)-1;
}

static void mark_used(uint32_t addr, uint32_t size)
{
    uint32_t fs = ADDR_TO_FRAME(ALIGN_DOWN(addr, PAGE_SIZE));
    uint32_t fe = ADDR_TO_FRAME(ALIGN_UP(addr + size, PAGE_SIZE));
    for (uint32_t f = fs; f < fe && f < pmm_max_frame; f++)
        if (!bitmap_test(f)) { bitmap_set(f); pmm_used_frames++; }
}

static void mark_free(uint32_t addr, uint32_t size)
{
    uint32_t fs = ADDR_TO_FRAME(ALIGN_UP(addr, PAGE_SIZE));
    uint32_t fe = ADDR_TO_FRAME(ALIGN_DOWN(addr + size, PAGE_SIZE));
    for (uint32_t f = fs; f < fe && f < pmm_max_frame; f++)
        if (bitmap_test(f)) { bitmap_clear(f); pmm_used_frames--; }
}

/*---------------------------------------------------------------------------
 * x86 init
 *---------------------------------------------------------------------------*/
#ifdef ARCH_X86
void pmm_init(multiboot_info_t *mbi, uint32_t kstart, uint32_t kend)
{
    for (uint32_t i = 0; i < PMM_BITMAP_SIZE; i++)
        pmm_bitmap[i] = 0xFFFFFFFF;

    pmm_total_frames = ((mbi->mem_lower + mbi->mem_upper) * 1024) / PAGE_SIZE;
    if (pmm_total_frames > PMM_BITMAP_SIZE * 32)
        pmm_total_frames = PMM_BITMAP_SIZE * 32;
    pmm_max_frame   = pmm_total_frames;
    pmm_used_frames = pmm_total_frames;

    if (!(mbi->flags & MULTIBOOT_INFO_MMAP)) return;

    multiboot_mmap_t *mmap =
        (multiboot_mmap_t *)mbi->mmap_addr;
    multiboot_mmap_t *mmap_end =
        (multiboot_mmap_t *)(mbi->mmap_addr + mbi->mmap_length);

    while (mmap < mmap_end) {
        if (mmap->type == MULTIBOOT_MMAP_AVAILABLE
            && mmap->base_addr >= PAGE_SIZE
            && mmap->base_addr < 0x100000000ULL) {
            uint32_t base = (uint32_t)mmap->base_addr;
            uint32_t len  = (mmap->base_addr + mmap->length > 0x100000000ULL)
                            ? (uint32_t)(0x100000000ULL - mmap->base_addr)
                            : (uint32_t)mmap->length;
            mark_free(base, len);
        }
        mmap = (multiboot_mmap_t *)
            ((uint32_t)mmap + mmap->size + sizeof(mmap->size));
    }

    mark_used(0x0, MB(1));
    mark_used(kstart, kend - kstart);
}
#endif

/*---------------------------------------------------------------------------
 * ARM init
 *---------------------------------------------------------------------------*/
#ifdef ARCH_ARM
void pmm_init_simple(uint32_t mem_start, uint32_t mem_size,
                     uint32_t kstart, uint32_t kend)
{
    for (uint32_t i = 0; i < PMM_BITMAP_SIZE; i++)
        pmm_bitmap[i] = 0xFFFFFFFF;

    pmm_total_frames = mem_size / PAGE_SIZE;
    if (pmm_total_frames > PMM_BITMAP_SIZE * 32)
        pmm_total_frames = PMM_BITMAP_SIZE * 32;
    pmm_max_frame   = pmm_total_frames;
    pmm_used_frames = pmm_total_frames;

    mark_free(mem_start, mem_size);
    mark_used(0x0, MB(1));
    mark_used(kstart, kend - kstart);
}
#endif

/*---------------------------------------------------------------------------
 * Public API — sama untuk semua arch
 *---------------------------------------------------------------------------*/
uint32_t pmm_alloc(void)
{
    if (pmm_used_frames >= pmm_total_frames) return 0;
    uint32_t f = bitmap_find_free();
    if (f == (uint32_t)-1 || f == 0) return 0;
    bitmap_set(f); pmm_used_frames++;
    return FRAME_TO_ADDR(f);
}

uint32_t pmm_alloc_n(uint32_t n)
{
    if (!n || pmm_used_frames + n > pmm_total_frames) return 0;
    uint32_t f = bitmap_find_free_n(n);
    if (f == (uint32_t)-1) return 0;
    for (uint32_t i = 0; i < n; i++) { bitmap_set(f+i); pmm_used_frames++; }
    return FRAME_TO_ADDR(f);
}

void pmm_free(uint32_t addr)
{
    if (!addr) return;
    uint32_t f = ADDR_TO_FRAME(addr);
    if (f >= pmm_max_frame || !bitmap_test(f)) return;
    bitmap_clear(f); pmm_used_frames--;
}

void pmm_free_n(uint32_t addr, uint32_t n)
{
    for (uint32_t i = 0; i < n; i++) pmm_free(addr + i * PAGE_SIZE);
}

void pmm_reserve(uint32_t addr, uint32_t size)   { mark_used(addr, size); }
void pmm_unreserve(uint32_t addr, uint32_t size) { mark_free(addr, size); }

pmm_stats_t pmm_get_stats(void)
{
    return (pmm_stats_t){
        .total_frames    = pmm_total_frames,
        .used_frames     = pmm_used_frames,
        .free_frames     = pmm_total_frames - pmm_used_frames,
        .reserved_frames = 0
    };
}