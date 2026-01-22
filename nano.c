#include "nano.h"
#include "graphics.h"
#include "vfs.h"
#include "memory.h"

// External string helpers
extern int strcmp(const char* s1, const char* s2);
extern void strcpy(char* dest, const char* src);
extern int strlen(const char* str);
extern void strncpy(char* dest, const char* src, int n);

#define MAX_LINES 100
#define MAX_LINE_LEN 256

typedef struct {
    char lines[MAX_LINES][MAX_LINE_LEN];
    int line_count;
    int cursor_line;
    int cursor_col;
    char filename[256];
    bool modified;
    bool needs_redraw;
    bool is_active;
    fs_node_t* cwd;
} nano_state_t;

static nano_state_t nano;

void nano_init() {
    nano.line_count = 1;
    nano.cursor_line = 0;
    nano.cursor_col = 0;
    nano.filename[0] = '\0';
    nano.modified = false;
    nano.needs_redraw = true;
    nano.is_active = false;
    nano.cwd = vfs_get_root();
    
    // Initialize with one empty line
    nano.lines[0][0] = '\0';
}

void nano_open(char* filename) {
    nano_init();
    nano.is_active = true;
    
    if (filename && filename[0] != '\0') {
        strcpy(nano.filename, filename);
        
        // Try to load file from VFS
        fs_node_t* file = vfs_find(nano.cwd, filename);
        if (file && file->flags == FS_FILE) {
            char* content = vfs_read(file);
            if (content) {
                // Parse content into lines
                int line = 0;
                int col = 0;
                for (int i = 0; content[i] != '\0' && line < MAX_LINES; i++) {
                    if (content[i] == '\n') {
                        nano.lines[line][col] = '\0';
                        line++;
                        col = 0;
                    } else if (col < MAX_LINE_LEN - 1) {
                        nano.lines[line][col] = content[i];
                        col++;
                    }
                }
                nano.lines[line][col] = '\0';
                nano.line_count = line + 1;
            }
        }
    }
    
    nano.needs_redraw = true;
}

void nano_save() {
    if (nano.filename[0] == '\0') {
        return; // No filename specified
    }
    
    // Find or create file
    fs_node_t* file = vfs_find(nano.cwd, nano.filename);
    if (!file) {
        file = vfs_creat(nano.cwd, nano.filename);
    }
    
    if (file) {
        // Combine all lines into one string
        char buffer[MAX_LINES * MAX_LINE_LEN];
        int pos = 0;
        
        for (int i = 0; i < nano.line_count && pos < sizeof(buffer) - 2; i++) {
            int j = 0;
            while (nano.lines[i][j] != '\0' && pos < sizeof(buffer) - 2) {
                buffer[pos++] = nano.lines[i][j++];
            }
            if (i < nano.line_count - 1) {
                buffer[pos++] = '\n';
            }
        }
        buffer[pos] = '\0';
        
        vfs_write(file, buffer);
        nano.modified = false;
        nano.needs_redraw = true;
    }
}

void nano_handle_key(char c) {
    if (!nano.is_active) return;
    
    if (c == 15) { // Ctrl+O - Save
        nano_save();
    }
    else if (c == 24) { // Ctrl+X - Exit
        nano.is_active = false;
        nano.needs_redraw = true;
    }
    else if (c == '\n') {
        // Enter - new line
        if (nano.line_count < MAX_LINES) {
            // Shift lines down
            for (int i = nano.line_count; i > nano.cursor_line + 1; i--) {
                strcpy(nano.lines[i], nano.lines[i - 1]);
            }
            
            // Split current line
            strcpy(nano.lines[nano.cursor_line + 1], &nano.lines[nano.cursor_line][nano.cursor_col]);
            nano.lines[nano.cursor_line][nano.cursor_col] = '\0';
            
            nano.line_count++;
            nano.cursor_line++;
            nano.cursor_col = 0;
            nano.modified = true;
            nano.needs_redraw = true;
        }
    }
    else if (c == '\b') {
        // Backspace
        if (nano.cursor_col > 0) {
            // Delete character before cursor
            int len = strlen(nano.lines[nano.cursor_line]);
            for (int i = nano.cursor_col - 1; i < len; i++) {
                nano.lines[nano.cursor_line][i] = nano.lines[nano.cursor_line][i + 1];
            }
            nano.cursor_col--;
            nano.modified = true;
            nano.needs_redraw = true;
        } else if (nano.cursor_line > 0) {
            // Join with previous line
            int prev_len = strlen(nano.lines[nano.cursor_line - 1]);
            if (prev_len + strlen(nano.lines[nano.cursor_line]) < MAX_LINE_LEN - 1) {
                strcpy(&nano.lines[nano.cursor_line - 1][prev_len], nano.lines[nano.cursor_line]);
                
                // Shift lines up
                for (int i = nano.cursor_line; i < nano.line_count - 1; i++) {
                    strcpy(nano.lines[i], nano.lines[i + 1]);
                }
                
                nano.line_count--;
                nano.cursor_line--;
                nano.cursor_col = prev_len;
                nano.modified = true;
                nano.needs_redraw = true;
            }
        }
    }
    else if (c >= 32 && c < 127) {
        // Printable character
        int len = strlen(nano.lines[nano.cursor_line]);
        if (len < MAX_LINE_LEN - 1) {
            // Shift characters right
            for (int i = len; i >= nano.cursor_col; i--) {
                nano.lines[nano.cursor_line][i + 1] = nano.lines[nano.cursor_line][i];
            }
            nano.lines[nano.cursor_line][nano.cursor_col] = c;
            nano.cursor_col++;
            nano.modified = true;
            nano.needs_redraw = true;
        }
    }
}

void nano_render(int win_x, int win_y, int win_w, int win_h) {
    if (!nano.needs_redraw) return;
    
    int content_x = win_x + 12;
    int content_y = win_y + 40;
    int content_w = win_w - 24;
    int content_h = win_h - 70;
    
    int line_height = 12;
    int max_visible = content_h / line_height;
    
    // Determine visible range
    int start_line = 0;
    if (nano.cursor_line >= max_visible) {
        start_line = nano.cursor_line - max_visible + 1;
    }
    
    // Draw lines
    int y = content_y;
    for (int i = start_line; i < nano.line_count && i < start_line + max_visible; i++) {
        draw_string(content_x, y, nano.lines[i], 0xFFFFFFFF);
        y += line_height;
    }
    
    // Draw cursor
    int cursor_y = content_y + (nano.cursor_line - start_line) * line_height;
    int cursor_x = content_x + (nano.cursor_col * 8);
    if (cursor_y >= content_y && cursor_y < content_y + content_h) {
        draw_rect(cursor_x, cursor_y, 2, 10, 0xFFFFFFFF);
    }
    
    // Draw status bar
    int status_y = win_y + win_h - 24;
    char status[256];
    
    // Filename and modified indicator
    if (nano.filename[0] != '\0') {
        strcpy(status, "File: ");
        int pos = 6;
        int i = 0;
        while (nano.filename[i] && pos < 200) {
            status[pos++] = nano.filename[i++];
        }
        status[pos] = '\0';
    } else {
        strcpy(status, "File: [New File]");
    }
    
    if (nano.modified) {
        int len = strlen(status);
        status[len] = ' ';
        status[len + 1] = '[';
        status[len + 2] = '+';
        status[len + 3] = ']';
        status[len + 4] = '\0';
    }
    
    draw_string(content_x, status_y, status, 0xFFFFFF00); // Yellow
    
    // Help text
    draw_string(content_x, status_y + 12, "^O Save  ^X Exit", 0xFF888888);
    
    nano.needs_redraw = false;
}

bool nano_needs_redraw() {
    return nano.needs_redraw;
}

bool nano_is_active() {
    return nano.is_active;
}

void nano_close() {
    nano.is_active = false;
    nano.needs_redraw = false;
}
