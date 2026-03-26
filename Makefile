# ==========================================
# MoonOS - Makefile (FINAL ATA READY)
# ==========================================

# ── Toolchain ──
CXX     := g++
ASM     := nasm
LD      := ld
GRUB    := grub-mkrescue

# ── Flags ──
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
CXX_SRCS := $(shell find kernel -name "*.cpp")
ASM_SRCS := $(shell find . -name "*.asm")
CXX_OBJS := $(CXX_SRCS:.cpp=.o)
ASM_OBJS := $(ASM_SRCS:.asm=.o)
ALL_OBJS := $(CXX_OBJS) $(ASM_OBJS)

# ── Output ──
KERNEL  := iso/boot/kernel.bin
ISO     := build/MoonOS.iso
DISK    := build/disk.img

# ==========================================

.PHONY: all clean run

all: $(ISO)

# =========================
# Compile C++
# =========================
%.o: %.cpp
	@echo "[CXX] $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# =========================
# Assemble ASM
# =========================
%.o: %.asm
	@echo "[ASM] $<"
	@$(ASM) $(ASMFLAGS) $< -o $@

# =========================
# Link Kernel
# =========================
$(KERNEL): $(ALL_OBJS)
	@mkdir -p iso/boot/grub
	@mkdir -p build
	@echo "[LD]  Linking kernel..."
	@$(LD) $(LDFLAGS) $(ALL_OBJS) -o build/kernel.bin
	@cp build/kernel.bin $(KERNEL)
	@cp boot/grub/grub.cfg iso/boot/grub/grub.cfg
	@echo "[OK]  Kernel & Config ready"

# =========================
# Build ISO
# =========================
$(ISO): $(KERNEL)
	@echo "[ISO] Building MoonOS.iso..."
	@$(GRUB) -o $(ISO) iso/
	@echo "[OK]  ISO ready: $(ISO)"

# =========================
# Create Disk (PERSIST)
# =========================
$(DISK):
	@mkdir -p build
	@echo "[DISK] Creating disk image (16MB)..."
	@dd if=/dev/zero of=$(DISK) bs=1M count=16 status=none
	@echo "[OK]  Disk ready: $(DISK)"

# =========================
# Run QEMU (WITH DISK)
# =========================
run: $(ISO) $(DISK)
	@echo "[QEMU] Booting MoonOS... (Type shutdown to exit)"
	qemu-system-x86_64 \
	    -cdrom $(ISO) \
	    -drive file=$(DISK),format=raw,if=ide \
	    -m 128M \
	    -display none \
	    -serial stdio \
	    -no-reboot

# =========================
# Clean
# =========================
clean:
	@echo "[CLN] Cleaning..."
	@rm -rf build
	@rm -f $(ALL_OBJS)
	@rm -f iso/boot/kernel.bin
	@echo "[OK]  Clean done"