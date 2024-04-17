#include "utils.h"
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <pigpio.h>
#include "mqttClient.h" 
volatile sig_atomic_t emergency_stop_triggered = 0;


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
