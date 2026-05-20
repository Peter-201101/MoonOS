#include <mm/vmm.h>
#include <mm/pmm.h>
#include <core/panic.h>
#include <include/kernel.h>
#include <include/config.h>

#ifdef ARCH_X86

static page_dir_t  kernel_dir __attribute__((aligned(4096)));
static page_dir_t *current_dir = NULL;

static inline void cr3_write(uint32_t addr)
{
    __asm__ volatile("mov %0, %%cr3" : : "r"(addr) : "memory");
}

static inline void cr0_set_pg(void)
{
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000001;
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0) : "memory");
}

static inline void tlb_flush(uint32_t virt)
{
    __asm__ volatile("invlpg (%0)" : : "r"(virt) : "memory");
}

/* Temporary mapping slot for initializing page tables (last identity-mapped page) */
#define TEMP_PAGE_VIRT 0x7FF000

static page_table_t *get_or_create_table(page_dir_t *pd, uint32_t pd_idx, uint32_t flags)
{
    pde_t pde = pd->entries[pd_idx];
    if (pde & PAGE_PRESENT)
        return (page_table_t *)(pde & ~0xFFF);

    uint32_t phys = pmm_alloc();
    if (!phys) PANIC("VMM: out of memory allocating page table");

    /* Map the newly allocated page table into the temporary slot
     * so we can write to it as a virtual address */
    uint32_t temp_pd_idx = PD_INDEX(TEMP_PAGE_VIRT);
    uint32_t temp_pt_idx = PT_INDEX(TEMP_PAGE_VIRT);
    
    pde_t temp_pde = kernel_dir.entries[temp_pd_idx];
    page_table_t *temp_table = (page_table_t *)(temp_pde & ~0xFFF);
    temp_table->entries[temp_pt_idx] = phys | PAGE_PRESENT | PAGE_WRITABLE;
    tlb_flush(TEMP_PAGE_VIRT);

    page_table_t *table = (page_table_t *)TEMP_PAGE_VIRT;
    for (uint32_t i = 0; i < PAGE_TBL_SIZE; i++)
        table->entries[i] = 0;

    pd->entries[pd_idx] = phys | PAGE_PRESENT | flags;
    return table;
}

void vmm_map(page_dir_t *pd, uint32_t virt, uint32_t phys, uint32_t flags)
{
    uint32_t pd_idx = PD_INDEX(virt);
    uint32_t pt_idx = PT_INDEX(virt);
    page_table_t *table = get_or_create_table(pd, pd_idx, flags);
    table->entries[pt_idx] = (phys & ~0xFFF) | PAGE_PRESENT | flags;
    tlb_flush(virt);
}

void vmm_unmap(page_dir_t *pd, uint32_t virt)
{
    pde_t pde = pd->entries[PD_INDEX(virt)];
    if (!(pde & PAGE_PRESENT)) return;
    page_table_t *table = (page_table_t *)(pde & ~0xFFF);
    table->entries[PT_INDEX(virt)] = 0;
    tlb_flush(virt);
}

bool vmm_is_mapped(page_dir_t *pd, uint32_t virt)
{
    pde_t pde = pd->entries[PD_INDEX(virt)];
    if (!(pde & PAGE_PRESENT)) return false;
    page_table_t *table = (page_table_t *)(pde & ~0xFFF);
    return (table->entries[PT_INDEX(virt)] & PAGE_PRESENT) != 0;
}

uint32_t vmm_get_phys(page_dir_t *pd, uint32_t virt)
{
    pde_t pde = pd->entries[PD_INDEX(virt)];
    if (!(pde & PAGE_PRESENT)) return 0;
    page_table_t *table = (page_table_t *)(pde & ~0xFFF);
    pte_t pte = table->entries[PT_INDEX(virt)];
    if (!(pte & PAGE_PRESENT)) return 0;
    return (pte & ~0xFFF) | PAGE_OFFSET(virt);
}

page_dir_t *vmm_create_dir(void)
{
    uint32_t phys = pmm_alloc();
    if (!phys) PANIC("VMM: out of memory creating page directory");
    page_dir_t *pd = (page_dir_t *)phys;
    for (uint32_t i = 0; i < PAGE_DIR_SIZE; i++)
        pd->entries[i] = 0;
    return pd;
}

void vmm_switch_dir(page_dir_t *pd)
{
    current_dir = pd;
    cr3_write((uint32_t)pd);
}

page_dir_t *vmm_get_current_dir(void) { return current_dir; }

void vmm_enable(void) { cr0_set_pg(); }

void vmm_init(void)
{
    for (uint32_t i = 0; i < PAGE_DIR_SIZE; i++)
        kernel_dir.entries[i] = 0;

    /* Pre-allocate page tables for lower memory identity mapping.
     * We need to create these manually before using vmm_map() */
    page_table_t *pt0 = (page_table_t *)pmm_alloc();
    page_table_t *pt1 = (page_table_t *)pmm_alloc();
    
    if (!pt0 || !pt1) PANIC("VMM: out of memory pre-allocating page tables");

    /* Initialize page tables */
    for (uint32_t i = 0; i < PAGE_TBL_SIZE; i++) {
        pt0->entries[i] = 0;
        pt1->entries[i] = 0;
    }

    /* Create page directory entries pointing to these page tables */
    kernel_dir.entries[0] = ((uint32_t)pt0) | PAGE_PRESENT | PAGE_WRITABLE;
    kernel_dir.entries[1] = ((uint32_t)pt1) | PAGE_PRESENT | PAGE_WRITABLE;

    /* Now fill in the page table entries for identity mapping 0x0 to 0x800000 */
    uint32_t paddr = 0;
    for (uint32_t va = 0; va < 0x800000; va += PAGE_SIZE) {
        uint32_t pt_idx = PT_INDEX(va);
        uint32_t pd_idx = PD_INDEX(va);
        page_table_t *pt = (pd_idx == 0) ? pt0 : pt1;
        pt->entries[pt_idx] = paddr | PAGE_PRESENT | PAGE_WRITABLE;
        paddr += PAGE_SIZE;
    }

    klog("[VMM] Identity map: 0x0 - 0x800000\n");
    cr3_write((uint32_t)&kernel_dir);
    current_dir = &kernel_dir;
    cr0_set_pg();
    klog("[VMM] Paging enabled\n");
}

#endif /* ARCH_X86 */

/*---------------------------------------------------------------------------
 * ARM VMM — pakai MMU AArch64
 *---------------------------------------------------------------------------*/
#ifdef ARCH_ARM

/*
 * ARM64 pakai 4-level page table (PGD → PUD → PMD → PTE).
 * Untuk sekarang kita skip paging — jalankan dengan MMU disabled.
 * Ini cukup untuk bring-up awal di QEMU/RPi.
 * MMU ARM akan diimplementasikan setelah kernel ARM stable.
 */

void vmm_init(void)
{
    /* MMU disabled untuk sekarang — bare metal tanpa paging */
    klog("[VMM] ARM: running without MMU (bare metal)\n");
}

/* Stub functions untuk ARM — belum diimplementasikan */
void vmm_map(void *pd, uint32_t virt, uint32_t phys, uint32_t flags)
{
    (void)pd; (void)virt; (void)phys; (void)flags;
}

void vmm_unmap(void *pd, uint32_t virt)
{
    (void)pd; (void)virt;
}

bool vmm_is_mapped(void *pd, uint32_t virt)
{
    (void)pd; (void)virt;
    return true; /* assume mapped tanpa MMU */
}

void *vmm_get_current_dir(void) { return NULL; }
void  vmm_switch_dir(void *pd)  { (void)pd; }
void  vmm_enable(void)          {}

#endif /* ARCH_ARM */