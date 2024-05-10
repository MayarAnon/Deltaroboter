#include "utils.h"
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <pigpio.h>
#include "mqttClient.h" 
#include "queue.h"

// Globale Variable zur Anzeige eines Notstopps
volatile sig_atomic_t emergency_stop_triggered = 0;


// Auslösen eines Notstopps
void trigger_emergency_stop() {
    gpioWaveTxStop(); // Stoppen der PWM-Wellenformen
    clearQueue(&messageQueue); // Leeren der Nachrichten-Warteschlange
    emergency_stop_triggered = 0; // Zurücksetzen des Notstopps
}
// Worker-Thread für die Ausführung von Sequenzen
void* sequence_worker_thread(void* arg) {
    while (1) {
        char* payloadStr = dequeue(&messageQueue);// Entnehmen einer Nachricht aus der Warteschlange
        parse_and_execute_json_sequences(payloadStr);// Parsen und Ausführen der JSON-Sequenzen
        free(payloadStr);// Freigabe des Speichers der Nachricht
    }
    return NULL;
}