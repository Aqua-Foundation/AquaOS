#include "memory.h"

#define HEAP_SIZE 1024 * 1024 * 32 // 32 MB Heap

// Static heap to avoid complex paging logic for this demo
static uint8_t heap_data[HEAP_SIZE];

typedef struct block_header {
    size_t size;
    struct block_header* next;
    int is_free;
} block_header_t;

static block_header_t* head = NULL;

void memory_init() {
    head = (block_header_t*)heap_data;
    head->size = HEAP_SIZE - sizeof(block_header_t);
    head->next = NULL;
    head->is_free = 1;
}

void* malloc(size_t size) {
    if (size == 0) return NULL;
    
    // Align size to 16 bytes
    size_t aligned_size = (size + 15) & ~15;
    
    block_header_t* curr = head;
    while (curr) {
        if (curr->is_free && curr->size >= aligned_size) {
            // Found a block. Can we split it?
            if (curr->size > aligned_size + sizeof(block_header_t) + 16) {
                // Split
                block_header_t* new_block = (block_header_t*)((uint8_t*)curr + sizeof(block_header_t) + aligned_size);
                new_block->size = curr->size - aligned_size - sizeof(block_header_t);
                new_block->is_free = 1;
                new_block->next = curr->next;
                
                curr->size = aligned_size;
                curr->next = new_block;
            }
            curr->is_free = 0;
            return (void*)((uint8_t*)curr + sizeof(block_header_t));
        }
        curr = curr->next;
    }
    return NULL; // OOM
}

void free(void* ptr) {
    if (!ptr) return;
    block_header_t* block = (block_header_t*)((uint8_t*)ptr - sizeof(block_header_t));
    block->is_free = 1;
    
    // Merge only effectively next block for simplicity in this demo (Coalescing)
    if (block->next && block->next->is_free) {
        block->size += sizeof(block_header_t) + block->next->size;
        block->next = block->next->next;
    }
}

void* memset(void* ptr, int value, size_t num) {
    unsigned char* p = ptr;
    while (num--) *p++ = (unsigned char)value;
    return ptr;
}

void* memcpy(void* dest, const void* src, size_t num) {
    unsigned char* d = dest;
    const unsigned char* s = src;
    while (num--) *d++ = *s++;
    return dest;
}
