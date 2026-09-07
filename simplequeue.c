#include "simplequeue.h"
#include "simplevector.h"
#include <stdlib.h>
#include <string.h>

struct Queue{
    Vector* storage;
    size_t i;
};

Queue* queue_new(Vector* vec){
    Queue* queue = (Queue*) malloc(sizeof(Queue));
    queue->storage = vec;
    queue->i=0;
}

void* queue_peek(Queue* queue){
    return vector_get(queue->storage,queue->i);
}

void* queue_ahead(Queue* queue, size_t n){
    if(queue->i+n > vector_size(queue->storage)) return NULL;
    return vector_get(queue->storage,queue->i+n);
}

void* queue_consume(Queue* queue){
    if(queue->i >= vector_size(queue->storage)) return NULL;
    return vector_get(queue->storage,(queue->i)++);
}

void queue_back(Queue* queue, int n){
    queue->i = queue->i>=n?queue->i-n:0;
}

unsigned char queue_match(Queue* queue, void* v){
    if(memcmp(vector_get(queue->storage,queue->i),v,vector_item_size(queue->storage)) != 0){
        return 0;
    }
    (queue->i)++;
    return 1;
}