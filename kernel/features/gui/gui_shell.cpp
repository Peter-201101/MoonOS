#include "gui_shell.hpp"
#include <io/serial.hpp>

namespace GUI {

// =============== GUI Shell Implementation ===============

Shell::Shell()
    : file_manager_win(nullptr), command_win(nullptr), terminal_win(nullptr),
      directory_listing(nullptr), current_path_label(nullptr),
      parent_btn(nullptr), new_folder_btn(nullptr), delete_btn(nullptr),
      command_input(nullptr), terminal_input(nullptr), terminal_output(nullptr),
      running(true), mouse_x(0), mouse_y(0), mouse_pressed(false)
{
    current_directory[0] = '/';
    current_directory[1] = '\0';
}

Shell::~Shell()
{
    // Window manager will clean up windows
}

void Shell::init()
{
    Serial::writeln("[GUI] Initializing GUI Shell...");
    
    // Clear screen
    Graphics::g_framebuffer.clear(Graphics::Colors::BACKGROUND);
    
    // Create windows
    create_windows();
    
    Serial::writeln("[GUI] GUI Shell ready!");
}

void Shell::create_windows()
{
    // File Manager Window (left side, 330x700)
    file_manager_win = manager.create_window(10, 30, 330, 700, "File Manager");
    if (file_manager_win) {
        setup_file_manager();
    }
    
    // Command Palette Window (middle, 340x700)
    command_win = manager.create_window(352, 30, 340, 700, "Command Palette");
    if (command_win) {
        setup_command_palette();
    }
    
    // Terminal Window (right side, 320x700)
    terminal_win = manager.create_window(704, 30, 310, 700, "Terminal");
    if (terminal_win) {
        setup_terminal();
    }
}

void Shell::setup_file_manager()
{
    if (!file_manager_win) return;
    
    // Path label
    current_path_label = new Label(15, 35, "Path: /", Graphics::Colors::TEXT, file_manager_win);
    file_manager_win->add_widget(current_path_label);
    
    // Directory listing
    directory_listing = new ListBox(15, 55, 300, 500, file_manager_win);
    file_manager_win->add_widget(directory_listing);
    update_file_listing();
    
    // Buttons
    parent_btn = new Button(15, 560, 80, 25, "Parent", file_manager_win);
    file_manager_win->add_widget(parent_btn);
    
    new_folder_btn = new Button(105, 560, 100, 25, "New Folder", file_manager_win);
    file_manager_win->add_widget(new_folder_btn);
    
    delete_btn = new Button(215, 560, 80, 25, "Delete", file_manager_win);
    file_manager_win->add_widget(delete_btn);
}

void Shell::setup_command_palette()
{
    if (!command_win) return;
    
    // Command buttons
    const char* cmd_labels[] = {
        "ls", "mkdir", "cd", "pwd", "touch", 
        "rm", "cat", "edit", "help", "clear",
        "version", "reboot", "poweroff"
    };
    
    for (int i = 0; i < 13; i++) {
        int row = i / 3;
        int col = i % 3;
        int bx = 15 + col * 100;
        int by = 35 + row * 40;
        
        cmd_buttons[i] = new Button(bx, by, 90, 30, cmd_labels[i], command_win);
        command_win->add_widget(cmd_buttons[i]);
    }
    
    // Command input
    command_input = new TextBox(15, 240, 300, 25, command_win);
    command_input->set_placeholder("Enter command...");
    command_win->add_widget(command_input);
    
    // Output label
    output_label = new Label(15, 270, "Ready.", Graphics::Colors::TEXT, command_win);
    command_win->add_widget(output_label);
}

void Shell::setup_terminal()
{
    if (!terminal_win) return;
    
    // Terminal output (read-only)
    terminal_output = new Label(15, 35, "MoonOS Terminal\nType commands above", Graphics::Colors::TEXT, terminal_win);
    terminal_win->add_widget(terminal_output);
    
    // Terminal input
    terminal_input = new TextBox(15, 650, 280, 25, terminal_win);
    terminal_input->set_placeholder("Command...");
    terminal_win->add_widget(terminal_input);
}

void Shell::update_file_listing()
{
    if (!directory_listing) return;
    
    directory_listing->clear();
    
    // For now, just add some placeholder items
    // In a real implementation, this would enumerate FS files
    directory_listing->add_item("file1.txt");
    directory_listing->add_item("file2.txt");
    directory_listing->add_item("folder1");
}

void Shell::handle_events()
{
    // Poll mouse
    Mouse::State mouse_state = Mouse::g_mouse.get_state();
    mouse_x = mouse_state.x;
    mouse_y = mouse_state.y;
    
    // Check for mouse clicks
    if (mouse_state.left_button && !mouse_pressed) {
        mouse_pressed = true;
        
        Event e;
        e.type = EventType::MOUSE_CLICK;
        e.data.mouse.x = mouse_x;
        e.data.mouse.y = mouse_y;
        manager.handle_event(e);
    } else if (!mouse_state.left_button && mouse_pressed) {
        mouse_pressed = false;
        
        Event e;
        e.type = EventType::MOUSE_RELEASE;
        e.data.mouse.x = mouse_x;
        e.data.mouse.y = mouse_y;
        manager.handle_event(e);
    }
    
    // Check for keyboard input
    char c;
    if (Keyboard::try_read(c)) {
        Event e;
        e.type = EventType::KEY_PRESS;
        e.data.keyboard.ascii = c;
        e.data.keyboard.key = 0;
        
        manager.handle_event(e);
        
        // Check for Ctrl+Q to exit
        if (c == 17) {  // Ctrl+Q
            running = false;
        }
    }
}

void Shell::run()
{
    init();
    
    while (running) {
        handle_events();
        manager.render_all();
    }
    
    Serial::writeln("[GUI] Shell exiting...");
}

}; // namespace GUI
