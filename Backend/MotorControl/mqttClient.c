#include "mqttClient.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "MotorControl.h"
#include "config.h"
#include "queue.h"
Config globalConfig;
Queue messageQueue;
MQTTAsync client;
void initialize_mqtt() {
     // MQTT-Client erstellen und Callbacks setzen
    MQTTAsync_create(&client, globalConfig.address, globalConfig.clientId, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTAsync_setCallbacks(client, NULL, connectionLost, onMessage, NULL);
     // Verbindungsoptionen konfigurieren
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.automaticReconnect = 1;
    conn_opts.onSuccess = onConnect;
    conn_opts.onFailure = onConnectFailure;
    conn_opts.context = client;
    MQTTAsync_connect(client, &conn_opts);
}
// Callback-Funktion für erfolgreiche Verbindung
void onConnect(void* context, MQTTAsync_successData* response) {
    printf("Connected\n");
     // Abonnieren der MQTT-Topics
    MQTTAsync_subscribe(client, globalConfig.topic, globalConfig.qos, NULL);
    MQTTAsync_subscribe(client, globalConfig.stopTopic, globalConfig.qos, NULL);
}
// Callback-Funktion für Verbindungsfehler
void onConnectFailure(void* context, MQTTAsync_failureData* response) {
    fprintf(stderr, "Connect failed, rc %d\n", response ? response->code : 0);
}
// Callback-Funktion für den Empfang von MQTT-Nachrichten
int onMessage(void *context, char *topicName, int topicLen, MQTTAsync_message *message) {
    // Nachrichtenverarbeitung
    char* payloadStr = strndup(message->payload, message->payloadlen);
    // printf("Empfangene Nachricht auf Topic '%s': %s\n", topicName, payloadStr);
    if (strcmp(topicName, globalConfig.stopTopic) == 0 && strcmp(payloadStr, "true") == 0) {
        fprintf(stderr, "Emergency stop triggered!\n");
        emergency_stop_triggered = 1;
        free(payloadStr);
        trigger_emergency_stop();
    } else {
        enqueue(&messageQueue, payloadStr);
    }
    // Freigabe von MQTT-Nachricht und Topic
    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}
// Callback-Funktion für Verbindungsverlust
void connectionLost(void *context, char *cause) {
    fprintf(stderr, "Connection lost, cause: %s\n", cause);
    initialize_mqtt();
}
// Funktion zum Parsen und Ausführen von JSON-Sequenzen
void parse_and_execute_json_sequences(const char *jsonString) {
    cJSON *json = cJSON_Parse(jsonString);
    if (!json) {
        fprintf(stderr, "Error before: %s\n", cJSON_GetErrorPtr());
        return;
    }

    cJSON *sequence = NULL;
    cJSON_ArrayForEach(sequence, json) {
        cJSON *motors = cJSON_GetObjectItemCaseSensitive(sequence, "motorpulses");
        cJSON *timing = cJSON_GetObjectItemCaseSensitive(sequence, "timing");
        // Motorpulsationen und Timing aus JSON extrahieren
        int *pulses = malloc(sizeof(int) * globalConfig.motor_count);
        for (int i = 0; i < cJSON_GetArraySize(motors); i++) {
            pulses[i] = cJSON_GetArrayItem(motors, i)->valueint;
        }
        // Timing-Parameter setzen (falls vorhanden, ansonsten Standardwerte verwenden)
        int sequencePulseWidth = (timing && cJSON_GetArraySize(timing) > 0) ? cJSON_GetArrayItem(timing, 0)->valueint : globalConfig.pulseWidth;
        int sequencePauseBetweenPulses = (timing && cJSON_GetArraySize(timing) > 1) ? cJSON_GetArrayItem(timing, 1)->valueint : globalConfig.pauseBetweenPulses;
        int sequenceDirectionChangeDelay = (timing && cJSON_GetArraySize(timing) > 2) ? cJSON_GetArrayItem(timing, 2)->valueint : globalConfig.directionChangeDelay;
        // Sequenz ausführen (MotorControl.h)
        execute_interpolated_sequence(pulses, sequencePulseWidth, sequencePauseBetweenPulses, sequenceDirectionChangeDelay);
        free(pulses);
    }
    
    cJSON_Delete(json);
}