# MoonOS 🌙 — The Home System Ecosystem

**MoonOS** is a from-scratch operating system ecosystem designed for **home-centric devices** — from headless servers and NAS boxes to embedded panels and mobile terminals. It is built around a microkernel-inspired architecture with a strong separation between kernel, modules, platform services, and user-facing variants.

> 🚧 *All components are in active early development. APIs and structures will evolve.*

---

## 🌍 Ecosystem Overview

MoonOS is divided into several repositories, each with a distinct role:

| Repository | Description |
|-----------|-------------|
| **[moonos](https://github.com/Peter-201101/moonos)** | Core kernel: memory management, scheduler, syscalls, HAL, panic handler. |
| **[moonos-modules](https://github.com/Peter-201101/moonos-modules)** | Hardware drivers: VGA/display, PS/2 keyboard, serial port, ATA storage, network stub. |
| **[moonos-platform](https://github.com/Peter-201101/moonos-platform)** | Platform abstraction layer: device manager, driver manager, virtual filesystem (VFS), network service. |
| **[moonos-server](https://github.com/Peter-201101/moonos-server)** | Headless server OS variant: service manager, interactive shell, basic userland commands. |
| **[moonos-phone](https://github.com/Peter-201101/moonos-phone)** | Mobile OS variant: UI compositor, window management, touch input handler. |

---

## 🧱 Architecture & Boot Flow

```

┌───────────┐
│   Boot    │  multiboot (x86) / UEFI stub (ARM)
└─────┬─────┘
      │
      ▼
┌───────────┐
│  kmain    │  Kernel entry: PMM, VMM, paging, heap
└─────┬─────┘
      │
      ▼
┌───────────┐
│  HAL init │  Architecture-specific setup (x86 / ARM)
└─────┬─────┘
      │
      ▼
┌───────────┐
│  Modules  │  Load drivers: serial, VGA, keyboard, ATA
└─────┬─────┘
      │
      ▼
┌───────────┐
│ Platform  │  Device manager → Driver manager → VFS → Network
└─────┬─────┘
      │
      ▼
┌───────────┐
│  Variant  │  Server (shell + services) or Phone (UI + touch)
└───────────┘
```

---

## ⚙️ Core Kernel (`moonos`)

| Subsystem | Description |
|-----------|-------------|
| **PMM** | Physical memory manager with page frame tracking. |
| **VMM** | Virtual memory manager with paging enabled. |
| **Heap** | Free-list kernel heap with corruption detection (`0xDEADBEEF`/`0xCAFEBABE`). |
| **Scheduler** | Priority-based preemptive scheduler with round-robin within priority levels. |
| **Syscalls** | `int 0x80` dispatcher (x86) with initial system call table. |
| **HAL** | Separate implementations for **x86 (32-bit)** and **ARM (aarch64/armv8-a)**. |
| **Panic** | Informative kernel panic with architecture-specific CPU halt. |

---

## 🔌 Modules (`moonos-modules`)

| Module | File(s) | Status |
|--------|---------|--------|
| **VGA Display** | `display/vga.c` | Text-mode VGA with scroll, cursor, color support |
| **Framebuffer** | `display/framebuffer.c` | Placeholder for graphical framebuffer |
| **PS/2 Keyboard** | `keyboard/ps2.c` | US QWERTY scancode → ASCII, shift/ctrl/alt modifiers, circular buffer |
| **Serial Port** | `serial/serial.c` | COM1–4 initialization, baud rate configuration, blocking & non-blocking I/O |
| **ATA Storage** | `storage/ata.c` | PIO-mode ATA driver with IDENTIFY, LBA28 read/write, multi-drive support |
| **Network** | `network/net.c` | Stub (placeholder for future TCP/IP stack) |

---

## 🖥️ Platform Layer (`moonos-platform`)

| Component | File(s) | Description |
|-----------|---------|-------------|
| **Device Manager** | `device/device.c` | Register/unregister devices by ID and name, dump device list |
| **Driver Manager** | `driver/driver_manager.c` | Bind modules to devices, manage driver lifecycle |
| **Virtual Filesystem** | `fs/fs_service.c` | In-memory VFS with directories, files, path resolution, `ls`/`mkdir`/`touch` support |
| **Network Service** | `net/net_service.c` | Placeholder for network stack integration |
| **Platform Init** | `platform.c` | Ties all platform components together, registers built-in devices |

---

## 🖧 Server Variant (`moonos-server`)

| Component | Description |
|-----------|-------------|
| **Service Manager** | `services/service_manager.c` — manages background services |
| **Shell** | `shell/shell.c` — interactive command-line with history, cursor movement, escape sequences |
| **Commands** | `shell/commands.c` — `help`, `clear`, `ls`, `mkdir`, `touch`, `pwd`, `devices`, `drivers`, `tasks`, `meminfo`, `uname`, `reboot`, `halt` |
| **Init** | `init/init.c` — server startup: creates base filesystem, launches shell |

---

## 📱 Phone Variant (`moonos-phone`)

| Component | Description |
|-----------|-------------|
| **Init** | `init/init.c` — phone-specific initialization |
| **Compositor** | `ui/compositor.c` — display compositor |
| **Window Manager** | `ui/window.c` — window management |
| **Input Handler** | `ui/input.c` — touch/input event processing |

---

## 🏗️ Building

### Prerequisites
- `gcc` cross-compiler for **i686-elf** (x86) and **aarch64-elf** (ARM)
- `ld` (GNU linker)
- `make`
- `qemu` (optional, for testing)

### Build
```bash
curl -sSL https://moonx.com/os/build.sh

# Build for x86 (32-bit)
make x86

# Build for ARM (aarch64)
make arm
```

Run in QEMU

```bash
make run-x86   # boots MoonOS on x86
make run-arm   # boots MoonOS on ARM (virt machine)
```

Each sub-repository (moonos-modules, moonos-platform, moonos-server, moonos-phone) has its own Makefile and build instructions.

---

🗺️ Roadmap

Short-term

· Userspace & Processes — move from kernel threads to user mode
· Filesystem Persistence — disk-backed VFS (ext2 or simple FS)
· Networking Stack — TCP/IP for home server use cases
· Module Loader — dynamic module loading/unloading at runtime

Medium-term

· x86_64 Support — 64-bit kernel and userland
· RISC-V Architecture — extend HAL for RISC-V
· Package System — modular package/plugin system for home services

Long-term Vision

· Home automation controller
