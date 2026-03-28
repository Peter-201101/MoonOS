#ifndef VESA_HPP
#define VESA_HPP

#include <types.hpp>

namespace Graphics {

// =============== VESA Structures ===============

struct VesaModeInfo {
    uint16_t attributes;
    uint8_t  win_a_attributes;
    uint8_t  win_b_attributes;
    uint16_t win_granularity;
    uint16_t win_size;
    uint16_t win_a_segment;
    uint16_t win_b_segment;
    uint32_t win_func_ptr;
    uint16_t bytes_per_scanline;
    
    uint16_t x_resolution;
    uint16_t y_resolution;
    uint8_t  x_char_size;
    uint8_t  y_char_size;
    uint8_t  number_of_planes;
    uint8_t  bits_per_pixel;
    uint8_t  number_of_banks;
    uint8_t  memory_model;
    uint8_t  bank_size;
    uint8_t  number_of_image_pages;
    uint8_t  reserved;
    
    // VBE 1.2+ info
    uint8_t red_mask_size;
    uint8_t red_field_position;
    uint8_t green_mask_size;
    uint8_t green_field_position;
    uint8_t blue_mask_size;
    uint8_t blue_field_position;
    uint8_t reserved_mask_size;
    uint8_t reserved_field_position;
    uint8_t direct_color_mode_info;
    
    // VBE 2.0+ info
    uint32_t phys_base_ptr;
    uint32_t reserved_32;
    uint16_t reserved_16;
    
    // VBE 3.0
    uint16_t linear_bytes_per_scanline;
    uint8_t  bnk_number_of_image_pages;
    uint8_t  lin_number_of_image_pages;
    uint8_t  lin_red_mask_size;
    uint8_t  lin_red_field_position;
    uint8_t  lin_green_mask_size;
    uint8_t  lin_green_field_position;
    uint8_t  lin_blue_mask_size;
    uint8_t  lin_blue_field_position;
    uint8_t  lin_reserved_mask_size;
    uint8_t  lin_reserved_field_position;
    uint32_t max_pixel_clock;
    
    uint8_t  reserved_end[190];
} __attribute__((packed));

struct MultibootFramebuffer {
    uint32_t type;          // 0=indexed, 1=RGB
    uint32_t addr;
    uint32_t pitch;
    uint32_t width;
    uint32_t height;
    uint8_t  bpp;
    // ... more fields for RGB/indexed mode
} __attribute__((packed));

// =============== Graphics State ===============

struct GraphicsState {
    uint32_t frame_buffer;  // Linear framebuffer address
    uint16_t width;
    uint16_t height;
    uint16_t pitch;         // Bytes per scanline
    uint8_t  bpp;           // Bits per pixel
    bool     initialized;
};

// =============== Color Definitions ===============

namespace Colors {
    // Catppuccin Mocha Theme (24-bit)
    const uint32_t BACKGROUND  = 0x1E1E2E; // Dark purple
    const uint32_t FOREGROUND  = 0xCDD6F4; // Light text
    const uint32_t ACCENT      = 0x89B4FA; // Blue
    const uint32_t BUTTON_BG   = 0x313244; // Darker background
    const uint32_t BUTTON_FG   = 0xCDD6F4; // Button text
    const uint32_t HIGHLIGHT   = 0xA6E3A1; // Green (hover)
    const uint32_t DANGER      = 0xF38BA8; // Red (delete)
    const uint32_t BORDER      = 0x45475A; // Gray border
    const uint32_t TEXT        = 0xCDD6F4; // Text
    
    // Grayscale fallback
    const uint32_t WHITE   = 0xFFFFFF;
    const uint32_t BLACK   = 0x000000;
    const uint32_t GRAY    = 0x808080;
}

// =============== Graphics API ===============

class Framebuffer {
private:
    GraphicsState state;
    uint32_t cursor_x;
    uint32_t cursor_y;
    
public:
    Framebuffer();
    
    // Initialization
    bool init_from_multiboot(const MultibootFramebuffer* mbt);
    bool detect_vesa();
    
    // Basic drawing
    void clear(uint32_t color);
    void draw_pixel(int x, int y, uint32_t color);
    void draw_rectangle(int x, int y, int w, int h, uint32_t color);
    void fill_rectangle(int x, int y, int w, int h, uint32_t color);
    void draw_border(int x, int y, int w, int h, uint32_t color, int thickness = 1);
    
    // Text drawing (basic bitmap font)
    void draw_char(int x, int y, char c, uint32_t fg, uint32_t bg = 0);
    void draw_string(int x, int y, const char* str, uint32_t fg, uint32_t bg = 0);
    
    // Cursor
    void set_cursor(int x, int y);
    void draw_cursor(uint32_t color = Colors::FOREGROUND);
    
    // Getters
    uint16_t get_width() const { return state.width; }
    uint16_t get_height() const { return state.height; }
    uint8_t  get_bpp() const { return state.bpp; }
    bool     is_initialized() const { return state.initialized; }
    
    // Line drawing (Bresenham's)
    void draw_line(int x1, int y1, int x2, int y2, uint32_t color);
    
    // Utility
    void scroll_up(int pixels, uint32_t bg_color);
};

// =============== Global Instance ===============
extern Framebuffer g_framebuffer;

// =============== Inline Drawing Helpers ===============

inline void clear_screen(uint32_t color = Colors::BACKGROUND) {
    if (g_framebuffer.is_initialized()) {
        g_framebuffer.clear(color);
    }
}

inline void put_char(int x, int y, char c, uint32_t fg = Colors::TEXT) {
    if (g_framebuffer.is_initialized()) {
        g_framebuffer.draw_char(x, y, c, fg);
    }
}

inline void put_string(int x, int y, const char* str, uint32_t fg = Colors::TEXT) {
    if (g_framebuffer.is_initialized()) {
        g_framebuffer.draw_string(x, y, str, fg);
    }
}

}; // namespace Graphics

#endif // VESA_HPP
