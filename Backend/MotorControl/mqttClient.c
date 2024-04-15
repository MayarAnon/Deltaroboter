#include "mqttClient.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "MotorControl.h"


MQTTAsync client;

void initialize_mqtt() {
    MQTTAsync_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTAsync_setCallbacks(client, NULL, connectionLost, onMessage, NULL);

    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.automaticReconnect = 1;
    conn_opts.onSuccess = onConnect;
    conn_opts.onFailure = onConnectFailure;
    conn_opts.context = client;
    MQTTAsync_connect(client, &conn_opts);
}

void onConnect(void* context, MQTTAsync_successData* response) {
    printf("Connected\n");
    MQTTAsync_subscribe(client, TOPIC, QOS, NULL);
    MQTTAsync_subscribe(client, STOP_TOPIC, QOS, NULL);
}

void onConnectFailure(void* context, MQTTAsync_failureData* response) {
    fprintf(stderr, "Connect failed, rc %d\n", response ? response->code : 0);
}

int onMessage(void *context, char *topicName, int topicLen, MQTTAsync_message *message) {
    char* payloadStr = strndup(message->payload, message->payloadlen);

    if (strcmp(topicName, STOP_TOPIC) == 0 && strcmp(payloadStr, "true") == 0) {
        // Notstop-Nachricht empfangen
        fprintf(stderr, "Emergency stop triggered!\n");
        emergency_stop_triggered = 1;
        free(payloadStr);
        trigger_emergency_stop(); // Führe Notstop aus ohne einen neuen Thread zu starten
    } else {
        // Starte einen neuen Thread für die Verarbeitung anderer Nachrichten
        pthread_t thread;
        if (pthread_create(&thread, NULL, message_processing_thread, payloadStr) != 0) {
            fprintf(stderr, "Failed to create thread\n");
            free(payloadStr);  // Freigabe von Speicher, falls Thread-Erstellung fehlschlägt
        } else {
            pthread_detach(thread);  // Detach the thread to prevent memory leaks
        }
    }

    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}

void connectionLost(void *context, char *cause) {
    fprintf(stderr, "Connection lost, cause: %s\n", cause);
    initialize_mqtt();
}

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

        int pulses[MOTOR_COUNT];
        for (int i = 0; i < cJSON_GetArraySize(motors); i++) {
            pulses[i] = cJSON_GetArrayItem(motors, i)->valueint;
        }

        int sequencePulseWidth = (timing && cJSON_GetArraySize(timing) > 0) ? cJSON_GetArrayItem(timing, 0)->valueint : PULSE_WIDTH_DEFAULT;
        int sequencePauseBetweenPulses = (timing && cJSON_GetArraySize(timing) > 1) ? cJSON_GetArrayItem(timing, 1)->valueint : PAUSE_BETWEEN_PULSES_DEFAULT;
        int sequenceDirectionChangeDelay = (timing && cJSON_GetArraySize(timing) > 2) ? cJSON_GetArrayItem(timing, 2)->valueint : DIRECTION_CHANGE_DELAY_DEFAULT;

        execute_interpolated_sequence(pulses, sequencePulseWidth, sequencePauseBetweenPulses, sequenceDirectionChangeDelay);
    }

    cJSON_Delete(json);
}