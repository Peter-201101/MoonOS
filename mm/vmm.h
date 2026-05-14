#ifndef VMM_H
#define VMM_H

#include <types.h>

#define PAGE_PRESENT    (1 << 0)
#define PAGE_WRITABLE   (1 << 1)
#define PAGE_USER       (1 << 2)

#ifdef ARCH_X86

#define PAGE_DIR_SIZE   1024
#define PAGE_TBL_SIZE   1024
#define PAGE_SHIFT      12

#define PD_INDEX(va)    (((va) >> 22) & 0x3FF)
#define PT_INDEX(va)    (((va) >> 12) & 0x3FF)
#define PAGE_OFFSET(va) ((va) & 0xFFF)

typedef uint32_t pde_t;
typedef uint32_t pte_t;

typedef struct {
    pde_t entries[PAGE_DIR_SIZE];
} __attribute__((aligned(4096))) page_dir_t;

typedef struct {
    pte_t entries[PAGE_TBL_SIZE];
} __attribute__((aligned(4096))) page_table_t;

void        vmm_init(void);
void        vmm_map(page_dir_t *pd, uint32_t virt, uint32_t phys, uint32_t flags);
void        vmm_unmap(page_dir_t *pd, uint32_t virt);
bool        vmm_is_mapped(page_dir_t *pd, uint32_t virt);
uint32_t    vmm_get_phys(page_dir_t *pd, uint32_t virt);
page_dir_t *vmm_create_dir(void);
void        vmm_switch_dir(page_dir_t *pd);
page_dir_t *vmm_get_current_dir(void);
void        vmm_enable(void);

#endif /* ARCH_X86 */

#ifdef ARCH_ARM

void  vmm_init(void);
void  vmm_map(void *pd, uint32_t virt, uint32_t phys, uint32_t flags);
void  vmm_unmap(void *pd, uint32_t virt);
bool  vmm_is_mapped(void *pd, uint32_t virt);
void *vmm_get_current_dir(void);
void  vmm_switch_dir(void *pd);
void  vmm_enable(void);

#endif /* ARCH_ARM */

#endif /* VMM_H */