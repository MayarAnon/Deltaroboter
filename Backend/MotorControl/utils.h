#ifndef UTILS_H
#define UTILS_H

#include <pthread.h>
#include "signal.h"

extern volatile sig_atomic_t emergency_stop_triggered;
void trigger_emergency_stop();
void* message_processing_thread(void* arg);
#endif
