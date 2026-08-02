#include "simplequeue.h"
#include "simplevector.h"
#include <stdlib.h>

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

void* queue_consume(Queue* queue){
    if(queue->i >= vector_size(queue->storage)) return NULL;
    return vector_get(queue->storage,(queue->i)++);
}