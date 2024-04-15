#ifndef UTILS_H
#define UTILS_H

#include <pthread.h>
#include "signal.h"

extern volatile sig_atomic_t emergency_stop_triggered;
extern pthread_mutex_t waveMutex;
extern pthread_cond_t waveCond;
extern volatile int waveTransmissionActive;
void trigger_emergency_stop();
void* waveWatchdog(void* arg);
void* message_processing_thread(void* arg);
#endif
