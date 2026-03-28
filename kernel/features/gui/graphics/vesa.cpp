#include "vesa.hpp"
#include <io/serial.hpp>

namespace Graphics {

// Global framebuffer instance
Framebuffer g_framebuffer;

// =============== Simple Bitmap Font (4x8 pixels) ===============
// ASCII printable characters 32-126

const uint8_t FONT_WIDTH = 4;
const uint8_t FONT_HEIGHT = 8;

// Each character is 4 bytes (4x8 pixels, 1 bit per pixel)
const uint8_t SIMPLE_FONT[] = {
    // Space (32)
    0x00, 0x00, 0x00, 0x00,
    // ! (33)
    0x44, 0x44, 0x44, 0x04,
    // " (34)
    0xAA, 0xAA, 0x00, 0x00,
    // # (35)
    0xAA, 0xFF, 0xAA, 0xFF,
    // $ (36)
    0x4E, 0xA4, 0x2A, 0x72,
    // % (37)
    0x88, 0x22, 0x88, 0x22,
    // & (38)
    0x44, 0xAA, 0x44, 0xAA,
    // ' (39)
    0x44, 0x44, 0x00, 0x00,
    // ( (40)
    0x24, 0x44, 0x44, 0x24,
    // ) (41)
    0x42, 0x22, 0x22, 0x42,
    // * (42)
    0x44, 0xFF, 0x44, 0xFF,
    // + (43)
    0x04, 0x4E, 0x04, 0x00,
    // , (44)
    0x00, 0x00, 0x44, 0x24,
    // - (45)
    0x00, 0x0E, 0x00, 0x00,
    // . (46)
    0x00, 0x00, 0x00, 0x44,
    // / (47)
    0x08, 0x84, 0x22, 0x01,
    // 0-9 (48-57)
    0x44, 0xAA, 0xAA, 0x44,  // 0
    0x24, 0x24, 0x24, 0x24,  // 1
    0x4E, 0x22, 0x8E, 0xEE,  // 2
    0x4E, 0x26, 0x22, 0x4E,  // 3
    0xA4, 0xAE, 0x24, 0x24,  // 4
    0xEE, 0x8E, 0x22, 0x4E,  // 5
    0x4E, 0x8A, 0xAE, 0x4E,  // 6
    0xEE, 0x22, 0x44, 0x44,  // 7
    0x4E, 0xAE, 0xAA, 0x4E,  // 8
    0x4E, 0xAE, 0x22, 0x4E,  // 9
    // : (58)
    0x00, 0x44, 0x00, 0x44,
    // ; (59)
    0x00, 0x44, 0x00, 0x24,
    // < (60)
    0x22, 0x44, 0x44, 0x22,
    // = (61)
    0x0E, 0x00, 0x0E, 0x00,
    // > (62)
    0x44, 0x22, 0x22, 0x44,
    // ? (63)
    0x4E, 0x22, 0x04, 0x04,
    // @ (64) - simplified
    0x4E, 0xBA, 0x8A, 0x4E,
    // A-Z (65-90)
    0x44, 0xAA, 0xAE, 0xAA,  // A
    0xCE, 0xAA, 0xAC, 0xEE,  // B
    0x4E, 0x88, 0x88, 0x4E,  // C
    0xCE, 0xAA, 0xAA, 0xCE,  // D
    0xEE, 0x8E, 0x88, 0xEE,  // E
    0xEE, 0x8E, 0x88, 0x88,  // F
    0x4E, 0x8A, 0xAA, 0x4E,  // G
    0xAA, 0xAE, 0xAA, 0xAA,  // H
    0xEE, 0x44, 0x44, 0xEE,  // I
    0x22, 0x22, 0xAA, 0x44,  // J
    0xAA, 0xAC, 0xAC, 0xAA,  // K
    0x88, 0x88, 0x88, 0xEE,  // L
    0xAA, 0xEA, 0xAA, 0xAA,  // M
    0xAA, 0xBA, 0x9A, 0xAA,  // N
    0x44, 0xAA, 0xAA, 0x44,  // O
    0xCE, 0xAA, 0xAC, 0x88,  // P
    0x44, 0xAA, 0xBA, 0x42,  // Q
    0xCE, 0xAA, 0xAC, 0xAA,  // R
    0x4E, 0x8A, 0x24, 0x4E,  // S
    0xEE, 0x44, 0x44, 0x44,  // T
    0xAA, 0xAA, 0xAA, 0x4E,  // U
    0xAA, 0xAA, 0x44, 0x44,  // V
    0xAA, 0xAA, 0xEA, 0xAA,  // W
    0xAA, 0x44, 0x44, 0xAA,  // X
    0xAA, 0x44, 0x44, 0x44,  // Y
    0xEE, 0x24, 0x84, 0xEE,  // Z
    // [ (91)
    0x6E, 0x88, 0x88, 0x6E,
    // \ (92)
    0x88, 0x44, 0x22, 0x11,
    // ] (93)
    0x6E, 0x22, 0x22, 0x6E,
    // ^ (94)
    0x4A, 0xAA, 0x00, 0x00,
    // _ (95)
    0x00, 0x00, 0x00, 0xEE,
    // ` (96)
    0x44, 0x22, 0x00, 0x00,
    // a-z (97-122)
    0x00, 0x4E, 0xAE, 0x4A,  // a
    0x88, 0xCE, 0xAA, 0xCE,  // b
    0x00, 0x4E, 0x88, 0x4E,  // c
    0x22, 0x4E, 0xAE, 0x4E,  // d
    0x00, 0x4E, 0xAE, 0x46,  // e
    0x24, 0x4E, 0x44, 0x44,  // f
    0x00, 0x4E, 0xAE, 0x4E,  // g - simplified
    0x88, 0xCE, 0xAA, 0xAA,  // h
    0x04, 0x04, 0x44, 0x4E,  // i
    0x02, 0x02, 0x2A, 0x44,  // j
    0x88, 0xAC, 0xAC, 0xAA,  // k
    0x44, 0x44, 0x44, 0x44,  // l
    0x00, 0xEA, 0xAA, 0xAA,  // m
    0x00, 0xCE, 0xAA, 0xAA,  // n
    0x00, 0x44, 0xAA, 0x44,  // o
    0x00, 0xCE, 0xAA, 0xCE,  // p - simplified
    0x00, 0x4E, 0xAE, 0x4E,  // q - simplified
    0x00, 0xCE, 0x88, 0x88,  // r
    0x00, 0x4E, 0x4E, 0x4E,  // s
    0x44, 0x4E, 0x44, 0x24,  // t
    0x00, 0xAA, 0xAA, 0x4E,  // u
    0x00, 0xAA, 0x44, 0x44,  // v
    0x00, 0xAA, 0xEA, 0xAA,  // w
    0x00, 0xAA, 0x44, 0xAA,  // x
    0x00, 0xAA, 0x44, 0x44,  // y - simplified
    0x00, 0xEE, 0x44, 0xEE,  // z
    // { (123)
    0x24, 0x44, 0x44, 0x24,
    // | (124)
    0x44, 0x44, 0x44, 0x44,
    // } (125)
    0x44, 0x22, 0x22, 0x44,
    // ~ (126)
    0x2A, 0xA4, 0x00, 0x00,
};

// =============== Framebuffer Implementation ===============

Framebuffer::Framebuffer() 
    : cursor_x(0), cursor_y(0)
{
    // Manually zero out state
    state.frame_buffer = 0;
    state.width = 0;
    state.height = 0;
    state.pitch = 0;
    state.bpp = 0;
    state.initialized = false;
}

bool Framebuffer::init_from_multiboot(const MultibootFramebuffer* mbt)
{
    if (!mbt || mbt->type != 1) { // 1 = RGB, 0 = indexed
        return false;
    }
    
    state.frame_buffer = mbt->addr;
    state.width = mbt->width;
    state.height = mbt->height;
    state.pitch = mbt->pitch;
    state.bpp = mbt->bpp;
    state.initialized = true;
    
    Serial::writeln("[VESA] Initialized from multiboot");
    Serial::write  ("[VESA] Mode: ");
    Serial::write_dec(state.width);
    Serial::write  ("x");
    Serial::write_dec(state.height);
    Serial::write  (" @ ");
    Serial::write_dec(state.bpp);
    Serial::writeln(" bpp");
    
    clear(Colors::BACKGROUND);
    return true;
}

void Framebuffer::clear(uint32_t color)
{
    if (!state.initialized) return;
    
    uint32_t* fb = (uint32_t*)(uint64_t)state.frame_buffer;
    uint32_t pixel_count = (state.pitch / (state.bpp / 8)) * state.height;
    
    for (uint32_t i = 0; i < pixel_count; i++) {
        fb[i] = color;
    }
}

void Framebuffer::draw_pixel(int x, int y, uint32_t color)
{
    if (!state.initialized) return;
    if (x < 0 || x >= (int)state.width || y < 0 || y >= (int)state.height) return;
    
    uint32_t offset = (y * state.pitch) / (state.bpp / 8) + x;
    uint32_t* fb = (uint32_t*)(uint64_t)state.frame_buffer;
    fb[offset] = color;
}

void Framebuffer::fill_rectangle(int x, int y, int w, int h, uint32_t color)
{
    if (!state.initialized) return;
    
    for (int py = y; py < y + h; py++) {
        for (int px = x; px < x + w; px++) {
            draw_pixel(px, py, color);
        }
    }
}

void Framebuffer::draw_rectangle(int x, int y, int w, int h, uint32_t color)
{
    if (!state.initialized) return;
    
    // Top and bottom
    for (int px = x; px < x + w; px++) {
        draw_pixel(px, y, color);
        draw_pixel(px, y + h - 1, color);
    }
    
    // Left and right
    for (int py = y; py < y + h; py++) {
        draw_pixel(x, py, color);
        draw_pixel(x + w - 1, py, color);
    }
}

void Framebuffer::draw_border(int x, int y, int w, int h, uint32_t color, int thickness)
{
    if (!state.initialized) return;
    
    for (int t = 0; t < thickness; t++) {
        draw_rectangle(x + t, y + t, w - 2*t, h - 2*t, color);
    }
}

void Framebuffer::draw_char(int x, int y, char c, uint32_t fg, uint32_t bg)
{
    if (!state.initialized) return;
    if (c < 32 || c > 126) return;
    
    // Draw background
    if (bg != 0) {
        fill_rectangle(x, y, FONT_WIDTH, FONT_HEIGHT, bg);
    }
    
    // Get character bitmap from font
    int char_idx = (int)(c - 32);
    const uint8_t* char_bitmap = SIMPLE_FONT + (char_idx * 4);
    
    // Draw font bits
    for (int py = 0; py < FONT_HEIGHT; py++) {
        uint8_t row = char_bitmap[py % 4];
        for (int px = 0; px < FONT_WIDTH; px++) {
            if (row & (1 << (FONT_WIDTH - 1 - px))) {
                draw_pixel(x + px, y + py, fg);
            }
        }
    }
}

void Framebuffer::draw_string(int x, int y, const char* str, uint32_t fg, uint32_t bg)
{
    if (!str) return;
    
    int curr_x = x;
    for (const char* p = str; *p; p++) {
        if (*p == '\n') {
            curr_x = x;
            y += FONT_HEIGHT;
            continue;
        }
        
        draw_char(curr_x, y, *p, fg, bg);
        curr_x += FONT_WIDTH;
    }
}

void Framebuffer::set_cursor(int x, int y)
{
    cursor_x = x;
    cursor_y = y;
}

void Framebuffer::draw_cursor(uint32_t color)
{
    if (!state.initialized) return;
    
    fill_rectangle(cursor_x, cursor_y + FONT_HEIGHT - 2, FONT_WIDTH, 2, color);
}

void Framebuffer::draw_line(int x1, int y1, int x2, int y2, uint32_t color)
{
    // Bresenham's line algorithm
    int dx = (x2 > x1) ? (x2 - x1) : (x1 - x2);
    int dy = (y2 > y1) ? (y2 - y1) : (y1 - y2);
    int sx = (x2 > x1) ? 1 : -1;
    int sy = (y2 > y1) ? 1 : -1;
    int err = dx - dy;
    
    int x = x1, y = y1;
    while (true) {
        draw_pixel(x, y, color);
        
        if (x == x2 && y == y2) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }
}

void Framebuffer::scroll_up(int pixels, uint32_t bg_color)
{
    if (!state.initialized || pixels <= 0) return;
    
    uint8_t* fb = (uint8_t*)(uint64_t)state.frame_buffer;
    uint32_t copy_height = state.height - pixels;
    
    // Copy pixels up
    for (uint32_t y = 0; y < copy_height; y++) {
        uint8_t* src = fb + ((y + pixels) * state.pitch);
        uint8_t* dst = fb + (y * state.pitch);
        for (uint32_t x = 0; x < state.pitch; x++) {
            dst[x] = src[x];
        }
    }
    
    // Fill bottom with bg color
    fill_rectangle(0, state.height - pixels, state.width, pixels, bg_color);
}

}; // namespace Graphics
