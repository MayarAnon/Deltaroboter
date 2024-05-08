//gcc -o PulseChecker ./main.c  -I/usr/local/include/cjson -L/usr/local/lib/cjson  -lpaho-mqtt3as -lm -lcjson



#include "MQTTAsync.h"
#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ADDRESS "tcp://localhost:1883"
#define CLIENTID "MotionPlaning_Sequence"
#define TOPIC "motors/sequence"
#define QOS 1

MQTTAsync client;
int motorCounter[4] = {0, 0, 0, 0};  // Zähler für jeden Motor
int lastMotorPulses[4] = {0, 0, 0, 0};  // Letzter Zustand der Motorpulse

void onConnect(void* context, MQTTAsync_successData* response) {
    printf("Connected\n");
    MQTTAsync_subscribe(client, TOPIC, QOS, NULL);
}

void onConnectFailure(void* context, MQTTAsync_failureData* response) {
    fprintf(stderr, "Connect failed, rc %d\n", response ? response->code : 0);
}

void connectionLost(void *context, char *cause) {
    fprintf(stderr, "Connection lost, cause: %s\n", cause);
}

int messageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message) {
    char* payloadStr = strndup(message->payload, message->payloadlen);
    cJSON *json = cJSON_Parse(payloadStr);
    cJSON *sequence = NULL;
    cJSON_ArrayForEach(sequence, json) {
        cJSON *motorpulses = cJSON_GetObjectItemCaseSensitive(sequence, "motorpulses");
        if (motorpulses && cJSON_IsArray(motorpulses)) {
            for (int i = 0; i < cJSON_GetArraySize(motorpulses); i++) {
                int pulse = cJSON_GetArrayItem(motorpulses, i)->valueint;
                motorCounter[i] += pulse;
                lastMotorPulses[i] = pulse;  // Aktualisieren des letzten Pulses
            }
        }
    }
    cJSON_Delete(json);
    free(payloadStr);

    // Ausgabe der Zähler für jeden Motor in einer einzigen Zeile
    printf("Motors Counters | Motor 0: %d, Motor 1: %d, Motor 2: %d, Motor 3: %d\n",
           motorCounter[0], motorCounter[1], motorCounter[2], motorCounter[3]);

    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}

void initializeMqtt() {
    MQTTAsync_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTAsync_setCallbacks(client, NULL, connectionLost, messageArrived, NULL);

    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.onSuccess = onConnect;
    conn_opts.onFailure = onConnectFailure;
    MQTTAsync_connect(client, &conn_opts);
}

int main() {
    initializeMqtt();
    while (1) {
        usleep(100000); // Verzögert die Schleife, um CPU-Ressourcen zu sparen.
    }
    MQTTAsync_destroy(&client);
    return 0;
}

