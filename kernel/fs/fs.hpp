#pragma once
#include <types.hpp>

#define MOONFS_MAGIC 0x4D4F4F4E // "MOON"
#define MAX_FILES 16

namespace FS {
    struct FileEntry {
        char     name[32];
        uint32_t start_lba;
        uint32_t size_sectors;
        uint8_t  type; // 0: Data, 1: Executable
        uint8_t  present;
    };

    struct Superblock {
        uint32_t magic;
        uint32_t file_count;
        FileEntry files[MAX_FILES];
    };

    void init();
    bool list_files();
    void format();
    bool load_and_run(const char* name);
}