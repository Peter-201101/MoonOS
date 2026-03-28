# MoonOS GUI Architecture - Complete Refactor 🎨

**Objective**: Transform MoonOS from CLI to GUI shell  
**Compatibility**: VirtualBox + QEMU  
**Status**: Planning phase

---

## 📐 Architecture Overview

### Current (CLI Only)
```
Bootloader (GRUB)
    ↓
Kernel (x86_64 bare-metal)
    ├─ Memory (PMM/VMM)
    ├─ Drivers (Serial, Keyboard, ATA)
    ├─ File System
    └─ Shell (Text-based CLI)
         ├─ Commands: ls, mkdir, etc.
         └─ Input: Keyboard only
```

### New (GUI + CLI)
```
Bootloader (GRUB)
    ↓
Kernel (x86_64 bare-metal)
    ├─ Memory (PMM/VMM)
    ├─ Drivers
    │  ├─ Serial
    │  ├─ Keyboard
    │  ├─ Mouse (NEW)
    │  ├─ Graphics/VESA (NEW)
    │  └─ ATA
    ├─ File System
    ├─ GUI Framework (NEW)
    │  ├─ Framebuffer
    │  ├─ Window Manager
    │  ├─ Event System
    │  └─ Widgets (Button, Text, etc.)
    └─ GUI Shell (NEW)
         ├─ Desktop/Taskbar
         ├─ File Manager Window
         ├─ Command Palette
         ├─ Terminal Window
         └─ Input: Keyboard + Mouse
```

---

## 🎯 Phase 1: Graphics & Input

### 1.1 VESA Graphics Driver
**Goal**: 1024×768 @ 24-bit color

**Why VESA?**
- Universal in QEMU + VirtualBox
- Hardware-agnostic
- 1024×768 good balance (performance vs usability)

**Implementation**:
```cpp
// kernel/drivers/graphics/vesa.hpp
namespace Graphics {
    struct Mode {
        uint16_t width;
        uint16_t height;
        uint8_t bpp;        // Bits per pixel
        uint32_t framebuffer; // Linear frame buffer address
    };
    
    void init();            // Detect VESA 2.0
    void set_mode(uint16_t width, uint16_t height, uint8_t bpp);
    void clear(uint32_t color);
    void draw_pixel(int x, int y, uint32_t color);
    void draw_rectangle(int x, int y, int w, int h, uint32_t color);
    void draw_char(int x, int y, char c, uint32_t fg, uint32_t bg);
    void draw_string(int x, int y, const char* s, uint32_t fg, uint32_t bg);
}
```

### 1.2 Mouse Driver
**Goal**: PS/2 mouse input (same as keyboard)

**Features**:
- Relative motion tracking
- Button states (left, middle, right)
- Cursor position

**Implementation**:
```cpp
// kernel/drivers/mouse.hpp
namespace Mouse {
    struct State {
        int16_t x, y;           // Position
        bool left, right, wheel; // Buttons
    };
    
    void init();
    bool try_read(State& state);
    State get_state();
}
```

---

## 🎨 Phase 2: GUI Framework

### 2.1 Event System
```cpp
// kernel/gui/event.hpp
namespace GUI {
    enum class EventType {
        MOUSE_MOVE,
        MOUSE_CLICK,
        MOUSE_RELEASE,
        KEY_PRESS,
        KEY_RELEASE,
        WINDOW_CLOSE,
        WINDOW_FOCUS,
        CUSTOM
    };
    
    struct Event {
        EventType type;
        union {
            struct { int x, y; } mouse;
            struct { int key; } keyboard;
            struct { int window_id; } window;
        } data;
    };
}
```

### 2.2 Widget System
```cpp
// kernel/gui/widgets.hpp
namespace GUI {
    class Widget {
    public:
        virtual void draw() = 0;
        virtual void on_event(const Event& e) = 0;
        void set_bounds(int x, int y, int w, int h);
        virtual ~Widget() = default;
    };
    
    class Button : public Widget {
        // Click handler
        void (*on_click)() = nullptr;
    };
    
    class TextBox : public Widget {
        char text[256];
        void handle_key(char c);
    };
    
    class Window : public Widget {
        List<Widget*> children;
        bool draggable = true;
    };
}
```

### 2.3 Window Manager
```cpp
// kernel/gui/window_manager.hpp
namespace GUI {
    class WindowManager {
    public:
        void add_window(Window* w);
        void remove_window(Window* w);
        void handle_event(const Event& e);
        void render();
        void set_focus(Window* w);
    };
}
```

---

## 💻 Phase 3: GUI Shell

### 3.1 Desktop Layout
```
┌─────────────────────────────────────────┐
│ MoonOS GUI Desktop    [─][□][×]         │ ← Titlebar
├─────────────────────────────────────────┤
│                                         │
│ [File Manager] [Commands] [Terminal]    │ ← Taskbar buttons
│                                         │
│  ┌─ File Manager ─────────────────────┐ │
│  │ / documents projects data.txt       │ │
│  │                                     │ │
│  └─────────────────────────────────────┘ │
│                                         │
│  ┌─ Command Palette ──────────────────┐ │
│  │ > _                                 │ │
│  │ [ls] [mkdir] [cd] [touch] [rm]     │ │
│  │ [pwd] [cat] [edit] [help]          │ │
│  │                                     │ │
│  └─────────────────────────────────────┘ │
│                                         │
└─────────────────────────────────────────┘
```

### 3.2 Components

**File Manager Window**:
- List view of current directory
- Double-click to open/navigate
- Right-click context menu
- Buttons: New Folder, Delete, Rename

**Command Palette**:
- Command buttons (ls, mkdir, cd, etc.)
- Text input for parameters
- Output display

**Terminal Window**:
- Fallback to text shell
- For advanced commands

---

## 📁 Directory Structure

```
kernel/
├─ drivers/
│  ├─ graphics/
│  │  ├─ vesa.hpp
│  │  ├─ vesa.cpp
│  │  └─ framebuffer.hpp
│  ├─ mouse.hpp
│  └─ mouse.cpp
├─ gui/
│  ├─ event.hpp
│  ├─ widgets.hpp
│  ├─ window_manager.hpp
│  ├─ widgets.cpp
│  ├─ window_manager.cpp
│  └─ gui_shell.cpp
├─ core/
│  └─ kernel.cpp (modified for GUI init)
```

---

## 🔧 Implementation Strategy

### Step 1: Graphics Driver
- Detect VESA modes via multiboot
- Set graphics mode (1024×768×24)
- Implement basic drawing (pixel, rect, text)

### Step 2: Mouse Driver  
- Enable PS/2 mouse
- Read mouse packets
- Update cursor position

### Step 3: GUI Widgets
- Simple button rendering
- Text box input
- Window frame (draggable)

### Step 4: Window Manager
- Maintain window list
- Handle events
- Render all windows with z-order

### Step 5: GUI Shell
- File manager window
- Command palette window
- Integrate with filesystem

---

## ⚙️ Boot Sequence (Modified)

```
Bootloader passes VBE info
    ↓
Kernel Init (same as before)
    ↓
Graphics::init()        ← NEW: Detect VESA modes
    ↓
Graphics::set_mode()    ← NEW: 1024×768×24
    ↓
Mouse::init()           ← NEW: PS/2 mouse
    ↓
... other drivers ...
    ↓
GUI::WindowManager manager
    ↓
GUI::Shell shell(&manager)
    ↓
shell.run()            ← NEW: Event loop instead of CLI loop
```

---

## 🎨 Color Palette & Theme

```cpp
// kernel/gui/theme.hpp
namespace GUI {
    namespace Colors {
        const uint32_t BACKGROUND  = 0x1E1E2E; // Dark purple
        const uint32_t FOREGROUND  = 0xCDD6F4; // Light text
        const uint32_t ACCENT      = 0x89B4FA; // Blue
        const uint32_t BUTTON_BG   = 0x313244; // Darker bg
        const uint32_t BUTTON_FG   = 0xCDD6F4; // Text
        const uint32_t HIGHLIGHT   = 0xA6E3A1; // Green (hover)
        const uint32_t DANGER      = 0xF38BA8; // Red (delete)
    }
}
```

---

## 📊 Performance Considerations

| Component | Memory | CPU Impact |
|-----------|--------|-----------|
| 1024×768×24 Framebuffer | 3 MB | Low |
| GUI Framework | ~10 KB | Minimal |
| Window Manager | ~5 KB | Minimal |
| Mouse Driver | ~2 KB | Low |
| Graphics Driver | ~5 KB | Moderate (drawing) |
| **Total Overhead** | **~20 MB** | **Acceptable** |

---

## 🧪 Testing Plan

### QEMU Testing:
```bash
qemu-system-x86_64 \
  -cdrom build/MoonOS.iso \
  -drive file=build/disk.img,format=raw,if=ide \
  -m 512M \
  -vga std \        # Enable graphics
  -serial stdio
```

### VirtualBox Testing:
- Create new VM (512MB RAM)
- ISO as boot drive
- File Manager: View filesystem visually
- Command Palette: Execute commands via GUI
- Terminal: Full CLI fallback

---

## 🎯 Success Criteria

- ✅ Boot to GUI desktop (not text prompt)
- ✅ Mouse cursor visible and responsive
- ✅ File manager shows directories/files
- ✅ Click files to open/navigate
- ✅ Command buttons execute shell commands
- ✅ Works in both QEMU and VirtualBox
- ✅ Professional appearance (colors, layout)
- ✅ No crashes on window interactions

---

## 🚀 Phase Timeline

| Phase | Task | Estimated |
|-------|------|-----------|
| 1 | VESA Driver | 1-2 hours |
| 2 | Mouse Driver | 30 mins |
| 3 | GUI Framework | 2-3 hours |
| 4 | Window Manager | 1-2 hours |
| 5 | GUI Shell | 2-3 hours |
| 6 | File Manager GUI | 1-2 hours |
| 7 | Testing & Polish | 1-2 hours |
| **Total** | | **9-15 hours** |

---

## 📝 Architecture Benefits

✅ **Professional**: Looks like a real OS  
✅ **Intuitive**: GUI is more user-friendly than CLI  
✅ **Educational**: Learn kernel + graphics + GUI together  
✅ **Portable**: Works in QEMU, VirtualBox, real hardware  
✅ **Extensible**: Easy to add more windows/features  
✅ **Performance**: Lightweight, no bloat  

---

**Ready to start Phase 1?** 🎉
