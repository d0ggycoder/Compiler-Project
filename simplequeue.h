#ifndef _simplequeue
#define _simplequeue
#include "simplevector.h"
typedef struct Queue Queue;

Queue* queue_make(Vector* storage);
void* queue_peek(Queue* queue);
void* queue_consume(Queue* queue);

#endif