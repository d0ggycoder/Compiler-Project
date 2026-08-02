#include <stdlib.h>
#include <string.h>
#include "simplevector.h"
struct Vector{
    void* data;
    size_t itemSize;
    size_t length;
    size_t capacity;
};

Vector* vector_new(size_t itemSize){
    Vector* vec = (Vector*) malloc(sizeof(Vector));
    vec->data = NULL;
    vec->itemSize = itemSize;
    vec->length = 0;
    vec->capacity = 0;
    return vec;
}

Vector* vector_from(size_t itemSize, size_t dataLength, void* startingData){
    Vector* vec = (Vector*) malloc(sizeof(Vector));
    vec->length = dataLength*itemSize;
    vec->capacity = dataLength*itemSize;
    vec->itemSize = itemSize;
    vec->data = malloc(vec->capacity);
    memcpy(vec->data, startingData, vec->capacity);
    return vec;
}

Vector* vector_from_capacity(size_t itemSize, size_t dataLength){
    Vector* vec = (Vector*) malloc(sizeof(Vector));
    vec->length = dataLength*itemSize;
    vec->capacity = dataLength*itemSize;
    vec->itemSize = itemSize;
    vec->data = malloc(vec->capacity);
    return vec;
}

void* vector_get(Vector* vec, int i){
    return vec->data + i*vec->itemSize;
}

void vector_append(Vector* vec, void* v){
    if(vec->length + vec->itemSize > vec->capacity){
        vec->capacity = vec->capacity*2+vec->itemSize;
        void* newData = malloc(vec->capacity);
        memcpy(newData,vec->data,vec->length);
        free(vec->data);
        vec->data = newData;
    }
    memcpy(vec->data + (vec->length),v,vec->itemSize);
    vec->length+=vec->itemSize;
}

size_t vector_size(Vector* vec){
    return vec->length/vec->itemSize;
}

size_t vector_item_size(Vector* vec){
    return vec->itemSize;
}

void vector_free(Vector* vec){
    free(vec->data);
    free(vec);
}

void vector_free_custom(Vector* vec, void free_custom(void*)){
    for(int i=0;i<vec->length;i+=vec->itemSize){
        free_custom(vec->data+i);
    }
    free(vec->data);
    free(vec);
}
