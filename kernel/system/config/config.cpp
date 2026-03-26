#include "config.hpp"
#include "../../utils/string.hpp" // Sesuaikan path-nya

static SystemConfig config;

void Config::init() {
    String::memset(&config, 0, sizeof(SystemConfig));
}

SystemConfig* Config::get() {
    return &config;
}

void Config::set(const SystemConfig& cfg) {
    // Gunakan fungsi buatanmu sendiri daripada operator '=' 
    // Ini menghindari compiler menggunakan instruksi SSE yang sensitif alignment
    String::memcpy(&config, &cfg, sizeof(SystemConfig));
}