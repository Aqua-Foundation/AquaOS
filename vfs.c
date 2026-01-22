#include "vfs.h"
#include "memory.h"
#include "graphics.h" // For null check debugging if needed
#include "shell.h" // For string helpers

// Re-declare string helpers if not in a shared header (doing here for now)
int strcmp(const char* s1, const char* s2); // External from shell.c/lib
void strcpy(char* dest, const char* src);

static fs_node_t* root_node = NULL;

void vfs_init() {
    // Create Root Node "/"
    root_node = (fs_node_t*)malloc(sizeof(fs_node_t));
    strcpy(root_node->name, "/");
    root_node->flags = FS_DIRECTORY;
    root_node->first_child = NULL;
    root_node->next_sibling = NULL;
    root_node->parent = NULL;
}

fs_node_t* vfs_get_root() {
    return root_node;
}

fs_node_t* vfs_create_node(char* name, int flags) {
    fs_node_t* node = (fs_node_t*)malloc(sizeof(fs_node_t));
    strcpy(node->name, name);
    node->flags = flags;
    node->size = 0;
    node->content = NULL; // Initialize content as NULL
    node->first_child = NULL;
    node->next_sibling = NULL;
    return node;
}

// ... (keep mkdir and creat functions the same) ...

int vfs_write(fs_node_t* file, char* data) {
    if (!file || file->flags != FS_FILE) return -1;
    
    // Free old content if exists
    if (file->content) {
        free(file->content);
    }
    
    // Calculate size
    int len = 0;
    while (data[len]) len++;
    
    // Allocate new content
    file->content = (char*)malloc(len + 1);
    if (!file->content) return -1;
    
    // Copy data
    for (int i = 0; i <= len; i++) {
        file->content[i] = data[i];
    }
    
    file->size = len;
    return len;
}

char* vfs_read(fs_node_t* file) {
    if (!file || file->flags != FS_FILE) return NULL;
    return file->content;
}

int vfs_remove(fs_node_t* parent, char* name) {
    if (!parent) return -1;
    
    fs_node_t* prev = NULL;
    fs_node_t* curr = parent->first_child;
    
    while (curr) {
        if (strcmp(curr->name, name) == 0) {
            // Found the node to remove
            if (prev) {
                prev->next_sibling = curr->next_sibling;
            } else {
                parent->first_child = curr->next_sibling;
            }
            
            // Free content if it's a file
            if (curr->content) {
                free(curr->content);
            }
            
            free(curr);
            return 0;
        }
        prev = curr;
        curr = curr->next_sibling;
    }
    
    return -1; // Not found
}

fs_node_t* vfs_mkdir(fs_node_t* parent, char* name) {
    if (!parent) return NULL;
    fs_node_t* node = vfs_create_node(name, FS_DIRECTORY);
    node->parent = parent;
    
    // Add to parent list
    if (parent->first_child == NULL) {
        parent->first_child = node;
    } else {
        fs_node_t* curr = parent->first_child;
        while (curr->next_sibling) curr = curr->next_sibling;
        curr->next_sibling = node;
    }
    return node;
}

fs_node_t* vfs_creat(fs_node_t* parent, char* name) {
    if (!parent) return NULL;
    fs_node_t* node = vfs_create_node(name, FS_FILE);
    node->parent = parent;

    if (parent->first_child == NULL) {
        parent->first_child = node;
    } else {
        fs_node_t* curr = parent->first_child;
        while (curr->next_sibling) curr = curr->next_sibling;
        curr->next_sibling = node;
    }
    return node;
}

fs_node_t* vfs_find(fs_node_t* parent, char* name) {
    if (!parent) return NULL;
    fs_node_t* curr = parent->first_child;
    while (curr) {
        if (strcmp(curr->name, name) == 0) return curr;
        curr = curr->next_sibling;
    }
    return NULL;
}

// Very basic LS implementation that writes names to a buffer (separated by spaces)
void vfs_list(fs_node_t* parent, char* output_buffer) {
    if (!parent || !output_buffer) return;
    output_buffer[0] = '\0';
    
    fs_node_t* curr = parent->first_child;
    while (curr) {
        // Simple string concat: dest + name + " "
        // Not safe but works for MiniOS demo
        char* d = output_buffer;
        while (*d) d++; // Find end
        
        char* s = curr->name;
        while (*s) *d++ = *s++;
        
        // Append suffix "/" for dirs
        if (curr->flags == FS_DIRECTORY) *d++ = '/';
        
        *d++ = ' ';
        *d = '\0';
        
        curr = curr->next_sibling;
    }
}
