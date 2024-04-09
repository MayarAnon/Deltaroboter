// gcc -o interpolation interpolation.c -I/usr/local/include/cjson -L/usr/local/lib/cjson -lpigpio -lrt -pthread -lpaho-mqtt3c -lcjson


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
const int pulseWidthUs = 500; // Pulsbreite in Mikrosekunden
const int pauseBetweenPulsesUs = 500; // Pause zwischen Pulsen in Mikrosekunden

MQTTClient client;


// Prototypen
void initialize_motors();
void execute_interpolated_sequence(int pulses[MOTOR_COUNT]);
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
        if (cJSON_IsArray(motors)) {
            int pulses[MOTOR_COUNT];
            for (int i = 0; i < cJSON_GetArraySize(motors); i++) {
                cJSON *pulse = cJSON_GetArrayItem(motors, i);
                pulses[i] = pulse->valueint;
            }
            execute_interpolated_sequence(pulses);
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

void execute_interpolated_sequence(int pulses[MOTOR_COUNT]) {
    gpioWaveClear();

    int maxPulses = 0;
    for (int i = 0; i < MOTOR_COUNT; ++i) {
        maxPulses = (abs(pulses[i]) > maxPulses) ? abs(pulses[i]) : maxPulses;
    }

    // Initialisiere das Pulse-Array mit einer Schätzung für die maximale Größe
    gpioPulse_t combinedPulses[(maxPulses + 2) * MOTOR_COUNT * 2];
    int pulseIndex = 0;

    // Füge Richtungsänderungen zu Beginn der Sequenz hinzu, falls notwendig
    for (int i = 0; i < MOTOR_COUNT; ++i) {
        if (pulses[i] < 0) { // Nur wenn die Pulse negativ sind
            combinedPulses[pulseIndex++] = (gpioPulse_t){
                .gpioOn = (1 << dir_gpios[i]),
                .gpioOff = 0,
                .usDelay = 1
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

    // Füge Richtungsänderungen am Ende der Sequenz hinzu, falls notwendig
    for (int i = 0; i < MOTOR_COUNT; ++i) {
        if (pulses[i] < 0) { // Setze den dir_GPIO zurück auf LOW nach negativen Pulsen
            combinedPulses[pulseIndex++] = (gpioPulse_t){
                .gpioOn = 0,
                .gpioOff = (1 << dir_gpios[i]),
                .usDelay = 1
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

// void execute_interpolated_sequence(int pulses[MOTOR_COUNT]) {
//     gpioWaveClear();

//     int maxPulses = 0;
//     for (int i = 0; i < MOTOR_COUNT; ++i) {
//         maxPulses = (abs(pulses[i]) > maxPulses) ? abs(pulses[i]) : maxPulses;
//         gpioWrite(dir_gpios[i], pulses[i] >= 0 ? 1 : 0); // Setze Richtung vor der Pulse-Generierung
//     }

//     gpioPulse_t combinedPulses[maxPulses * MOTOR_COUNT * 2];
//     int pulseIndex = 0;

//     for (int pulseNum = 0; pulseNum < maxPulses; ++pulseNum) {
//         unsigned int bitsOn = 0, bitsOff = 0;
//         for (int motorIndex = 0; motorIndex < MOTOR_COUNT; ++motorIndex) {
//             if (pulseNum < abs(pulses[motorIndex])) {
//                 bitsOn |= (1 << motor_gpios[motorIndex]);
//             } else {
//                 bitsOff |= (1 << motor_gpios[motorIndex]);
//             }
//         }
//         combinedPulses[pulseIndex++] = (gpioPulse_t){.gpioOn = bitsOn, .gpioOff = bitsOff, .usDelay = pulseWidthUs};
//         // Für eine kontinuierliche Bewegung ohne Unterbrechung kann die nächste Zeile angepasst oder weggelassen werden.
//         combinedPulses[pulseIndex++] = (gpioPulse_t){.gpioOn = 0, .gpioOff = bitsOn, .usDelay = pauseBetweenPulsesUs};
//     }

//     gpioWaveAddGeneric(pulseIndex, combinedPulses);
//     int wave_id = gpioWaveCreate();
//     if (wave_id >= 0) {
//         gpioWaveTxSend(wave_id, PI_WAVE_MODE_ONE_SHOT);
//         while (gpioWaveTxBusy()) usleep(10);
//         gpioWaveDelete(wave_id);
//     } else {
//         fprintf(stderr, "Fehler beim Erzeugen der Waveform\n");
//     }
// }


// void execute_sequences(const int sequence[][MOTOR_COUNT], int sequenceCount) {
//     for (int seqIndex = 0; seqIndex < sequenceCount; ++seqIndex) {
//         gpioWaveClear();

//         gpioPulse_t pulses[SEQUENCE_COUNT * MOTOR_COUNT * 2];
//         int pulseIndex = 0;

//         for (int motorIndex = 0; motorIndex < MOTOR_COUNT; ++motorIndex) {
//             int pulsesForMotor = abs(sequence[seqIndex][motorIndex]);
//             // Setze die Richtung basierend auf dem Vorzeichen des Sequenzwertes
//             gpioWrite(dir_gpios[motorIndex], sequence[seqIndex][motorIndex] >= 0 ? 1 : 0);

//             for (int pulseNum = 0; pulseNum < pulsesForMotor; ++pulseNum) {
//                 // Puls an
//                 pulses[pulseIndex++] = (gpioPulse_t){.gpioOn = 1 << motor_gpios[motorIndex], .gpioOff = 0, .usDelay = pulseWidthUs};
//                 // Puls aus
//                 pulses[pulseIndex++] = (gpioPulse_t){.gpioOn = 0, .gpioOff = 1 << motor_gpios[motorIndex], .usDelay = pauseBetweenPulsesUs};
//             }
//         }

//         // Füge Pulse hinzu und sende die Waveform
//         if(pulseIndex > 0) { // Überprüfe, ob Pulse vorhanden sind
//             gpioWaveAddGeneric(pulseIndex, pulses);
//             int wave_id = gpioWaveCreate();
//             if (wave_id >= 0) {
//                 gpioWaveTxSend(wave_id, PI_WAVE_MODE_ONE_SHOT);
//                 while (gpioWaveTxBusy()) usleep(10);
//                 gpioWaveDelete(wave_id);
//             } else {
//                 fprintf(stderr, "Fehler beim Erzeugen der Waveform\n");
//             }
//         }
//     }
// }
// void execute_sequences(const int sequence[][MOTOR_COUNT], int sequenceCount) {
//     for (int seqIndex = 0; seqIndex < sequenceCount; ++seqIndex) {
//         gpioWaveClear();

//         int maxPulses = 0;
//         for (int i = 0; i < MOTOR_COUNT; ++i) {
//             if (sequence[seqIndex][i] > maxPulses) {
//                 maxPulses = sequence[seqIndex][i];
//             }
//         }

//         gpioPulse_t pulses[maxPulses * MOTOR_COUNT * 2];
//         int pulseIndex = 0;

//         for (int pulseNum = 0; pulseNum < maxPulses; ++pulseNum) {
//             unsigned int bitsOn = 0, bitsOff = 0;
//             for (int motorIndex = 0; motorIndex < MOTOR_COUNT; ++motorIndex) {
//                 if (pulseNum < sequence[seqIndex][motorIndex]) {
//                     bitsOn |= (1 << motor_gpios[motorIndex]);
//                 }
//             }
//             // Puls an
//             pulses[pulseIndex++] = (gpioPulse_t){.gpioOn = bitsOn, .gpioOff = bitsOff, .usDelay = pulseWidthUs};
//             // Puls aus (alle Motoren ausschalten)
//             pulses[pulseIndex++] = (gpioPulse_t){.gpioOn = 0, .gpioOff = bitsOn, .usDelay = pauseBetweenPulsesUs};
//         }

//         gpioWaveAddGeneric(pulseIndex, pulses);
//         int wave_id = gpioWaveCreate();
//         if (wave_id >= 0) {
//             gpioWaveTxSend(wave_id, PI_WAVE_MODE_ONE_SHOT);
//             while (gpioWaveTxBusy()) usleep(10);
//             gpioWaveDelete(wave_id);
//         } else {
//             fprintf(stderr, "Fehler beim Erzeugen der Waveform\n");
//         }
//     }
// }
