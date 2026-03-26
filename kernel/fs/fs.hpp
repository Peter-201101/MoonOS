#pragma once

#define MAX_FILES 32
#define MAX_FILE_SIZE 256

struct File {
    char name[32];
    char data[MAX_FILE_SIZE];
    int size;
    bool used;
};

namespace FS {
    void init();

    bool create(const char* name);
    bool write(const char* name, const char* data);
    const char* read(const char* name);

    void list();
}