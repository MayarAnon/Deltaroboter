#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>
#include <semaphore.h>

typedef struct node {
    char* data;
    struct node* next;
} Node;

typedef struct {
    Node* head;
    Node* tail;
    pthread_mutex_t lock;
} Queue;

void initQueue(Queue* q);
void enqueue(Queue* q, char* data);
char* dequeue(Queue* q);
void clearQueue(Queue* q);
#endif
