// queue.c
#include "queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include "utils.h"
void initQueue(Queue* q) {
    q->head = NULL;
    q->tail = NULL;
    pthread_mutex_init(&q->lock, NULL);
}

void enqueue(Queue* q, char* data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        perror("Failed to allocate node");
        exit(EXIT_FAILURE);
    }
    newNode->data = data;
    newNode->next = NULL;

    pthread_mutex_lock(&q->lock);
    if (q->tail != NULL) {
        q->tail->next = newNode;
    }
    q->tail = newNode;
    if (q->head == NULL) {
        q->head = newNode;
    }
    pthread_mutex_unlock(&q->lock);
    sem_post(&queueSemaphore);
}

char* dequeue(Queue* q) {
    sem_wait(&queueSemaphore);
    pthread_mutex_lock(&q->lock);
    if (q->head == NULL) {
        pthread_mutex_unlock(&q->lock);
        return NULL; // Warteschlange ist leer
    }
    Node* temp = q->head;
    char* data = temp->data;
    q->head = q->head->next;
    if (q->head == NULL) {
        q->tail = NULL;
    }
    pthread_mutex_unlock(&q->lock);
    free(temp);
    return data;
}
void clearQueue(Queue* q) {
    pthread_mutex_lock(&q->lock);
    Node* current = q->head;
    Node* next;

    while (current != NULL) {
        next = current->next;
        free(current->data); // Gehe sicher, dass der Speicher für data auch freigegeben wird
        free(current);
        current = next;
    }

    q->head = NULL;
    q->tail = NULL;
    pthread_mutex_unlock(&q->lock);

    // Nach dem Leeren, setze das Semaphore zurück, um falsche Trigger zu vermeiden
    int sval;
    while (sem_getvalue(&queueSemaphore, &sval) == 0 && sval > 0) {
        sem_wait(&queueSemaphore);
    }
}
