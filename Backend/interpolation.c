// gcc -o interpolation interpolation.c -I/usr/local/include/cjson -L/usr/local/lib/cjson -lpigpio -lrt -pthread -lpaho-mqtt3c -lcjson
//Beispiel nachricht: 
//[{\"motors\": [100, 100, 100],\"pulswidth\"=10}, {\"motors\": [-10, 0, 20], \"pulswidth\"=10}, {\"motors\": [5, 2, 3],\"pulswidth\"=10}] 

#include <pigpio.h>
#include <stdio.h>
#include <unistd.h> // Für usleep-Funktionen
#include <stdlib.h>
#include "MQTTClient.h"
#include "cJSON.h"
#include <string.h>

// Konstanten für die Motor-GPIO-Pins und Pulse
#define MOTOR_COUNT 3
#define SEQUENCE_COUNT 46

// MQTT-Definitions
#define ADDRESS     "tcp://localhost:1883"
#define CLIENTID    "MotorControllerClient"
#define TOPIC       "motor/sequence"
#define QOS         1

const int motor_gpios[MOTOR_COUNT] = {17, 27, 24};
const int dir_gpios[MOTOR_COUNT] = {23, 9, 7};
const int pulseWidthDefault = 500; // Pulsbreite in Mikrosekunden
const int pauseBetweenPulsesDefault = 500; // Pause zwischen Pulsen in Mikrosekunden
const int directionChangeDelayDefault = 5;
MQTTClient client;


// Prototypen
void initialize_motors();
void execute_interpolated_sequence(int pulses[MOTOR_COUNT], int pulseWidthUs, int pauseBetweenPulsesUs, int directionChangeDelayUs);
void initialize_mqtt();
int on_message(void *context, char *topicName, int topicLen, MQTTClient_message *message);
void parse_and_execute_json_sequences(const char* jsonString);

void parse_and_execute_json_sequences(const char* jsonString) {
    cJSON *json = cJSON_Parse(jsonString);
    if (json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Fehler vor: %s\n", error_ptr);
        }
        return;
    }

    const cJSON *sequence = NULL;
    cJSON_ArrayForEach(sequence, json) {
        cJSON *motors = cJSON_GetObjectItemCaseSensitive(sequence, "motors");
        cJSON *pulseWidth = cJSON_GetObjectItemCaseSensitive(sequence, "pulswidth");
        int sequencePulseWidth = pulseWidthDefault; // Verwenden des Standardwerts

        if (cJSON_IsArray(motors)) {
            int pulses[MOTOR_COUNT];
            for (int i = 0; i < cJSON_GetArraySize(motors); i++) {
                cJSON *pulse = cJSON_GetArrayItem(motors, i);
                pulses[i] = pulse->valueint;
            }
            // Überprüfen, ob ein 'pulseWidth'-Wert vorhanden ist, und diesen verwenden, falls ja
            if (pulseWidth && cJSON_IsNumber(pulseWidth)) {
                sequencePulseWidth = pulseWidth->valueint;
            }
            execute_interpolated_sequence(pulses, sequencePulseWidth, pauseBetweenPulsesDefault, directionChangeDelayDefault);
        }
    }

    cJSON_Delete(json);
}

void initialize_motors() {
    if (gpioInitialise() < 0) {
        fprintf(stderr, "Pigpio-Initialisierung fehlgeschlagen\n");
        exit(1);
    }

    for (int i = 0; i < MOTOR_COUNT; ++i) {
        gpioSetMode(motor_gpios[i], PI_OUTPUT);
        gpioSetMode(dir_gpios[i], PI_OUTPUT); // Initialisiere Richtungs-Pins als Ausgänge
    }
}

void execute_interpolated_sequence(int pulses[MOTOR_COUNT], int pulseWidthUs, int pauseBetweenPulsesUs, int directionChangeDelayUs) {
    gpioWaveClear();

    int maxPulses = 0;
    for (int i = 0; i < MOTOR_COUNT; ++i) {
        maxPulses = (abs(pulses[i]) > maxPulses) ? abs(pulses[i]) : maxPulses;
    }

    gpioPulse_t combinedPulses[(maxPulses + 2) * MOTOR_COUNT * 2];
    int pulseIndex = 0;

    // Zu Beginn der Sequenz: Richtungsänderung einfügen, falls notwendig, mit einer Verzögerung vor dem ersten Puls
    for (int i = 0; i < MOTOR_COUNT; ++i) {
        if (pulses[i] < 0) { // Nur wenn die Pulse negativ sind, setze den dir_GPIO auf HIGH
            combinedPulses[pulseIndex++] = (gpioPulse_t){
                .gpioOn = (1 << dir_gpios[i]),
                .gpioOff = 0,
                .usDelay = directionChangeDelayUs // Verzögerung für den Richtungswechsel aus Parameter
            };
        }
    }

    // Generiere Pulse für jeden Motor
    for (int pulseNum = 0; pulseNum < maxPulses; ++pulseNum) {
        unsigned int bitsOn = 0, bitsOff = 0;
        for (int motorIndex = 0; motorIndex < MOTOR_COUNT; ++motorIndex) {
            if (pulseNum < abs(pulses[motorIndex])) {
                bitsOn |= (1 << motor_gpios[motorIndex]);
            } else {
                bitsOff |= (1 << motor_gpios[motorIndex]);
            }
        }
        combinedPulses[pulseIndex++] = (gpioPulse_t){.gpioOn = bitsOn, .gpioOff = bitsOff, .usDelay = pulseWidthUs};
        combinedPulses[pulseIndex++] = (gpioPulse_t){.gpioOn = 0, .gpioOff = bitsOn, .usDelay = pauseBetweenPulsesUs};
    }

    // Richtungsänderungen am Ende der Sequenz einfügen, falls notwendig
    for (int i = 0; i < MOTOR_COUNT; ++i) {
        if (pulses[i] < 0) { // Setze den dir_GPIO zurück auf LOW nach negativen Pulsen
            combinedPulses[pulseIndex++] = (gpioPulse_t){
                .gpioOn = 0,
                .gpioOff = (1 << dir_gpios[i]),
                .usDelay = directionChangeDelayUs // Gleich wie bei Start für Konsistenz, kann angepasst werden
            };
        }
    }

    gpioWaveAddGeneric(pulseIndex, combinedPulses);
    int wave_id = gpioWaveCreate();
    if (wave_id >= 0) {
        gpioWaveTxSend(wave_id, PI_WAVE_MODE_ONE_SHOT);
        while (gpioWaveTxBusy()) usleep(10);
        gpioWaveDelete(wave_id);
    } else {
        fprintf(stderr, "Fehler beim Erzeugen der Waveform\n");
    }
}

void initialize_mqtt() {
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTClient_setCallbacks(client, NULL, NULL, on_message, NULL);

    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_connect(client, &conn_opts);

    MQTTClient_subscribe(client, TOPIC, QOS);
}

int on_message(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    char* payloadStr = malloc(message->payloadlen + 1);
    memcpy(payloadStr, message->payload, message->payloadlen);
    payloadStr[message->payloadlen] = '\0'; // Ensure null-terminated string

    // Ausgabe des empfangenen Payloads
    printf("Empfangene Nachricht auf Topic '%s': %s\n", topicName, payloadStr);

    // Verarbeite die empfangene Nachricht
    parse_and_execute_json_sequences(payloadStr);

    free(payloadStr);
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}


int main() {
    
    initialize_motors();
    initialize_mqtt();

    while (1) {
        usleep(100000);
    }
    
    MQTTClient_disconnect(client, 1000);
    MQTTClient_destroy(&client);
    gpioTerminate();
    return 0;
}