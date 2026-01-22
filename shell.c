#include "shell.h"
#include "graphics.h"
#include "io.h"
#include "vfs.h"
#include "auth.h"

// Configuration
#define MAX_LINES 100
#define MAX_LINE_LEN 256
#define MAX_INPUT_LEN 256
#define MAX_HISTORY 20

// Terminal state
typedef struct {
    char lines[MAX_LINES][MAX_LINE_LEN];
    int line_count;
    int scroll_offset;
    
    char input[MAX_INPUT_LEN];
    int input_len;
    int cursor_pos;
    
    char history[MAX_HISTORY][MAX_INPUT_LEN];
    int history_count;
    int history_index;
    
    bool needs_redraw;
    fs_node_t* cwd;
} terminal_t;

static terminal_t term;

// Nano editor request globals
char nano_requested_file[256];
bool nano_requested = false;

// String helpers
int strcmp(const char* s1, const char* s2) {
    while(*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

int strlen(const char* str) {
    int len = 0;
    while (str[len]) len++;
    return len;
}

void strcpy(char* dest, const char* src) {
    while ((*dest++ = *src++));
}

int strncmp(const char* s1, const char* s2, int n) {
    while(n > 0 && *s1 && (*s1 == *s2)) {
        s1++; s2++; n--;
    }
    if (n == 0) return 0;
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

void strncpy(char* dest, const char* src, int n) {
    int i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    for (; i < n; i++) {
        dest[i] = '\0';
    }
}

// Terminal functions
void terminal_add_line(const char* line) {
    if (term.line_count >= MAX_LINES) {
        // Scroll: shift all lines up
        for (int i = 0; i < MAX_LINES - 1; i++) {
            strcpy(term.lines[i], term.lines[i + 1]);
        }
        term.line_count = MAX_LINES - 1;
    }
    
    strncpy(term.lines[term.line_count], line, MAX_LINE_LEN - 1);
    term.lines[term.line_count][MAX_LINE_LEN - 1] = '\0';
    term.line_count++;
    term.needs_redraw = true;
}

void terminal_add_to_history(const char* cmd) {
    if (strlen(cmd) == 0) return;
    
    // Don't add duplicates of last command
    if (term.history_count > 0 && strcmp(term.history[term.history_count - 1], cmd) == 0) {
        return;
    }
    
    if (term.history_count >= MAX_HISTORY) {
        // Shift history
        for (int i = 0; i < MAX_HISTORY - 1; i++) {
            strcpy(term.history[i], term.history[i + 1]);
        }
        term.history_count = MAX_HISTORY - 1;
    }
    
    strcpy(term.history[term.history_count], cmd);
    term.history_count++;
}

void terminal_execute_command() {
    // Add command to output
    char prompt_line[MAX_LINE_LEN];
    prompt_line[0] = '>';
    prompt_line[1] = ' ';
    strncpy(prompt_line + 2, term.input, MAX_LINE_LEN - 3);
    terminal_add_line(prompt_line);
    
    // Add to history
    terminal_add_to_history(term.input);
    
    // Parse and execute
    if (strcmp(term.input, "help") == 0) {
        terminal_add_line("Available commands:");
        terminal_add_line("  ls, cd, pwd, mkdir, touch");
        terminal_add_line("  cat, rm, echo, clear");
        terminal_add_line("  whoami, uname, help, reboot");
    }
    else if (strcmp(term.input, "clear") == 0) {
        term.line_count = 0;
    }
    else if (strcmp(term.input, "ls") == 0) {
        char buf[256];
        vfs_list(term.cwd, buf);
        if (buf[0] == '\0') {
            terminal_add_line("(empty)");
        } else {
            terminal_add_line(buf);
        }
    }
    else if (strcmp(term.input, "pwd") == 0) {
        if (term.cwd == vfs_get_root()) {
            terminal_add_line("/");
        } else {
            terminal_add_line(term.cwd->name);
        }
    }
    else if (strcmp(term.input, "whoami") == 0) {
        terminal_add_line(auth_get_current_user());
    }
    else if (strcmp(term.input, "uname") == 0) {
        terminal_add_line("AquaOS 1.0 x86_64");
    }
    else if (strncmp(term.input, "cd ", 3) == 0) {
        char* arg = term.input + 3;
        if (strcmp(arg, "..") == 0) {
            if (term.cwd->parent) {
                term.cwd = term.cwd->parent;
            }
        } else if (strcmp(arg, "/") == 0) {
            term.cwd = vfs_get_root();
        } else {
            fs_node_t* dir = vfs_find(term.cwd, arg);
            if (dir && dir->flags == FS_DIRECTORY) {
                term.cwd = dir;
            } else {
                terminal_add_line("cd: no such directory");
            }
        }
    }
    else if (strncmp(term.input, "mkdir ", 6) == 0) {
        char* arg = term.input + 6;
        if (*arg) {
            vfs_mkdir(term.cwd, arg);
            terminal_add_line("Directory created");
        }
    }
    else if (strncmp(term.input, "touch ", 6) == 0) {
        char* arg = term.input + 6;
        if (*arg) {
            vfs_creat(term.cwd, arg);
            terminal_add_line("File created");
        }
    }
    else if (strncmp(term.input, "cat ", 4) == 0) {
        char* arg = term.input + 4;
        fs_node_t* file = vfs_find(term.cwd, arg);
        if (file && file->flags == FS_FILE) {
            char* content = vfs_read(file);
            if (content) {
                terminal_add_line(content);
            } else {
                terminal_add_line("(empty file)");
            }
        } else {
            terminal_add_line("cat: file not found");
        }
    }
    else if (strncmp(term.input, "rm ", 3) == 0) {
        char* arg = term.input + 3;
        if (vfs_remove(term.cwd, arg) == 0) {
            terminal_add_line("Removed");
        } else {
            terminal_add_line("rm: file not found");
        }
    }
    else if (strncmp(term.input, "echo ", 5) == 0) {
        char* text = term.input + 5;
        char* redirect = NULL;
        
        // Find '>' for redirection
        for (int i = 5; i < term.input_len; i++) {
            if (term.input[i] == '>') {
                redirect = term.input + i + 1;
                term.input[i] = '\0';
                text = term.input + 5;
                break;
            }
        }
        
        if (redirect) {
            // Skip whitespace
            while (*redirect == ' ') redirect++;
            
            fs_node_t* file = vfs_find(term.cwd, redirect);
            if (!file) {
                file = vfs_creat(term.cwd, redirect);
            }
            if (file) {
                vfs_write(file, text);
                terminal_add_line("Written to file");
            }
        } else {
            terminal_add_line(text);
        }
    }
    else if (strcmp(term.input, "reboot") == 0) {
        outb(0x64, 0xFE);
    }
    else if (strncmp(term.input, "nano", 4) == 0) {
        // Nano text editor
        char* filename = NULL;
        if (term.input_len > 5 && term.input[4] == ' ') {
            filename = term.input + 5;
        }
        // Signal to open nano (will be handled by kernel)
        terminal_add_line("Opening nano editor...");
        // Store filename for kernel to use
        extern char nano_requested_file[256];
        extern bool nano_requested;
        if (filename) {
            int i = 0;
            while (filename[i] && i < 255) {
                nano_requested_file[i] = filename[i];
                i++;
            }
            nano_requested_file[i] = '\0';
        } else {
            nano_requested_file[0] = '\0';
        }
        nano_requested = true;
    }
    else if (term.input_len > 0) {
        terminal_add_line("Command not found");
    }
    
    // Clear input
    term.input[0] = '\0';
    term.input_len = 0;
    term.cursor_pos = 0;
    term.history_index = term.history_count;
    term.needs_redraw = true;
}

void shell_init() {
    term.line_count = 0;
    term.scroll_offset = 0;
    term.input[0] = '\0';
    term.input_len = 0;
    term.cursor_pos = 0;
    term.history_count = 0;
    term.history_index = 0;
    term.needs_redraw = true;
    
    vfs_init();
    term.cwd = vfs_get_root();
    
    terminal_add_line("AquaOS Terminal v1.0");
    terminal_add_line("Type 'help' for commands");
    terminal_add_line("");
}

void shell_handle_key(char c) {
    if (c == '\n') {
        // Enter - execute command
        terminal_execute_command();
    }
    else if (c == '\b') {
        // Backspace
        if (term.cursor_pos > 0) {
            // Shift characters left
            for (int i = term.cursor_pos - 1; i < term.input_len; i++) {
                term.input[i] = term.input[i + 1];
            }
            term.cursor_pos--;
            term.input_len--;
            term.needs_redraw = true;
        }
    }
    else if (c == 0x7F) {
        // Delete
        if (term.cursor_pos < term.input_len) {
            for (int i = term.cursor_pos; i < term.input_len; i++) {
                term.input[i] = term.input[i + 1];
            }
            term.input_len--;
            term.needs_redraw = true;
        }
    }
    else if (c == 0x1B) {
        // Escape sequences (arrows) - handled by keyboard driver
        // For now, we'll handle in a simplified way
    }
    else if (c >= 32 && c < 127) {
        // Printable character
        if (term.input_len < MAX_INPUT_LEN - 1) {
            // Shift characters right to make space
            for (int i = term.input_len; i > term.cursor_pos; i--) {
                term.input[i] = term.input[i - 1];
            }
            term.input[term.cursor_pos] = c;
            term.cursor_pos++;
            term.input_len++;
            term.input[term.input_len] = '\0';
            term.needs_redraw = true;
        }
    }
}

void shell_update(int win_x, int win_y, int win_w, int win_h) {
    // Don't clear background - it stays black and stable
    // Only draw text on top
    
    // Calculate content area
    int content_x = win_x + 12;
    int content_y = win_y + 40;
    int content_w = win_w - 24;
    int content_h = win_h - 52;
    
    int line_height = 12;
    int max_visible = (content_h - line_height) / line_height;
    
    // Determine which lines to show
    int start_line = 0;
    if (term.line_count > max_visible) {
        start_line = term.line_count - max_visible;
    }
    
    // Draw output lines
    int y = content_y;
    for (int i = start_line; i < term.line_count; i++) {
        if (y + line_height > content_y + content_h - line_height) break;
        draw_string(content_x, y, term.lines[i], 0xFF00FF00); // Green
        y += line_height;
    }
    
    // Draw prompt and input
    if (y + line_height <= content_y + content_h) {
        draw_string(content_x, y, "> ", 0xFFFFFFFF);
        draw_string(content_x + 16, y, term.input, 0xFFFFFFFF);
        
        // Draw cursor
        int cursor_x = content_x + 16 + (term.cursor_pos * 8);
        if (cursor_x < content_x + content_w - 10) {
            draw_rect(cursor_x, y, 2, 10, 0xFFFFFFFF);
        }
    }
    
    term.needs_redraw = false;
}

bool shell_needs_redraw() {
    return term.needs_redraw;
}

void shell_set_dirty() {
    term.needs_redraw = true;
}
