#include "login.hpp"
#include <drivers/serial.hpp>
#include <system/config/config.hpp>

// Declaration for input_field function
void input_field(const char* prompt, char* buffer, int max_length, bool is_password = false);

static bool str_eq(const char* a, const char* b) {
    int i = 0;
    while (a[i] != '\0' || b[i] != '\0') {
        if (a[i] != b[i]) return false;
        i++;
        if (i >= 32) break;
    }
    return true;
}

void run_login() {
    auto cfg = Config::get();
    char user[32], pass[32];
    
    while (true) {
        Serial::writeln("\n=== MoonOS Login ===");
        // Gunakan input_field yang sudah ada di kode kamu sebelumnya
        input_field("Username : ", user, 32);
        input_field("Password : ", pass, 32, true);

        if (str_eq(user, cfg->username) && str_eq(pass, cfg->password)) {
            Serial::write("\n[Login Success] Welcome, ");
            Serial::writeln(cfg->username);
            return;
        }
        Serial::writeln("\n[Login Failed] Incorrect credentials.");
    }
}