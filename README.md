# MoonOS - Custom Operating System

A custom-built 64-bit operating system kernel written in C++ and Assembly. MoonOS is designed from scratch with core kernel features, device drivers, and system utilities.

## 📋 Project Overview

MoonOS is an educational operating system project that demonstrates:
- **x86-64 Architecture Support** - 64-bit kernel with proper memory segmentation
- **Bootloader** - GRUB-based bootloader with Multiboot2 support
- **Kernel Core** - GDT (Global Descriptor Table) and IDT (Interrupt Descriptor Table) implementation
- **Device Drivers** - Keyboard, Serial port, and ATA disk drivers
- **Memory Management** - Physical Memory Manager (PMM) and Virtual Memory Manager (VMM)
- **File System** - Basic file system implementation
- **System Components** - Login system, configuration, and setup utilities

## 🛠️ Building MoonOS

### Prerequisites
- **GCC** with cross-compiler support for x86-64
- **NASM** - Netwide Assembler for assembly files
- **GNU Make** - Build tool
- **GNU ld** - Linker
- **GRUB tools** - grub-mkrescue for ISO creation
- **Linux** development environment

### Build Steps

```bash
# Clone the repository
git clone https://github.com/Peter-201101/MoonOS.git
cd MoonOS

# Build the kernel and ISO image
make all

# Clean build artifacts (excluding ISO)
make clean

# Run with QEMU
make run
```

The build process will:
1. Compile C++ kernel code with `-ffreestanding` flags
2. Assemble x86-64 assembly files
3. Link all object files using the kernel linker script
4. Generate a bootable ISO image with GRUB

## 🚀 Features

### Core Architecture
- **64-bit Kernel** - Full x86-64 support
- **GDT Implementation** - Proper segment descriptors for kernel and user space
- **IDT Implementation** - Interrupt handling setup
- **ISR Handlers** - Assembly-based interrupt service routines

### Device Support
- **Keyboard Driver** - PS/2 keyboard input handling
- **Serial Driver** - Serial port communication for debugging
- **ATA Driver** - SATA/IDE disk controller support

### Memory Management
- **PMM (Physical Memory Manager)** - Manages physical memory allocation
- **VMM (Virtual Memory Manager)** - Virtual memory and paging support

### File System
- **Basic FS** - File system operations and abstractions

### System Features
- **Login System** - User authentication
- **Configuration** - System configuration management
- **Setup** - System initialization and setup

## 📝 Build Output Files

After successful build:
- `build/MoonOS.iso` - Bootable ISO image
- `build/disk.img` - Disk image (if applicable)
- `build/kernel.bin` - Raw kernel binary
- Object files (`.o`) throughout the source tree

## 🔧 Key Components

### Multiboot2 Compliance
The kernel follows the Multiboot2 specification for compatibility with GRUB and other multiboot-compliant bootloaders.

### Toolchain
- **C++ Version**: C++17 with `-ffreestanding` mode
- **Optimization**: `-O2` for performance
- **Assembly Format**: ELF64 for x86-64

### Compilation Flags
```makefile
-std=c++17           # C++17 standard
-ffreestanding       # Freestanding environment (no libc)
-fno-exceptions      # Disable C++ exceptions
-fno-rtti            # Disable RTTI
-fno-stack-protector # Disable stack protector
-mno-red-zone        # Disable red zone (x86-64)
-m64                 # 64-bit platform
```

## 📦 Dependencies

C++ Standard Library: Not used (`-ffreestanding`)
External Libraries: None (bare metal)

## 🤝 Recent Changes

- Fixed login and keyboard driver implementations
- Added ATA disk driver support
- Implemented file system components
- Added configuration and setup utilities
- Organized system utilities

## 📄 License

This project is created by Peter-201101. Check LICENSE file for details.

## 🎯 Future Improvements

- [ ] Complete interrupt handling
- [ ] Implement paging and virtual memory
- [ ] Enhanced file system (journaling, permissions)
- [ ] Network stack
- [ ] Shell/Command interpreter
- [ ] Process/Thread management
- [ ] Memory protection

---

**Author**: Peter-201101  
**Last Updated**: March 2026
