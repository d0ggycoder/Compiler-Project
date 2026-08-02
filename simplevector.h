#ifndef _simplevector
#define _simplevector
#include <stddef.h>

typedef struct Vector Vector;
Vector* vector_new(size_t itemSize);
Vector* vector_from_capacity(size_t itemSize, size_t startingLength);
Vector* vector_from(size_t itemSize, size_t startingLength, void* data);
void* vector_get(Vector* vector, int i);
void vector_append(Vector* vector, void* value);
size_t vector_size(Vector* vector);
size_t vector_item_size(Vector* vector);
void vector_free(Vector* vector);
void vector_freeCustom(Vector* vector, void freeCustom(void*));

#endif