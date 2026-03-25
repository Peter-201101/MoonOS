# ==========================================
# MoonOS - Makefile (Revisi)
# ==========================================

# ── Toolchain ──
CXX     := g++
ASM     := nasm
LD      := ld
GRUB    := grub-mkrescue

# ── Flags ──
# Menambahkan -Ikernel agar bisa #include <drivers/serial.hpp>
CXXFLAGS := \
    -std=c++17           \
    -ffreestanding       \
    -fno-exceptions      \
    -fno-rtti            \
    -fno-stack-protector \
    -mno-red-zone        \
    -m64                 \
    -O2                  \
    -Wall                \
    -Wextra              \
    -Iinclude            \
    -Ikernel

ASMFLAGS := -f elf64

LDFLAGS  := \
    -T linker/linker.ld  \
    -nostdlib            \
    -z noexecstack

# ── Source & Object Files ──
# Menggunakan wildcard agar tidak perlu nambah satu-satu secara manual
CXX_SRCS := $(shell find kernel -name "*.cpp")
ASM_SRCS := $(shell find . -name "*.asm")
CXX_OBJS := $(CXX_SRCS:.cpp=.o)
ASM_OBJS := $(ASM_SRCS:.asm=.o)
ALL_OBJS := $(CXX_OBJS) $(ASM_OBJS)

# ── Output ──
KERNEL  := iso/boot/kernel.bin
ISO     := MoonOS.iso

# ==========================================

.PHONY: all clean run iso

all: $(ISO)

# Compile C++
%.o: %.cpp
	@echo "[CXX] $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# Assemble .asm
%.o: %.asm
	@echo "[ASM] $<"
	@$(ASM) $(ASMFLAGS) $< -o $@

# Link kernel & Prepare ISO structure
$(KERNEL): $(ALL_OBJS)
	@mkdir -p iso/boot/grub
	@echo "[LD]  Linking kernel..."
	@$(LD) $(LDFLAGS) $(ALL_OBJS) -o $(KERNEL)
	@cp boot/grub/grub.cfg iso/boot/grub/grub.cfg
	@echo "[OK]  Kernel & Config ready"

# Build ISO
$(ISO): $(KERNEL)
	@echo "[ISO] Building MoonOS.iso..."
	@$(GRUB) -o $(ISO) iso/
	@echo "[OK]  ISO ready: $(ISO)"

# Jalankan di QEMU (Fix Error: Multiple Character Devices)
# Menggunakan -display none agar tidak bentrok dengan -serial stdio
run: $(ISO)
	@echo "[QEMU] Booting MoonOS... (Press Ctrl+A then X to exit)"
	qemu-system-x86_64 \
	    -cdrom $(ISO)   \
	    -m 128M         \
	    -display none   \
	    -serial stdio   \
	    -no-reboot

clean:
	@echo "[CLN] Cleaning..."
	@rm -f $(ALL_OBJS) $(KERNEL) $(ISO)
	@echo "[OK]  Clean done"