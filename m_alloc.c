#include <stdio.h>
#include <stdlib.h>

#define MEMORY_POOL_SIZE 1024

char memory_pool[MEMORY_POOL_SIZE];

typedef struct {
    size_t size;  // Size of the block
    int is_free;  // 1 if free, 0 if allocated
} MetaData;

size_t internal_fragmentation = 0;

void memory_init() {
    MetaData* initial_block = (MetaData*)memory_pool;
    initial_block->size = MEMORY_POOL_SIZE - sizeof(MetaData);
    initial_block->is_free = 1;
}

void* my_malloc(size_t size) {
    MetaData* curr = (MetaData*)memory_pool;

    while ((char*)curr < memory_pool + MEMORY_POOL_SIZE) {
        if (curr->is_free && curr->size >= size) {
            size_t actual_size = (size + 3) & ~3; // Align size to 4 bytes
            size_t padding = actual_size - size;
            internal_fragmentation += padding;

            // Split the block if there's extra space
            if (curr->size > actual_size + sizeof(MetaData)) {
                MetaData* new_block = (MetaData*)((char*)curr + sizeof(MetaData) + actual_size);
                new_block->size = curr->size - actual_size - sizeof(MetaData);
                new_block->is_free = 1;

                curr->size = actual_size;
            }

            curr->is_free = 0;
            return (void*)((char*)curr + sizeof(MetaData));
        }
        curr = (MetaData*)((char*)curr + sizeof(MetaData) + curr->size);
    }

    return NULL; // No suitable block found
}

void my_free(void* ptr) {
    if (ptr == NULL) {
        return;
    }

    MetaData* block_to_free = (MetaData*)((char*)ptr - sizeof(MetaData));
    block_to_free->is_free = 1;

    // Merge with next block if free
    MetaData* next = (MetaData*)((char*)block_to_free + sizeof(MetaData) + block_to_free->size);
    if ((char*)next < memory_pool + MEMORY_POOL_SIZE && next->is_free) {
        block_to_free->size += sizeof(MetaData) + next->size;
    }
}

size_t ext_frag(size_t size) {
    size_t min_usable_size = size + sizeof(MetaData);
    MetaData* curr = (MetaData*)memory_pool;
    size_t ext_frag_cnt = 0;

    while ((char*)curr < memory_pool + MEMORY_POOL_SIZE) {
        if (curr->is_free && curr->size < min_usable_size) {
            ext_frag_cnt++;
        }
        curr = (MetaData*)((char*)curr + sizeof(MetaData) + curr->size);
    }

    return ext_frag_cnt;
}


int main() {
  memory_init();

  void*p1 = my_malloc(100);
  void*p2 = my_malloc(50);

  my_free(p1);

  size_t ext_fragments = ext_frag(16);
  printf("External fragmentation: %zu gaps\n", ext_fragments);
  printf("Internal fragmentation: %zu bytes\n", internal_fragmentation);

  return 0;
}
