#include <lib/stdlib.h>
#include <lib/string.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <core/panic.h>
#include <include/kernel.h>
#include <include/config.h>

/* --------------------------------------------------------------------------
 * Kernel Heap Allocator — Free List
 * -------------------------------------------------------------------------- */

#define HEAP_MAGIC_FREE 0xDEADBEEF
#define HEAP_MAGIC_USED 0xCAFEBABE
#define HEAP_MIN_SPLIT  32          /* minimum sisa untuk di-split */
#define ALIGN_BYTES     8           /* alignment 8 byte */

typedef struct block_hdr {
    uint32_t        magic;    /* DEAD atau CAFE — detect corruption */
    uint32_t        size;     /* ukuran data area (tidak include header) */
    uint8_t         free;     /* 1 = free, 0 = used */
    struct block_hdr *next;   /* next block di heap (bukan free list) */
} block_hdr_t;

/* Heap state */
static block_hdr_t *heap_start = NULL;
static uint32_t     heap_end   = 0;
static uint32_t     heap_base  = 0;

/* --------------------------------------------------------------------------
 * Internal helpers
 * -------------------------------------------------------------------------- */

/* Expand heap dengan alokasi page baru dari PMM */
static block_hdr_t *heap_expand(uint32_t min_size)
{
    uint32_t needed = sizeof(block_hdr_t) + min_size;
    uint32_t pages  = (needed + PAGE_SIZE - 1) / PAGE_SIZE;

    /* Guard: jangan expand melewati batas identity map */
    if (heap_end + pages * PAGE_SIZE > KERNEL_HEAP_END) {
        klog("[HEAP] Out of mapped heap space!\n");
        return NULL;
    }

    /* Alokasi halaman fisik dari PMM — gunakan alamat yang diberikan apa adanya */
    uint32_t phys = pmm_alloc_n(pages);
    if (!phys) {
        klog("[HEAP] pmm_alloc_n failed!\n");
        return NULL;
    }

    /* Inisialisasi blok baru di alamat fisik yang diberikan */
    block_hdr_t *block = (block_hdr_t *)phys;
    block->magic = HEAP_MAGIC_FREE;
    block->size  = pages * PAGE_SIZE - sizeof(block_hdr_t);
    block->free  = 1;
    block->next  = NULL;

    /* Update heap_end ke akhir blok baru */
    heap_end = phys + pages * PAGE_SIZE;

    /* Sambungkan ke linked list heap */
    if (heap_start == NULL) {
        heap_start = block;
    } else {
        block_hdr_t *cur = heap_start;
        while (cur->next) cur = cur->next;
        cur->next = block;

        /* Merge dengan blok sebelumnya jika free */
        if (cur->free) {
            cur->size += sizeof(block_hdr_t) + block->size;
            cur->next  = NULL;
        }
    }

    return block;
}

/* Split block jadi dua kalau sisanya cukup besar */
static void block_split(block_hdr_t *block, uint32_t size)
{
    uint32_t remainder = block->size - size;
    if (remainder < sizeof(block_hdr_t) + HEAP_MIN_SPLIT)
        return; /* tidak worth split */

    block_hdr_t *new_block = (block_hdr_t *)((uint8_t *)(block + 1) + size);
    new_block->magic = HEAP_MAGIC_FREE;
    new_block->size  = remainder - sizeof(block_hdr_t);
    new_block->free  = 1;
    new_block->next  = block->next;

    block->size = size;
    block->next = new_block;
}

/* Merge block free yang bersebelahan */
static void heap_merge_free(void)
{
    block_hdr_t *cur = heap_start;
    while (cur && cur->next) {
        if (cur->free && cur->next->free) {
            /* Merge cur dengan next */
            cur->size += sizeof(block_hdr_t) + cur->next->size;
            cur->next  = cur->next->next;
            /* Jangan advance — coba merge lagi dengan next baru */
        } else {
            cur = cur->next;
        }
    }
}

/* Validasi header block — detect heap corruption */
static void block_validate(block_hdr_t *block, const char *caller)
{
    if (block->magic != HEAP_MAGIC_FREE && block->magic != HEAP_MAGIC_USED) {
        klog("[HEAP] Corruption detected in %s! Block: %x magic: %x\n",
             caller, (uint32_t)block, block->magic);
        PANIC("Heap corruption detected");
    }
}

/* --------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------- */

extern char _end;

void heap_init(void)
{
    heap_base  = (uintptr_t)&_end;
    heap_start = NULL;          /* Belum ada blok */
    heap_end   = heap_base;     /* Awal mula */

    /* Alokasi blok pertama (1 halaman) */
    block_hdr_t *initial = heap_expand(PAGE_SIZE - sizeof(block_hdr_t));
    if (!initial)
        PANIC("Failed to initialize kernel heap");

    klog("[HEAP] Initialized at %x (base %x), size: %u KB\n",
         (uint32_t)initial, heap_base, (heap_end - heap_base) / 1024);
}

void *kmalloc(size_t size)
{
    if (!size) return NULL;

    /* Align size ke 8 byte boundary */
    size = ALIGN_UP(size, ALIGN_BYTES);

    /* First fit search */
    block_hdr_t *cur = heap_start;
    while (cur) {
        block_validate(cur, "kmalloc");

        if (cur->free && cur->size >= size) {
            block_split(cur, size);
            cur->free  = 0;
            cur->magic = HEAP_MAGIC_USED;
            return (void *)(cur + 1); /* return area setelah header */
        }
        cur = cur->next;
    }

    /* Tidak ada block yang cukup — expand heap */
    block_hdr_t *new_block = heap_expand(size);
    if (!new_block) {
        klog("[HEAP] Out of memory! Requested: %u bytes\n", size);
        return NULL;
    }

    block_split(new_block, size);
    new_block->free  = 0;
    new_block->magic = HEAP_MAGIC_USED;
    return (void *)(new_block + 1);
}

void kfree(void *ptr)
{
    if (!ptr) return;

    /* Header ada tepat sebelum pointer yang dikasih user */
    block_hdr_t *block = (block_hdr_t *)ptr - 1;
    block_validate(block, "kfree");

    if (block->free) {
        klog("[HEAP] Double free detected! ptr: %x\n", (uint32_t)ptr);
        PANIC("Double free");
    }

    block->free  = 1;
    block->magic = HEAP_MAGIC_FREE;

    /* Merge block free yang bersebelahan */
    heap_merge_free();
}

void *kcalloc(size_t n, size_t size)
{
    size_t total = n * size;
    void  *ptr   = kmalloc(total);
    if (ptr) memset(ptr, 0, total);
    return ptr;
}

void *krealloc(void *ptr, size_t new_size)
{
    if (!ptr)     return kmalloc(new_size);
    if (!new_size){ kfree(ptr); return NULL; }

    block_hdr_t *block = (block_hdr_t *)ptr - 1;
    block_validate(block, "krealloc");

    /* Kalau block sekarang sudah cukup besar, pakai saja */
    if (block->size >= ALIGN_UP(new_size, ALIGN_BYTES))
        return ptr;

    /* Alokasi baru, copy, free lama */
    void *new_ptr = kmalloc(new_size);
    if (!new_ptr) return NULL;

    memcpy(new_ptr, ptr, block->size);
    kfree(ptr);
    return new_ptr;
}

/* Debug: print semua block di heap */
void heap_dump(void)
{
    klog("[HEAP] === Heap dump ===\n");
    klog("[HEAP] Base: %x | End: %x | Used: %u KB\n",
         heap_base, heap_end, (heap_end - heap_base) / 1024);

    block_hdr_t *cur = heap_start;
    uint32_t     idx = 0;
    uint32_t     total_free = 0, total_used = 0;

    while (cur) {
        block_validate(cur, "heap_dump");
        klog("[HEAP] [%u] addr:%x size:%u %s\n",
             idx++,
             (uint32_t)(cur + 1),
             cur->size,
             cur->free ? "FREE" : "USED");

        if (cur->free) total_free += cur->size;
        else           total_used += cur->size;

        cur = cur->next;
    }

    klog("[HEAP] Free: %u bytes | Used: %u bytes\n", total_free, total_used);
    klog("[HEAP] ==================\n");
}
