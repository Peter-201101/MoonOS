#include "fs.hpp"
#include <storage/ata.hpp>
#include <io/serial.hpp>
#include <string.hpp>

static FS::Superblock sblock;
static uint32_t current_dir_inode = 0;  // Root dir inode
static uint32_t next_inode = 1;         // Next available inode

void FS::init() {
    uint8_t temp[1024]; // Buffer untuk 2 sektor
    if (ATA::read_sector(2, temp) && ATA::read_sector(3, temp + 512)) {
        String::memcpy(&sblock, temp, sizeof(FS::Superblock));
        if (sblock.magic == MOONFS_MAGIC) {
            Serial::writeln("[FS] MoonFS Mounted.");
            // Calculate next available inode
            for (int i = 0; i < MAX_FILES; i++) {
                if (sblock.files[i].present && sblock.files[i].inode >= next_inode) {
                    next_inode = sblock.files[i].inode + 1;
                }
            }
        } else {
            Serial::writeln("[FS] Magic Mismatch! Formatting...");
            format();
            // Create root directory
            String::memset(&sblock.files[0], 0, sizeof(FileEntry));
            sblock.files[0].inode = next_inode++;
            String::memcpy(sblock.files[0].name, "/", 2);
            sblock.files[0].type = FileType::TYPE_DIR;
            sblock.files[0].present = 1;
            sblock.files[0].parent_inode = 0;  // Root is its own parent
            sblock.file_count = 1;
        }
    }
}

void FS::format() {
    String::memset(&sblock, 0, sizeof(FS::Superblock));
    sblock.magic = MOONFS_MAGIC;
    sblock.file_count = 0;
    
    uint8_t* p = (uint8_t*)&sblock;
    if (ATA::write_sector(2, p) && ATA::write_sector(3, p + 512)) {
        Serial::writeln("[FS] Disk Formatted Successfully.");
    }
}

bool FS::mkdir_root() {
    // Create root directory entry after format
    String::memset(&sblock.files[0], 0, sizeof(FileEntry));
    sblock.files[0].inode = next_inode++;
    String::memcpy(sblock.files[0].name, "/", 2);
    sblock.files[0].type = FileType::TYPE_DIR;
    sblock.files[0].present = 1;
    sblock.files[0].parent_inode = 0;  // Root is its own parent
    sblock.file_count = 1;
    
    // Save to disk immediately
    uint8_t* p = (uint8_t*)&sblock;
    if (ATA::write_sector(2, p) && ATA::write_sector(3, p + 512)) {
        Serial::writeln("[FS] Root directory created");
        return true;
    }
    return false;
}

// Internal: Save filesystem state to disk
static void save_filesystem_to_disk() {
    uint8_t* p = (uint8_t*)&sblock;
    ATA::write_sector(2, p);
    ATA::write_sector(3, p + 512);
}

bool FS::list_files() {
    Serial::writeln("\n--- File List ---");
    int found = 0;
    
    for (int i = 0; i < MAX_FILES; i++) {
        if (sblock.files[i].present && sblock.files[i].parent_inode == current_dir_inode) {
            Serial::write(sblock.files[i].name);
            
            // Add "/" suffix for directories, but skip if name already ends with /
            if (sblock.files[i].type == FileType::TYPE_DIR && 
                (sblock.files[i].name[0] != '/' || sblock.files[i].name[1] != '\0')) {
                Serial::write("/");
            }
            Serial::write(" (");
            Serial::write_dec(sblock.files[i].size);
            Serial::writeln(" bytes)");
            found++;
        }
    }
    
    if (found == 0) Serial::writeln("(Empty directory)");
    return true;
}

bool FS::mkdir(const char* dirname) {
    if (!dirname || dirname[0] == '\0') {
        Serial::writeln("[FS] Error: Directory name empty");
        return false;
    }
    
    // Check if already exists
    for (int i = 0; i < MAX_FILES; i++) {
        if (sblock.files[i].present && 
            sblock.files[i].parent_inode == current_dir_inode &&
            String::compare(sblock.files[i].name, dirname) == 0) {
            Serial::write("[FS] Error: Directory '");
            Serial::write(dirname);
            Serial::writeln("' already exists");
            return false;
        }
    }
    
    // Find empty slot
    for (int i = 0; i < MAX_FILES; i++) {
        if (!sblock.files[i].present) {
            String::memset(&sblock.files[i], 0, sizeof(FileEntry));
            String::memcpy(sblock.files[i].name, dirname, 31);
            sblock.files[i].type = FileType::TYPE_DIR;
            sblock.files[i].present = 1;
            sblock.files[i].inode = next_inode++;
            sblock.files[i].parent_inode = current_dir_inode;
            sblock.files[i].size = 0;
            sblock.file_count++;
            
            Serial::write("[FS] Directory created: ");
            Serial::writeln(dirname);
            save_filesystem_to_disk();
            return true;
        }
    }
    
    Serial::writeln("[FS] Error: No space for new directory");
    return false;
}

bool FS::chdir(const char* dirname) {
    if (!dirname || dirname[0] == '\0') {
        Serial::writeln("[FS] Error: Directory name empty");
        return false;
    }
    
    // Special case: go to root
    if (String::compare(dirname, "/") == 0) {
        current_dir_inode = 0;
        return true;
    }
    
    // Search for directory
    for (int i = 0; i < MAX_FILES; i++) {
        if (sblock.files[i].present &&
            sblock.files[i].parent_inode == current_dir_inode &&
            sblock.files[i].type == FileType::TYPE_DIR &&
            String::compare(sblock.files[i].name, dirname) == 0) {
            current_dir_inode = sblock.files[i].inode;
            Serial::write("[FS] Changed directory to: ");
            Serial::writeln(dirname);
            return true;
        }
    }
    
    Serial::write("[FS] Error: Directory '");
    Serial::write(dirname);
    Serial::writeln("' not found");
    return false;
}

bool FS::pwd(char* path_buffer) {
    if (!path_buffer) return false;
    
    if (current_dir_inode == 0) {
        String::memcpy(path_buffer, "/", 2);
        return true;
    }
    
    // Find directory name by inode
    for (int i = 0; i < MAX_FILES; i++) {
        if (sblock.files[i].present && sblock.files[i].inode == current_dir_inode) {
            String::memcpy(path_buffer, sblock.files[i].name, 31);
            return true;
        }
    }
    
    String::memcpy(path_buffer, "/", 2);
    return true;
}

bool FS::touch(const char* filename) {
    if (!filename || filename[0] == '\0') {
        Serial::writeln("[FS] Error: Filename empty");
        return false;
    }
    
    // Check if already exists
    for (int i = 0; i < MAX_FILES; i++) {
        if (sblock.files[i].present && 
            sblock.files[i].parent_inode == current_dir_inode &&
            String::compare(sblock.files[i].name, filename) == 0) {
            Serial::write("[FS] Error: File '");
            Serial::write(filename);
            Serial::writeln("' already exists");
            return false;
        }
    }
    
    // Find empty slot
    for (int i = 0; i < MAX_FILES; i++) {
        if (!sblock.files[i].present) {
            String::memset(&sblock.files[i], 0, sizeof(FileEntry));
            String::memcpy(sblock.files[i].name, filename, 31);
            sblock.files[i].type = FileType::TYPE_FILE;
            sblock.files[i].present = 1;
            sblock.files[i].inode = next_inode++;
            sblock.files[i].parent_inode = current_dir_inode;
            sblock.files[i].size = 0;
            sblock.file_count++;
            
            Serial::write("[FS] File created: ");
            Serial::writeln(filename);
            save_filesystem_to_disk();
            return true;
        }
    }
    
    Serial::writeln("[FS] Error: No space for new file");
    return false;
}

bool FS::cat(const char* filename) {
    if (!filename || filename[0] == '\0') {
        Serial::writeln("[FS] Error: Filename empty");
        return false;
    }
    
    // Find file
    for (int i = 0; i < MAX_FILES; i++) {
        if (sblock.files[i].present &&
            sblock.files[i].parent_inode == current_dir_inode &&
            sblock.files[i].type == FileType::TYPE_FILE &&
            String::compare(sblock.files[i].name, filename) == 0) {
            
            if (sblock.files[i].size == 0) {
                Serial::writeln("(empty file)");
                return true;
            }
            
            Serial::writeln("");
            return true;
        }
    }
    
    Serial::write("[FS] Error: File '");
    Serial::write(filename);
    Serial::writeln("' not found");
    return false;
}

bool FS::rm(const char* filename) {
    if (!filename || filename[0] == '\0') {
        Serial::writeln("[FS] Error: Filename empty");
        return false;
    }
    
    // Find file/directory
    for (int i = 0; i < MAX_FILES; i++) {
        if (sblock.files[i].present &&
            sblock.files[i].parent_inode == current_dir_inode &&
            String::compare(sblock.files[i].name, filename) == 0) {
            
            // Check if directory is empty
            if (sblock.files[i].type == FileType::TYPE_DIR) {
                for (int j = 0; j < MAX_FILES; j++) {
                    if (sblock.files[j].present &&
                        sblock.files[j].parent_inode == sblock.files[i].inode) {
                        Serial::writeln("[FS] Error: Directory not empty");
                        return false;
                    }
                }
            }
            
            sblock.files[i].present = 0;
            sblock.file_count--;
            
            Serial::write("[FS] Deleted: ");
            Serial::writeln(filename);
            save_filesystem_to_disk();
            return true;
        }
    }
    
    Serial::write("[FS] Error: '");
    Serial::write(filename);
    Serial::writeln("' not found");
    return false;
}

bool FS::file_exists(const char* filename) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (sblock.files[i].present &&
            sblock.files[i].parent_inode == current_dir_inode &&
            String::compare(sblock.files[i].name, filename) == 0) {
            return true;
        }
    }
    return false;
}

uint32_t FS::get_current_dir() {
    return current_dir_inode;
}