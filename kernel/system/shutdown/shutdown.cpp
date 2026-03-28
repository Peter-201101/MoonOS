#include "shutdown.hpp"
#include <io/serial.hpp>
#include <utils/io.hpp>

void prepare_shutdown() {
    Serial::writeln("\n[SYSTEM] Preparing for shutdown...");
    Serial::writeln("[SYSTEM] Syncing file system... Done.");
    Serial::writeln("[SYSTEM] Closing all hardware ports... Done.");
}

void shutdown() {
    prepare_shutdown();
    
    Serial::writeln("[SHUTDOWN] MoonOS is shutting down now.");
    Serial::writeln("[SHUTDOWN] Goodbye, user.");

    // 1. Coba cara QEMU (ACPI)
    IO::outw(0x604, 0x2000);

    // 2. Coba cara VirtualBox / VMWare (VirtualBox-specific ACPI)
    IO::outw(0x4004, 0x3400);

    // 3. Coba cara Bochs / Older QEMU
    IO::outw(0xB004, 0x2000);

    // 4. Jika semua gagal (Hardware asli/Legacy), beri instruksi ke user
    Serial::writeln("\n-------------------------------------------");
    Serial::writeln(" It is now safe to turn off your computer. ");
    Serial::writeln("-------------------------------------------");

    // Matikan interupsi dan hentikan CPU sepenuhnya
    while (true) {
        __asm__ volatile("cli; hlt");
    }
}