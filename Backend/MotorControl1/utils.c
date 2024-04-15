#include "utils.h"
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <pigpio.h>
#include "mqttClient.h" 
volatile sig_atomic_t emergency_stop_triggered = 0;
volatile int waveTransmissionActive = 0;
pthread_mutex_t waveMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t waveCond = PTHREAD_COND_INITIALIZER;

void* waveWatchdog(void* arg) {
    while (1) {
        pthread_mutex_lock(&waveMutex);
        while (waveTransmissionActive && gpioWaveTxBusy()) {
            pthread_cond_wait(&waveCond, &waveMutex);
        }
        pthread_mutex_unlock(&waveMutex);
        usleep(100);  // Überwachungsintervall, eventuell anpassen
    }
    return NULL;
}

void* message_processing_thread(void* arg) {
    char* payloadStr = (char*) arg;
    parse_and_execute_json_sequences(payloadStr);
    free(payloadStr);
    return NULL;
}

void trigger_emergency_stop() {
    gpioWaveTxStop();
    emergency_stop_triggered = 0;
}
