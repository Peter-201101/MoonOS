#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <types.h>

#define MULTIBOOT_MAGIC             0x1BADB002
#define MULTIBOOT_BOOTLOADER_MAGIC  0x2BADB002

#define MULTIBOOT_FLAG_ALIGN    (1 << 0)
#define MULTIBOOT_FLAG_MEMINFO  (1 << 1)

#define MULTIBOOT_INFO_MEMORY   (1 << 0)
#define MULTIBOOT_INFO_MMAP     (1 << 6)

#define MULTIBOOT_MMAP_AVAILABLE 1
#define MULTIBOOT_MMAP_RESERVED  2

typedef struct {
    uint32_t size;
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
} __attribute__((packed)) multiboot_mmap_t;

typedef struct {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
    uint32_t drives_length;
    uint32_t drives_addr;
    uint32_t config_table;
    uint32_t boot_loader_name;
    uint32_t apm_table;
} __attribute__((packed)) multiboot_info_t;

#endif