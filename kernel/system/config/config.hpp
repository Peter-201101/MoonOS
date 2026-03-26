#pragma once

// Tambahkan alignas(16) agar compiler tidak crash saat copy struct
struct alignas(16) SystemConfig {
    char username[32];
    char password[32];
    char hostname[32];
    char timezone[32];
};

namespace Config {
    void init();
    SystemConfig* get();
    void set(const SystemConfig& cfg);
}