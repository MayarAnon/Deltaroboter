#ifndef UTILS_H
#define UTILS_H

#include <pthread.h>
#include "signal.h"
#include <semaphore.h>
// Externe Variablen und Funktionen
extern sem_t queueSemaphore;
extern volatile sig_atomic_t emergency_stop_triggered;

void trigger_emergency_stop();
void* sequence_worker_thread(void* arg);
#endif
