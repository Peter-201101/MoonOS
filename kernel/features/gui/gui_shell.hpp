#ifndef GUI_SHELL_HPP
#define GUI_SHELL_HPP

#include "widgets.hpp"
#include <gui/graphics/vesa.hpp>
#include <io/keyboard.hpp>
#include <io/mouse.hpp>
#include <storage/fs.hpp>
#include <utils/allocator.hpp>

namespace GUI {

// =============== Application State ===============

class Shell {
private:
    WindowManager manager;
    
    // Main windows
    Window* file_manager_win;
    Window* command_win;
    Window* terminal_win;
    
    // File manager widgets
    ListBox* directory_listing;
    Label* current_path_label;
    Button* parent_btn;
    Button* new_folder_btn;
    Button* delete_btn;
    
    // Command palette widgets
    TextBox* command_input;
    Button* cmd_buttons[13];  // One for each shell command
    Label* output_label;
    
    // Terminal widgets
    TextBox* terminal_input;
    Label* terminal_output;
    
    // State
    char current_directory[256];
    bool running;
    
    // Mouse tracking
    int mouse_x, mouse_y;
    bool mouse_pressed;
    
public:
    Shell();
    ~Shell();
    
    void init();
    void run();
    void handle_events();
    void update_file_listing();
    
private:
    void create_windows();
    void setup_file_manager();
    void setup_command_palette();
    void setup_terminal();
    
    // Event handlers
    void on_parent_clicked();
    void on_new_folder_clicked();
    void on_delete_clicked();
    void on_ls_clicked();
    void on_mkdir_clicked();
    void on_cd_clicked();
    void on_pwd_clicked();
    void on_touch_clicked();
    void on_rm_clicked();
    void on_cat_clicked();
    void on_edit_clicked();
    void on_help_clicked();
    
    // Helpers
    void execute_command(const char* cmd);
    void show_file_properties(const char* filename);
};

}; // namespace GUI

#endif // GUI_SHELL_HPP
