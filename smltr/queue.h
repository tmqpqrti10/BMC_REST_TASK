#include "head.h"

#ifndef __QUEUE_h
#define __QUEUE_h

typedef struct _queue_item{
        dev_handler* contents;
        struct _queue_item* next;
}queue_item;

typedef struct _queue_root{
        struct _queue_item* head;
        struct _queue_item* tail;
}queue_root;

void init_queue(queue_root* queue);
void push_queue(queue_root* queue, dev_handler* contents);
dev_handler* pop_queue(queue_root* queue);

#endif
