#ifndef _simplequeue
#define _simplequeue
#include "simplevector.h"
typedef struct Queue Queue;

Queue* queue_new(Vector* storage);
void* queue_peek(Queue* queue);
void* queue_ahead(Queue* queue, size_t n);
void* queue_consume(Queue* queue);

#endif