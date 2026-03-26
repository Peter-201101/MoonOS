#pragma once
#include "../include/types.hpp"

namespace ATA {
    void init();

    bool read_sector(uint32_t lba, uint8_t* buffer);
    bool write_sector(uint32_t lba, const uint8_t* buffer);
}