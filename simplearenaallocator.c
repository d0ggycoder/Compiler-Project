#include "simplearenaallocator.h"
#include <stdlib.h>
#include <string.h>

#define ALLOCATOR_MIN 4096
#define ALLOCATOR_MAX 65536
#define RESCALE_FUNC(n) 2*n
#define CLAMP(n,a,b) n >= a ? (n <= b ? n : b) : a 
#define RESCALE(n) CLAMP(RESCALE_FUNC(n),ALLOCATOR_MIN,ALLOCATOR_MAX)

typedef struct Chunk Chunk;

struct Chunk{
    void* memory;
    size_t capacity;
    size_t occupied;
    size_t idealCapacity;
    Chunk* next;
};

struct Allocator{
    Chunk* first;
    Chunk* current;
};

Allocator* allocator_new(size_t defaultCapacity){
    Allocator* alloc = (Allocator*)malloc(sizeof(Allocator));
    Chunk* fchunk = (Chunk*)malloc(sizeof(Chunk));
    fchunk->capacity = defaultCapacity > 0 ? defaultCapacity : ALLOCATOR_MIN;
    fchunk->memory = malloc(fchunk->capacity);
    fchunk->occupied = 0;
    fchunk->next = NULL;

    alloc->first = fchunk;
    alloc->current = fchunk;
}

Chunk* makeChunk(Chunk* cchunk, size_t amount){
    size_t idealSize = RESCALE(cchunk->idealCapacity);
    size_t capacity = amount > idealSize ? amount : idealSize;

    Chunk* nchunk = (Chunk*)malloc(sizeof(Chunk));
    nchunk->capacity = capacity;
    nchunk->memory = malloc(capacity);
    nchunk->idealCapacity = idealSize;
    nchunk->occupied = 0;
    nchunk->next = NULL;
}

void* allocator_alloc(Allocator* allocator, size_t amount){
    Chunk* cchunk = allocator->current;
    if(cchunk->occupied + amount > cchunk->capacity){
        //cchunk->next may not exist
        if(cchunk->next == NULL){
            cchunk->next = makeChunk(cchunk,amount);
            cchunk = cchunk->next;
        } else if(amount > cchunk->next->capacity){
            Chunk* nchunk = cchunk->next;
            free(nchunk->memory);
            nchunk->capacity = amount;
            nchunk->memory = malloc(amount);
            nchunk->occupied = 0;
        } else {
            cchunk = cchunk->next;
        }
    }
    void* mem = allocator->current->memory + allocator->current->occupied;
    allocator->current->occupied+=amount;
    return mem;
}

void* allocator_alloc_align(Allocator* allocator, size_t amount, size_t align){
    allocator->current->occupied = allocator->current->occupied & ~(align-1) + align;
    return allocator_alloc(allocator,amount);
}

void allocator_clear(Allocator* alloc){
    Chunk* chunk = alloc->first;
    while(chunk != NULL){
        chunk->occupied = 0;
        chunk=chunk->next;
    }
    alloc->current = alloc->first;
}

void allocator_free(Allocator* alloc){
    Chunk* chunk = alloc->first;
    while(chunk != NULL){
        Chunk* temp = chunk;
        chunk = chunk->next;
        free(temp->memory);
        free(temp);
    }
}