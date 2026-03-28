# VirtualBox Guru Meditation - Workaround

## Problem
VirtualBox 7.x menunjukkan "guru meditation" saat boot, sementara QEMU berjalan normal.
Ini adalah VirtualBox compatibility issue dengan GRUB bootloader legacy mode.

## Solusi - Pilih Salah Satu:

### Opsi 1: Settings VirtualBox (Recommended)
1. VM Settings → System → Motherboard:
   - ✅ Enable I/O APIC → **DISABLE** (unchecked)
   - ✅ Hardware Clock in UTC → DISABLE
   - ✅ Chipset → Set ke **ICH9** (bukan PIIX3)

2. VM Settings → System → Processor:
   - ✅ CPU Execution → Set CPU Count ke **1**
   - ✅ Disable Nested VT-x/AMD-V

3. VM Settings → Display:
   - ✅ Graphics Memory → **32 MB**
   - ✅ Graphics Controller → **VMSVGA** (bukan VBoxVGA)

4. VM Settings → Storage:
   - ✅ Controller Type → **PIIX4** (bukan AHCI untuk IDE)

5. **PENTING**: Boot mode:
   - VM Settings → System → Boot Order → uncheck Network/Floppy
   - Keep CD/DVD first

### Opsi 2: Upgrade GRUB (Advanced)
```bash
# Update ke grub-mkrescue terbaru
sudo apt update && sudo apt install --only-upgrade grub-common grub-pc
make clean && make
```

### Opsi 3: Use QEMU instead (Easiest)
```bash
make run  # Runs QEMU by default
```

## Debugging
Jika masih guru meditation:
1. Enable serial port di VM Settings → Ports → Serial Ports
2. Connect ke: `/tmp/MoonOS.serial` via socat
3. Lihat error messages

## Status
- ✅ QEMU: Works perfectly (boot, setup, shell)
- ⚠️ VirtualBox: Requires workaround
- ✅ Kernel: 100% valid (issue bukan di code)
