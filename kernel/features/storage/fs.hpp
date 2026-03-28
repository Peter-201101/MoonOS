#pragma once
#include <types.hpp>

#define MOONFS_MAGIC 0x4D4F4F4E // "MOON"
#define MAX_FILES 64
#define MAX_PATH_LEN 256

namespace FS {
    
    // File types
    enum class FileType : uint8_t {
        TYPE_FILE = 0,
        TYPE_DIR = 1,
        TYPE_EXECUTABLE = 2
    };

    struct FileEntry {
        char     name[32];          // Filename only, no path
        uint32_t start_lba;         // For actual disk files
        uint32_t size;              // File size in bytes
        FileType type;              // File/Directory/Executable
        uint8_t  present;           // 1 if entry is valid
        uint32_t parent_inode;      // Parent directory inode (for hierarchy)
        uint32_t inode;             // Unique identifier
    };

    struct Superblock {
        uint32_t magic;
        uint32_t file_count;
        FileEntry files[MAX_FILES];
    };

    // Initialize filesystem
    void init();

    // Directory operations
    bool mkdir(const char* dirname);           // Create directory
    bool mkdir_root();                         // Create root directory (for setup)
    bool chdir(const char* dirname);           // Change directory
    bool pwd(char* path_buffer);               // Get current working directory

    // File operations
    bool list_files();                         // List current directory (ls)
    bool cat(const char* filename);            // Show file contents
    bool touch(const char* filename);          // Create empty file
    bool rm(const char* filename);             // Delete file/directory
    
    // Utility
    void format();
    bool file_exists(const char* filename);
    uint32_t get_current_dir();

} // namespace FS