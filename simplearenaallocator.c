#include "simplearenaallocator.h"
#include <stdlib.h>
#include <string.h>

#define ALLOCATOR_MIN 64
#define RESCALE(n) 2*n

struct Allocator{
    void* memory;
    size_t capacity;
    size_t i;
};

Allocator* allocator_new(size_t defaultCapacity){
    Allocator* alloc = (Allocator*)malloc(sizeof(Allocator));
    alloc->capacity = defaultCapacity > 0 ? defaultCapacity : ALLOCATOR_MIN;
    alloc->i=0;
}

void* allocator_alloc(Allocator* allocator, size_t amount){
    if(amount + allocator->i > allocator->capacity){
        void* newMem = malloc(RESCALE(allocator->capacity));
        memcpy(newMem, allocator->memory, allocator->i);
        free(allocator->memory);
        allocator->memory = newMem;
    }
    void* mem = allocator->memory+allocator->i;
    allocator->i+=amount;
    return mem;
}

void allocator_free(Allocator* alloc){
    free(alloc->memory);
    free(alloc);
}