#ifndef VFS_H
#define VFS_H

#include <stdint.h>

#define FS_FILE 0
#define FS_DIRECTORY 1
#define MAX_FILE_CONTENT 4096

typedef struct fs_node {
    char name[32];
    uint32_t flags; // 0=file, 1=dir
    uint32_t size;
    char* content; // File content (dynamically allocated)
    struct fs_node* first_child;
    struct fs_node* next_sibling;
    struct fs_node* parent;
} fs_node_t;

void vfs_init();
fs_node_t* vfs_mkdir(fs_node_t* parent, char* name);
fs_node_t* vfs_creat(fs_node_t* parent, char* name);
fs_node_t* vfs_find(fs_node_t* parent, char* name);
void vfs_list(fs_node_t* parent, char* output_buffer); // Primitive ls
fs_node_t* vfs_get_root();
int vfs_write(fs_node_t* file, char* data);
char* vfs_read(fs_node_t* file);
int vfs_remove(fs_node_t* parent, char* name);

#endif
