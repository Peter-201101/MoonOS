# MoonOS CLI Quick Start Guide

## 🚀 Boot & Launch

```bash
cd /workspaces/MoonOS
make              # Build the ISO
make run          # Launch in QEMU
```

**Expected boot sequence:**
```
[KERNEL] MoonOS x86_64 Bootloader...
[GDT] Global Descriptor Table loaded
[IDT] Interrupt Descriptor Table loaded
[PMM] Physical Memory Manager initialized
[VMM] Virtual Memory Manager initialized
[INIT] Initializing hardware...
[ATA] Disk controller initialized
[FS] Filesystem ready
[CONFIG] Configuration loaded
[KEYBOARD] PS/2 Keyboard driver initialized

╔════════════════════════════════════════╗
║     Welcome to MoonOS CLI v0.1.0       ║
║     No GUI - Pure command line OS      ║
╚════════════════════════════════════════╝
Type 'help' for available commands.

moonos> _
```

---

## ⌨️ Keyboard Controls

### Basic Input
| Key | Function |
|-----|----------|
| A-Z | Lowercase letters |
| Shift+A-Z | **UPPERCASE** letters |
| 0-9 | Numbers (same with Shift: ! @ # $ % ^ & * () |
| Space | Space character |
| Enter | Execute command |
| Backspace | Delete before cursor |

### Navigation (In Command Line)
| Key | Function |
|-----|----------|
| ← | Move cursor left |
| → | Move cursor right |
| Home | Go to line start |
| End | Go to line end |
| Delete | Remove character at cursor |

### History
| Key | Function |
|-----|----------|
| ↑ | Previous command |
| ↓ | Next command |

---

## 📝 Available Commands

### `help`
Show all available commands with descriptions

**Usage:**
```
moonos> help
=== MoonOS CLI Help ===
Built-in commands:
  help      - Show available commands
  echo      - Print text
  clear     - Clear screen
  version   - Show kernel version
  edit      - Edit a text file
  poweroff  - Shutdown system
```

---

### `echo` - Print text
Print text to console

**Usage:**
```
moonos> echo Hello World!
Hello World!

moonos> echo This is MoonOS
This is MoonOS

moonos> echo Multiple  spaces   preserved
Multiple  spaces   preserved
```

---

### `version` - Show version
Display OS information

**Usage:**
```
moonos> version
MoonOS v0.1.0 CLI Edition
No GUI, pure CLI interface
```

---

### `clear` - Clear screen
Clear terminal (ANSI-based)

**Usage:**
```
moonos> clear
[Screen clears, cursor at top]
moonos> 
```

---

### `edit` - Text Editor 🎯 **NEW**
Open a text editor to create/edit files (in memory)

**Usage:**
```
moonos> edit myfile.txt
[EDITOR] Opening: myfile.txt
┌─ MoonOS Editor ──────────────────────┐
│ Ctrl+H for help, Ctrl+S save, Ctrl+Q quit  │
├──────────────────────────────────────┤
│   1 │ 
│   2 │ 
│   3 │ 
│  ...
└──────────────────────────────────────┘
Pos: 0/0 | Type text or Ctrl+H for help
> _
```

#### Editor Keyboard Shortcuts:
| Shortcut | Function |
|----------|----------|
| Ctrl+S | Save file |
| Ctrl+Q | Quit editor |
| Ctrl+H | Show help |
| ← / → | Move cursor left/right |
| ↑ / ↓ | Move to previous/next line |
| Home | Go to line start |
| End | Go to line end |
| Backspace | Delete before cursor |
| Delete | Delete at cursor |
| Enter | New line |
| Any ASCII | Insert character |

---

### `poweroff` - Shutdown
Shutdown the system

**Usage:**
```
moonos> poweroff
Powering off...
[System halts]
```

---

## 🎮 Interactive Example Session

### Example 1: Basic Commands
```
moonos> help
=== MoonOS CLI Help ===
Built-in commands:
[... help displayed ...]

moonos> version
MoonOS v0.1.0 CLI Edition
No GUI, pure CLI interface

moonos> echo Welcome to MoonOS!
Welcome to MoonOS!

moonos> 
```

### Example 2: Using History
```
moonos> echo First command
First command

moonos> echo Second command
Second command

moonos> echo Third command
Third command

moonos> [PRESS UP ARROW]
moonos> echo Third command

moonos> [PRESS UP ARROW AGAIN]
moonos> echo Second command

moonos> [PRESS DOWN ARROW]
moonos> echo Third command

moonos> [EDIT - remove "echo" with backspace]
moonos> Third command

moonos> [PRESS BACKSPACE x 6]
moonos> e command

moonos> [TYPE new text]
moonos> echo Different command

moonos> [PRESS ENTER]
Different command
```

### Example 3: Using Editor
```
moonos> edit README.txt
[EDITOR] Opening: README.txt

# [Type content]:
MoonOS - A Simple Operating System

This is a simple OS with:
- CLI interface
- Text editor
- No GUI

[EDITOR SHOWS]:
┌─ MoonOS Editor ──────────────────────┐
│ Ctrl+H for help, Ctrl+S save, Ctrl+Q quit  │
├──────────────────────────────────────┤
│   1 │ MoonOS - A Simple Operating System
│   2 │ 
│   3 │ This is a simple OS with:
│   4 │ - CLI interface
│   5 │ - Text editor
│   6 │ - No GUI
│   7 │ 
└──────────────────────────────────────┘
Pos: 127/127 | Type text or Ctrl+H for help
> 

moonos> [PRESS Ctrl+S]
[EDITOR] File saved: 127 bytes

moonos> [PRESS Ctrl+Q]
[Back to shell]
moonos> 
```

---

## 💡 Tips & Tricks

### Command History
- You can edit recalled commands before re-executing
- History persists for up to **32 commands** per session
- Mixing with ↑/↓ is smooth for quick command variations

### Editor Usage
- Great for creating quick notes
- Edit by inserting/deleting characters at cursor
- Use arrows to navigate the text freely
- Remember: Save with **Ctrl+S** before Ctrl+Q!
- Not yet saved to disk (future feature)

### Keyboard Typing
- Shift works for uppercase and special chars
- Backspace and Delete work intuitively
- No mouse needed - everything keyboard-driven
- Home/End for quick line navigation

---

## ⚠️ Limitations

1. **Text Editor**: Files are in-memory only (no disk persistence yet)
2. **History**: Max 32 commands stored
3. **Buffer sizes**: Fixed buffers (8KB editor, 256B command line)
4. **Characters**: ASCII printable only (32-127)
5. **No autocomplete**: Not yet implemented
6. **No pipes**: Command output not redirectable

---

## 🔍 Troubleshooting

### Keyboard not responding?
- Make sure shell started properly (check boot messages)
- IRQ1 handler should be registered after IDT init
- Try typing slowly if there's lag

### Editor text not showing?
- Text is shown line-by-line with numbers
- Make sure you've entered text and it's visible
- Use arrow keys to navigate

### History not working?
- Press UP/DOWN arrows to navigate
- First command in session won't have previous history
- History resets on each boot

### Clear screen not working?
- Uses ANSI escape codes (should work in modern terminals)
- If not, just type `poweroff` and reboot

---

## 📊 System Information

| Component | Status |
|-----------|--------|
| Boot | ✅ Multiboot2 GRUB |
| CPU | ✅ Long mode x86_64 |
| GDT/IDT | ✅ Initialized |
| Memory | ✅ PMM + VMM ready |
| Keyboard | ✅ PS/2 + Interrupt IRQ1 |
| Serial Console | ✅ COM1 output |
| Shell | ✅ Full CLI |
| Text Editor | ✅ In-memory |
| File System | ⏳ Pending |
| Processes | ⏳ Pending |

---

## 🎯 Next Features Coming

- [ ] File system (ls, mkdir, cat commands)
- [ ] Disk I/O (save editor files)
- [ ] Process management
- [ ] System calls

---

## 💾 Build & Test

```bash
# Full build
cd /workspaces/MoonOS && make

# Run in QEMU
make run

# Or manually:
qemu-system-x86_64 \
  -cdrom build/MoonOS.iso \
  -serial stdio \
  -m 512 \
  -d guest_errors

# In QEMU: Type commands → Enjoy! 🎉
```

---

**Enjoy MoonOS CLI!** 🚀

Type `help` in the shell to get started!
