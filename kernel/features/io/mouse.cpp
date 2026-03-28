#include "mouse.hpp"
#include "serial.hpp"

namespace Mouse {

// =============== Port I/O Utilities ===============

inline void outb(uint16_t port, uint8_t value) {
    asm volatile("outb %0, %1" : : "a"(value), "Nd"(port) : "memory");
}

inline uint8_t inb(uint16_t port) {
    uint8_t result;
    asm volatile("inb %1, %0" : "=a"(result) : "Nd"(port) : "memory");
    return result;
}

// =============== PS/2 Constants ===============

const uint16_t MOUSE_PORT = 0x60;  // Data port (shared with keyboard)
const uint16_t STATUS_PORT = 0x64;  // Status port
const uint16_t CMD_PORT = 0x64;     // Command port

const uint8_t STATUS_OUTPUT_BUFFER = 0x01;  // Data available
const uint8_t STATUS_INPUT_BUFFER = 0x02;   // Command buffer full
const uint8_t STATUS_MOUSE_DATA = 0x20;     // Mouse data in output buffer

// Mouse commands
const uint8_t MOUSE_SET_DEFAULTS = 0xF6;
const uint8_t MOUSE_ENABLE = 0xF4;
const uint8_t MOUSE_RESET = 0xFF;
const uint8_t MOUSE_ACK = 0xFA;

// PS/2 controller commands
const uint8_t PS2_ENABLE_MOUSE = 0xA8;
const uint8_t PS2_CMD_WRITE_CCB = 0x60;
const uint8_t PS2_CMD_READ_CCB = 0x20;
const uint8_t PS2_CMD_MOUSE_WRITE = 0xD4;

// =============== Global Instance ===============

Handler g_mouse;
uint8_t Handler::irq12_count = 0;

// =============== Handler Implementation ===============

Handler::Handler()
    : packet_index(0), initialized(false)
{
    current_state.x = 0;
    current_state.y = 0;
    current_state.left_button = false;
    current_state.right_button = false;
    current_state.middle_button = false;
}

void Handler::wait_for_input()
{
    // Wait for input buffer to be empty
    int timeout = 100000;
    while ((inb(STATUS_PORT) & STATUS_INPUT_BUFFER) && timeout--) {
        // Busy wait
    }
}

void Handler::send_command(uint8_t cmd)
{
    wait_for_input();
    outb(CMD_PORT, PS2_CMD_MOUSE_WRITE);
    wait_for_input();
    outb(MOUSE_PORT, cmd);
}

uint8_t Handler::read_response()
{
    int timeout = 100000;
    while (!(inb(STATUS_PORT) & STATUS_OUTPUT_BUFFER) && timeout--) {
        // Busy wait
    }
    return inb(MOUSE_PORT);
}

void Handler::init()
{
    Serial::writeln("[MOUSE] Initializing PS/2 mouse...");
    
    // Enable PS/2 mouse port
    outb(CMD_PORT, PS2_ENABLE_MOUSE);
    
    // Reset mouse
    send_command(MOUSE_RESET);
    read_response();  // Should be ACK
    uint8_t self_test = read_response();  // Self test result
    
    if (self_test != 0x00) {
        Serial::write("[MOUSE] Self test failed: ");
        Serial::write_dec(self_test);
        Serial::writeln("");
    }
    
    // Read device ID
    uint8_t device_id = read_response();
    Serial::write("[MOUSE] Device ID: ");
    Serial::write_dec(device_id);
    Serial::writeln("");
    
    // Set defaults
    send_command(MOUSE_SET_DEFAULTS);
    read_response();  // Should be ACK
    
    // Enable mouse
    send_command(MOUSE_ENABLE);
    uint8_t ack = read_response();  // Should be ACK
    
    if (ack == MOUSE_ACK) {
        initialized = true;
        Serial::writeln("[MOUSE] Mouse initialized successfully!");
    } else {
        Serial::writeln("[MOUSE] Mouse initialization failed!");
    }
}

void Handler::on_byte_received(uint8_t byte)
{
    if (!initialized) return;
    
    // Collect 3-byte packets
    packet_buffer[packet_index++] = byte;
    
    if (packet_index >= 3) {
        packet_index = 0;
        
        // Parse PS/2 mouse packet
        uint8_t status = packet_buffer[0];
        int8_t x_delta = packet_buffer[1];
        int8_t y_delta = packet_buffer[2];
        
        // Update button states
        current_state.left_button = (status & 0x01) != 0;
        current_state.right_button = (status & 0x02) != 0;
        current_state.middle_button = (status & 0x04) != 0;
        
        // Update position (adjust for screen if needed)
        current_state.x += x_delta;
        current_state.y -= y_delta;  // Y is inverted in PS/2
        
        // Clamp to screen bounds (assuming max 1024x768)
        if (current_state.x < 0) current_state.x = 0;
        if (current_state.x > 1024) current_state.x = 1024;
        if (current_state.y < 0) current_state.y = 0;
        if (current_state.y > 768) current_state.y = 768;
    }
}

}; // namespace Mouse
