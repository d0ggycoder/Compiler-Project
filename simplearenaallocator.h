#ifndef _simplearenaallocator
#define _simplearenaallocator
#include <stddef.h>
typedef struct Allocator Allocator;
typedef struct AllocatorState AllocatorState;

Allocator* allocator_new(size_t defaultCapacity);
void* allocator_alloc(Allocator* allocator, size_t amount);
void allocator_free(Allocator* allocator);

#endif