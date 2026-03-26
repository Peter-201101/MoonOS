#include "fs.hpp"
#include <drivers/serial.hpp>

// ==============================
// Internal Storage
// ==============================
static File files[MAX_FILES];

// ==============================
// Helpers
// ==============================
static bool str_eq(const char* a, const char* b) {
    int i = 0;
    while (a[i] && b[i]) {
        if (a[i] != b[i]) return false;
        i++;
    }
    return a[i] == b[i];
}

static void str_copy(char* dst, const char* src, int max) {
    int i = 0;
    for (; i < max - 1 && src[i]; i++) {
        dst[i] = src[i];
    }
    dst[i] = '\0';
}

// ==============================
// Init
// ==============================
void FS::init() {
    for (int i = 0; i < MAX_FILES; i++) {
        files[i].used = false;
    }
}

// ==============================
// Create File
// ==============================
bool FS::create(const char* name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (!files[i].used) {
            str_copy(files[i].name, name, 32);
            files[i].size = 0;
            files[i].used = true;
            return true;
        }
    }
    return false;
}

// ==============================
// Write File
// ==============================
bool FS::write(const char* name, const char* data) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used && str_eq(files[i].name, name)) {

            int j = 0;
            for (; j < MAX_FILE_SIZE - 1 && data[j]; j++) {
                files[i].data[j] = data[j];
            }

            files[i].data[j] = '\0';
            files[i].size = j;
            return true;
        }
    }
    return false;
}

// ==============================
// Read File
// ==============================
const char* FS::read(const char* name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used && str_eq(files[i].name, name)) {
            return files[i].data;
        }
    }
    return nullptr;
}

// ==============================
// List Files
// ==============================
void FS::list() {
    Serial::writeln("Files:");

    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used) {
            Serial::write("- ");
            Serial::writeln(files[i].name);
        }
    }
}