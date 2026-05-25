BUILD ?= build

.PHONY: all x86 arm clean clean-x86 clean-arm

all:
	@echo "ERROR: Tentukan target secara eksplisit."
	@echo "  make x86   → build untuk x86"
	@echo "  make arm   → build untuk ARM"
	@exit 1

#------------------------------------------------------------
# Source lists
#------------------------------------------------------------
X86_C_SRCS = core/kmain.c core/panic.c core/memory.c core/task.c     \
             core/scheduler.c core/syscall.c core/syscall_table.c      \
             mm/pmm.c mm/vmm.c                                          \
             hal/x86/boot/multiboot.c hal/x86/serial/serial_debug.c   \
             hal/x86/interrupt/idt.c hal/x86/interrupt/irq.c           \
             hal/x86/interrupt/pic.c hal/x86/timer/pit.c               \
             core/klog.c												\
             lib/string.c lib/stdlib.c								\
			 hal/x86/interrupt/isr.c								

X86_ASM_SRCS = boot/multiboot.asm boot/entry.asm                       \
               core/switch_context.asm hal/x86/interrupt/isr.asm

ARM_C_SRCS = core/kmain.c core/panic.c core/memory.c core/task.c      \
             core/scheduler.c core/syscall.c core/syscall_table.c      \
             mm/pmm.c mm/vmm.c                                          \
             hal/arm/serial/uart.c                                      \
             hal/arm/interrupt/gic.c hal/arm/interrupt/irq.c           \
             hal/arm/timer/timer.c                                      \
             lib/string.c lib/stdlib.c

ARM_S_SRCS = hal/arm/boot/entry.S core/switch_context_arm.S

#------------------------------------------------------------
# Object lists
#------------------------------------------------------------
X86_C_OBJS   = $(patsubst %.c,   $(BUILD)/x86/%.o, $(X86_C_SRCS))
X86_ASM_OBJS = $(patsubst %.asm, $(BUILD)/x86/%.o, $(X86_ASM_SRCS))
ARM_C_OBJS   = $(patsubst %.c,   $(BUILD)/arm/%.o, $(ARM_C_SRCS))
ARM_S_OBJS   = $(patsubst %.S,   $(BUILD)/arm/%.o, $(ARM_S_SRCS))

#------------------------------------------------------------
# Targets
#------------------------------------------------------------
x86: $(X86_ASM_OBJS) $(X86_C_OBJS)
	@echo "[OK] x86 kernel objects built"

arm: $(ARM_S_OBJS) $(ARM_C_OBJS)
	@echo "[OK] arm kernel objects built"

#------------------------------------------------------------
# x86 compile rules
#------------------------------------------------------------
$(BUILD)/x86/%.o: %.c
	@mkdir -p $(dir $@)
	gcc -m32 -std=c99 -ffreestanding -fno-stack-protector      \
	    -fno-pie -fno-pic -fno-builtin -O2 -Wall -Wextra       \
	    -nostdinc -I./include -I./lib -I.                       \
	    -I../moonos-platform                                     \
	    -I../moonos-modules/include -I../moonos-modules         \
	    -DARCH_X86 -c $< -o $@
	@echo "[CC x86] $<"

$(BUILD)/x86/%.o: %.asm
	@mkdir -p $(dir $@)
	nasm -f elf32 $< -o $@
	@echo "[AS x86] $<"

#------------------------------------------------------------
# ARM compile rules
#------------------------------------------------------------
$(BUILD)/arm/%.o: %.c
	@mkdir -p $(dir $@)
	aarch64-linux-gnu-gcc -march=armv8-a -std=c99 -ffreestanding \
	    -fno-stack-protector -fno-pie -fno-pic -fno-builtin      \
	    -O2 -Wall -Wextra -nostdinc                              \
	    -I./include -I./lib -I.                                  \
	    -I../moonos-platform                                      \
	    -I../moonos-modules/include -I../moonos-modules          \
	    -DARCH_ARM -c $< -o $@
	@echo "[CC arm] $<"

$(BUILD)/arm/%.o: %.S
	@mkdir -p $(dir $@)
	aarch64-linux-gnu-as $< -o $@
	@echo "[AS arm] $<"

#------------------------------------------------------------
# Clean
#------------------------------------------------------------
clean:
	rm -rf $(BUILD)
	@echo "[OK] Cleaned"

clean-x86:
	rm -rf $(BUILD)/x86
	@echo "[OK] Cleaned x86"

clean-arm:
	rm -rf $(BUILD)/arm
	@echo "[OK] Cleaned arm"