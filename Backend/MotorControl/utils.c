#include "utils.h"
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <pigpio.h>
#include "mqttClient.h" 
#include "queue.h"
volatile sig_atomic_t emergency_stop_triggered = 0;



void trigger_emergency_stop() {
    gpioWaveTxStop();
    emergency_stop_triggered = 0;
}
void* sequence_worker_thread(void* arg) {
    while (1) {
        char* payloadStr = dequeue(&messageQueue);
        parse_and_execute_json_sequences(payloadStr);
        free(payloadStr);
    }
    return NULL;
}