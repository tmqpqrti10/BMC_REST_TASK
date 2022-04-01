#include "queue.h"

void init_queue(queue_root* queue){
        queue->head = queue->tail = NULL;
}

void push_queue(queue_root* queue, dev_handler* contents){
        queue_item *item = malloc(sizeof(item));
        item->contents = contents;
        item->next = NULL;
        if (queue->head == NULL){
                queue->head = queue->tail = item;
        } else {
                queue->tail = queue->tail->next = item;
        }
}

dev_handler* pop_queue(queue_root* queue){
        dev_handler* popped;
        if (queue->head == NULL){
                return NULL;
        } else {
                popped = queue->head->contents;
                queue_item* next = queue->head->next;
                free(queue->head);
                queue->head = next;
                if (queue->head == NULL)
                        queue->tail = NULL;
        }
        return popped;
}
