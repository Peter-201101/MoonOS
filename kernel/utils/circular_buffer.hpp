#pragma once

#include "../../include/types.hpp"

namespace Utils {

template<typename T, uint32_t MAX_SIZE = 256>
class CircularBuffer {
private:
    T buffer[MAX_SIZE];
    uint32_t head = 0;
    uint32_t tail = 0;
    uint32_t count = 0;

public:
    CircularBuffer() = default;

    bool push(T value) {
        if (count >= MAX_SIZE) return false;
        buffer[tail] = value;
        tail = (tail + 1) % MAX_SIZE;
        count++;
        return true;
    }

    bool pop(T& value) {
        if (count == 0) return false;
        value = buffer[head];
        head = (head + 1) % MAX_SIZE;
        count--;
        return true;
    }

    bool peek(T& value) const {
        if (count == 0) return false;
        value = buffer[head];
        return true;
    }

    bool is_empty() const {
        return count == 0;
    }

    bool is_full() const {
        return count >= MAX_SIZE;
    }

    uint32_t size() const {
        return count;
    }

    void clear() {
        head = 0;
        tail = 0;
        count = 0;
    }
};

} // namespace Utils
