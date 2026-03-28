#include "widgets.hpp"
#include <io/serial.hpp>

namespace GUI {

// =============== Button Implementation ===============

void Button::draw()
{
    if (!visible) return;
    
    uint32_t bg_color = (focused || pressed) ? Graphics::Colors::HIGHLIGHT : Graphics::Colors::BUTTON_BG;
    uint32_t text_color = Graphics::Colors::BUTTON_FG;
    
    // Draw button background
    Graphics::g_framebuffer.fill_rectangle(x, y, w, h, bg_color);
    
    // Draw button border
    Graphics::g_framebuffer.draw_border(x, y, w, h, Graphics::Colors::BORDER, 1);
    
    // Draw text centered
    if (label) {
        int text_x = x + 4;
        int text_y = y + (h - 8) / 2;  // Vertically center
        Graphics::g_framebuffer.draw_string(text_x, text_y, label, text_color);
    }
}

bool Button::on_event(const Event& e)
{
    if (!visible) return false;
    
    switch (e.type) {
        case EventType::MOUSE_CLICK:
            if (contains_point(e.data.mouse.x, e.data.mouse.y)) {
                pressed = true;
                return true;
            }
            break;
            
        case EventType::MOUSE_RELEASE:
            if (pressed) {
                pressed = false;
                if (contains_point(e.data.mouse.x, e.data.mouse.y)) {
                    if (on_click) {
                        on_click();
                    }
                    return true;
                }
                return false;
            }
            break;
            
        default:
            break;
    }
    
    return false;
}

// =============== TextBox Implementation ===============

void TextBox::set_text(const char* new_text)
{
    if (!new_text) {
        clear();
        return;
    }
    
    int len = 0;
    for (const char* p = new_text; *p && len < MAX_TEXT - 1; p++) {
        text[len++] = *p;
    }
    text[len] = '\0';
    cursor_pos = len;
    scroll_offset = 0;
}

void TextBox::draw()
{
    if (!visible) return;
    
    uint32_t bg_color = focused ? Graphics::Colors::BUTTON_BG : 0x2A2A2A;
    uint32_t text_color = Graphics::Colors::TEXT;
    
    // Draw background
    Graphics::g_framebuffer.fill_rectangle(x, y, w, h, bg_color);
    
    // Draw border
    uint32_t border_color = focused ? Graphics::Colors::ACCENT : Graphics::Colors::BORDER;
    Graphics::g_framebuffer.draw_border(x, y, w, h, border_color, 1);
    
    // Draw text with scroll
    int text_x = x + 4;
    int text_y = y + (h - 8) / 2;
    
    if (text[0] != '\0') {
        // Draw visible portion of text
        Graphics::g_framebuffer.draw_string(text_x, text_y, text + scroll_offset, text_color);
    } else if (placeholder) {
        // Draw placeholder in gray
        Graphics::g_framebuffer.draw_string(text_x, text_y, placeholder, 0x666666);
    }
    
    // Draw cursor if focused
    if (focused) {
        int cursor_screen_x = text_x + (cursor_pos - scroll_offset) * 4;
        Graphics::g_framebuffer.fill_rectangle(cursor_screen_x, text_y + 7, 1, 2, Graphics::Colors::ACCENT);
    }
}

bool TextBox::on_event(const Event& e)
{
    if (!visible || !focused) return false;
    
    switch (e.type) {
        case EventType::MOUSE_CLICK:
            if (contains_point(e.data.mouse.x, e.data.mouse.y)) {
                return true;
            }
            break;
            
        case EventType::KEY_PRESS: {
            char c = e.data.keyboard.ascii;
            
            if (c == '\b') {  // Backspace
                if (cursor_pos > 0) {
                    cursor_pos--;
                    for (int i = cursor_pos; text[i]; i++) {
                        text[i] = text[i + 1];
                    }
                    if (scroll_offset > cursor_pos) {
                        scroll_offset = cursor_pos;
                    }
                }
                return true;
            }
            else if (c == '\n' || c == '\r') {  // Enter
                return false;  // Let parent handle enter
            }
            else if (c >= 32 && c < 127 && cursor_pos < MAX_TEXT - 1) {  // Printable
                for (int i = cursor_pos + 1; i >= cursor_pos; i--) {
                    text[i + 1] = text[i];
                }
                text[cursor_pos] = c;
                cursor_pos++;
                return true;
            }
            break;
        }
        
        default:
            break;
    }
    
    return false;
}

// =============== Label Implementation ===============

void Label::draw()
{
    if (!visible) return;
    
    Graphics::g_framebuffer.draw_string(x, y, text, color);
}

// =============== ListBox Implementation ===============

void ListBox::draw()
{
    if (!visible) return;
    
    // Draw background
    Graphics::g_framebuffer.fill_rectangle(x, y, w, h, Graphics::Colors::BUTTON_BG);
    
    // Draw border
    Graphics::g_framebuffer.draw_border(x, y, w, h, Graphics::Colors::BORDER, 1);
    
    // Draw items
    int item_height = 10;
    int max_visible = (h - 4) / item_height;
    
    for (int i = 0; i < max_visible && (scroll_offset + i) < item_count; i++) {
        int item_y = y + 2 + (i * item_height);
        int item_idx = scroll_offset + i;
        
        // Highlight selected
        if (item_idx == selected_index) {
            Graphics::g_framebuffer.fill_rectangle(x + 2, item_y, w - 4, item_height, Graphics::Colors::ACCENT);
            Graphics::g_framebuffer.draw_string(x + 4, item_y + 1, items[item_idx], Graphics::Colors::BUTTON_BG);
        } else {
            Graphics::g_framebuffer.draw_string(x + 4, item_y + 1, items[item_idx], Graphics::Colors::TEXT);
        }
    }
}

bool ListBox::on_event(const Event& e)
{
    if (!visible) return false;
    
    switch (e.type) {
        case EventType::MOUSE_CLICK:
            if (contains_point(e.data.mouse.x, e.data.mouse.y)) {
                int item_height = 10;
                int relative_y = e.data.mouse.y - y - 2;
                int item_idx = scroll_offset + (relative_y / item_height);
                
                if (item_idx >= 0 && item_idx < item_count) {
                    selected_index = item_idx;
                    return true;
                }
            }
            break;
            
        case EventType::KEY_PRESS:
            if (e.data.keyboard.key == 0x48) {  // UP arrow
                if (selected_index > 0) {
                    selected_index--;
                    if (selected_index < scroll_offset) {
                        scroll_offset = selected_index;
                    }
                }
                return true;
            }
            else if (e.data.keyboard.key == 0x50) {  // DOWN arrow
                if (selected_index < item_count - 1) {
                    selected_index++;
                    int item_height = 10;
                    int max_visible = (h - 4) / item_height;
                    if (selected_index >= scroll_offset + max_visible) {
                        scroll_offset = selected_index - max_visible + 1;
                    }
                }
                return true;
            }
            break;
            
        default:
            break;
    }
    
    return false;
}

// =============== Window Implementation ===============

void Window::draw()
{
    if (!visible) return;
    
    uint32_t title_bg = focused ? Graphics::Colors::ACCENT : Graphics::Colors::BUTTON_BG;
    uint32_t title_fg = Graphics::Colors::BUTTON_FG;
    uint32_t body_bg = Graphics::Colors::BACKGROUND;
    
    // Draw window background
    Graphics::g_framebuffer.fill_rectangle(x, y, w, h, body_bg);
    
    // Draw title bar
    Graphics::g_framebuffer.fill_rectangle(x, y, w, 20, title_bg);
    Graphics::g_framebuffer.draw_border(x, y, w, 20, title_bg, 1);
    
    // Draw title text
    Graphics::g_framebuffer.draw_string(x + 4, y + 6, title, title_fg);
    
    // Draw window border
    Graphics::g_framebuffer.draw_rectangle(x, y, w, h, Graphics::Colors::BORDER);
    
    // Draw children
    for (int i = 0; i < child_count; i++) {
        if (children[i]->is_visible()) {
            children[i]->draw();
        }
    }
}

bool Window::on_event(const Event& e)
{
    if (!visible) return false;
    
    switch (e.type) {
        case EventType::MOUSE_CLICK: {
            // Title bar dragging
            if (draggable && e.data.mouse.y >= y && e.data.mouse.y < y + 20) {
                if (e.data.mouse.x >= x && e.data.mouse.x < x + w) {
                    drag_active = true;
                    drag_offset_x = e.data.mouse.x - x;
                    drag_offset_y = e.data.mouse.y - y;
                    focused = true;
                    return true;
                }
            }
            
            // Check children
            if (e.data.mouse.x >= x && e.data.mouse.x < x + w &&
                e.data.mouse.y >= y && e.data.mouse.y < y + h) {
                for (int i = child_count - 1; i >= 0; i--) {
                    if (children[i]->on_event(e)) {
                        return true;
                    }
                }
                return true;  // Click in window area
            }
            break;
        }
        
        case EventType::MOUSE_MOVE:
            if (drag_active) {
                x = e.data.mouse.x - drag_offset_x;
                y = e.data.mouse.y - drag_offset_y;
                return true;
            }
            break;
            
        case EventType::MOUSE_RELEASE:
            if (drag_active) {
                drag_active = false;
                return true;
            }
            break;
            
        case EventType::KEY_PRESS:
            // Forward to children
            for (int i = child_count - 1; i >= 0; i--) {
                if (children[i]->on_event(e)) {
                    return true;
                }
            }
            break;
            
        default:
            break;
    }
    
    return false;
}

}; // namespace GUI
