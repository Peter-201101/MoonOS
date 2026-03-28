#ifndef GUI_WIDGETS_HPP
#define GUI_WIDGETS_HPP

#include <gui/graphics/vesa.hpp>
#include <string.hpp>
#include <types.hpp>

namespace GUI {

// =============== Event System ===============

enum class EventType {
    NONE,
    MOUSE_MOVE,
    MOUSE_CLICK,      // Left button
    MOUSE_RELEASE,
    MOUSE_RCLICK,     // Right button
    KEY_PRESS,
    KEY_RELEASE,
    WINDOW_CLOSE,
    WINDOW_FOCUS,
    CUSTOM
};

struct Event {
    EventType type;
    union {
        struct {
            int x;
            int y;
        } mouse;
        struct {
            int key;        // Keyboard key code
            char ascii;
        } keyboard;
        struct {
            int window_id;
        } window;
    } data;
};

// =============== Base Widget ===============

class Event;
class Window;

class Widget {
protected:
    int x, y, w, h;
    bool visible;
    bool focused;
    
public:
    Window* parent;
    
    Widget(int x = 0, int y = 0, int w = 100, int h = 30, Window* parent = nullptr)
        : x(x), y(y), w(w), h(h), visible(true), focused(false), parent(parent) {}
    
    virtual ~Widget() = default;
    
    // Lifecycle
    virtual void draw() = 0;
    virtual bool on_event(const Event& e) { return false; }
    
    // Bounds
    void set_bounds(int nx, int ny, int nw, int nh) {
        x = nx; y = ny; w = nw; h = nh;
    }
    
    bool contains_point(int px, int py) const {
        return px >= x && px < x + w && py >= y && py < y + h;
    }
    
    void get_bounds(int& nx, int& ny, int& nw, int& nh) const {
        nx = x; ny = y; nw = w; nh = h;
    }
    
    // Visibility
    void show() { visible = true; }
    void hide() { visible = false; }
    bool is_visible() const { return visible; }
    
    // Focus
    void set_focus(bool f) { focused = f; }
    bool has_focus() const { return focused; }
};

// =============== Button Widget ===============

class Button : public Widget {
private:
    const char* label;
    void (*on_click)() = nullptr;
    bool pressed = false;
    
public:
    Button(int x, int y, int w, int h, const char* label, Window* parent = nullptr)
        : Widget(x, y, w, h, parent), label(label) {}
    
    void draw() override;
    bool on_event(const Event& e) override;
    
    void set_on_click(void (*callback)()) {
        on_click = callback;
    }
    
    void set_label(const char* new_label) {
        label = new_label;
    }
};

// =============== Text Box Widget ===============

class TextBox : public Widget {
private:
    static const int MAX_TEXT = 256;
    char text[MAX_TEXT];
    int cursor_pos;
    int scroll_offset;
    const char* placeholder;
    
public:
    TextBox(int x, int y, int w, int h, Window* parent = nullptr)
        : Widget(x, y, w, h, parent), cursor_pos(0), scroll_offset(0), placeholder("") 
    {
        text[0] = '\0';
    }
    
    void draw() override;
    bool on_event(const Event& e) override;
    
    void clear() {
        text[0] = '\0';
        cursor_pos = 0;
        scroll_offset = 0;
    }
    
    const char* get_text() const { return text; }
    void set_text(const char* new_text);
    void set_placeholder(const char* ph) { placeholder = ph; }
};

// =============== Label Widget ===============

class Label : public Widget {
private:
    const char* text;
    uint32_t color;
    
public:
    Label(int x, int y, const char* text, uint32_t color = Graphics::Colors::TEXT, Window* parent = nullptr)
        : Widget(x, y, 200, 20, parent), text(text), color(color) {}
    
    void draw() override;
    
    void set_text(const char* new_text) { text = new_text; }
    void set_color(uint32_t c) { color = c; }
};

// =============== List Box Widget ===============

class ListBox : public Widget {
private:
    static const int MAX_ITEMS = 64;
    const char* items[MAX_ITEMS];
    int item_count;
    int selected_index;
    int scroll_offset;
    
public:
    ListBox(int x, int y, int w, int h, Window* parent = nullptr)
        : Widget(x, y, w, h, parent), item_count(0), selected_index(-1), scroll_offset(0)
    {
        for (int i = 0; i < MAX_ITEMS; i++) {
            items[i] = nullptr;
        }
    }
    
    void draw() override;
    bool on_event(const Event& e) override;
    
    void add_item(const char* item) {
        if (item_count < MAX_ITEMS) {
            items[item_count++] = item;
        }
    }
    
    void clear() {
        item_count = 0;
        selected_index = -1;
    }
    
    int get_selected() const { return selected_index; }
};

// =============== Window ===============

class Window : public Widget {
private:
    static const int MAX_CHILDREN = 16;
    Widget* children[MAX_CHILDREN];
    int child_count;
    bool draggable;
    bool drag_active;
    int drag_offset_x, drag_offset_y;
    const char* title;
    int window_id;
    
public:
    Window(int x, int y, int w, int h, const char* title = "Window", int id = 0, 
           Window* parent = nullptr)
        : Widget(x, y, w, h, parent), child_count(0), draggable(true), 
          drag_active(false), title(title), window_id(id) {}
    
    virtual ~Window() {
        for (int i = 0; i < child_count; i++) {
            delete children[i];
        }
    }
    
    void draw() override;
    bool on_event(const Event& e) override;
    
    void add_widget(Widget* widget) {
        if (child_count < MAX_CHILDREN) {
            widget->parent = this;
            children[child_count++] = widget;
        }
    }
    
    void remove_widget(Widget* widget) {
        for (int i = 0; i < child_count; i++) {
            if (children[i] == widget) {
                for (int j = i; j < child_count - 1; j++) {
                    children[j] = children[j + 1];
                }
                child_count--;
                break;
            }
        }
    }
    
    void set_draggable(bool d) { draggable = d; }
    int get_id() const { return window_id; }
    
    // Convert window-relative coords to screen coords
    void to_screen_coords(int& sx, int& sy) const {
        sx = x;
        sy = y;
    }
};

// =============== Window Manager ===============

class WindowManager {
private:
    static const int MAX_WINDOWS = 8;
    Window* windows[MAX_WINDOWS];
    int window_count;
    Window* focused_window;
    int next_window_id;
    
public:
    WindowManager()
        : window_count(0), focused_window(nullptr), next_window_id(1)
    {
        for (int i = 0; i < MAX_WINDOWS; i++) {
            windows[i] = nullptr;
        }
    }
    
    ~WindowManager() {
        for (int i = 0; i < window_count; i++) {
            delete windows[i];
        }
    }
    
    Window* create_window(int x, int y, int w, int h, const char* title = "Window") {
        if (window_count >= MAX_WINDOWS) return nullptr;
        
        Window* win = new Window(x, y, w, h, title, next_window_id++);
        windows[window_count++] = win;
        set_focus(win);
        return win;
    }
    
    void remove_window(Window* w) {
        for (int i = 0; i < window_count; i++) {
            if (windows[i] == w) {
                delete windows[i];
                for (int j = i; j < window_count - 1; j++) {
                    windows[j] = windows[j + 1];
                }
                window_count--;
                if (focused_window == w) {
                    focused_window = (window_count > 0) ? windows[0] : nullptr;
                }
                break;
            }
        }
    }
    
    void set_focus(Window* w) {
        if (focused_window) {
            focused_window->set_focus(false);
        }
        focused_window = w;
        if (w) {
            w->set_focus(true);
            // Bring to front
            if (window_count > 1) {
                for (int i = 0; i < window_count; i++) {
                    if (windows[i] == w) {
                        for (int j = i; j < window_count - 1; j++) {
                            windows[j] = windows[j + 1];
                        }
                        windows[window_count - 1] = w;
                        break;
                    }
                }
            }
        }
    }
    
    bool handle_event(const Event& e) {
        // Handle focused window first (top-most)
        if (focused_window && focused_window->on_event(e)) {
            return true;
        }
        
        // Then other windows in reverse z-order
        for (int i = window_count - 2; i >= 0; i--) {
            if (windows[i]->on_event(e)) {
                return true;
            }
        }
        
        return false;
    }
    
    void render_all() {
        Graphics::g_framebuffer.clear(Graphics::Colors::BACKGROUND);
        
        for (int i = 0; i < window_count; i++) {
            windows[i]->draw();
        }
    }
    
    int get_window_count() const { return window_count; }
    Window* get_window(int idx) {
        if (idx >= 0 && idx < window_count) {
            return windows[idx];
        }
        return nullptr;
    }
};

}; // namespace GUI

#endif // GUI_WIDGETS_HPP
