#ifndef MOUSE_HPP
#define MOUSE_HPP

#include <types.hpp>

namespace Mouse {

// =============== Mouse State ===============

struct State {
    int16_t x;              // Absolute X position
    int16_t y;              // Absolute Y position
    bool left_button;       // Left button pressed
    bool right_button;      // Right button pressed
    bool middle_button;     // Middle button pressed
};

// =============== Mouse Driver ===============

class Handler {
private:
    static const int BUFFER_SIZE = 256;
    
    uint8_t packet_buffer[3];  // PS/2 mouse sends 3-byte packets
    int packet_index;
    State current_state;
    bool initialized;
    
    // IRQ12 handler
    static uint8_t irq12_count;
    
public:
    Handler();
    
    // Init PS/2 mouse
    void init();
    
    // Called from IRQ12 handler
    void on_byte_received(uint8_t byte);
    
    // Get current mouse state
    State get_state() const { return current_state; }
    
    bool is_initialized() const { return initialized; }
    
private:
    void send_command(uint8_t cmd);
    uint8_t read_response();
    void wait_for_input();
};

extern Handler g_mouse;

}; // namespace Mouse

#endif // MOUSE_HPP
