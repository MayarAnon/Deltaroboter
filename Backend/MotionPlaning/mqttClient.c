

#include "mqttClient.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "global.h"

// Anpassen der Definition, um den Typ aus der Header-Datei zu verwenden
const char* globalTopicsTemp[] = {ROBOTSTATETOPIC, LOADPROGRAMMTOPIC, MANUELCONTROLCOORDINATESTOPIC, MANUELCONTROLGRIPPERTOPIC, STOPTOPIC};
const char** globalTopics = globalTopicsTemp;  // Pointer auf das Array zuweisen
int globalTopicCount = sizeof(globalTopicsTemp) / sizeof(globalTopicsTemp[0]);
MessageCallback globalOnMessageCallback;

MQTTAsync client;

void onConnect(void* context, MQTTAsync_successData* response) {
    printf("Connected\n");
    // Subscribing to topics after successful connection
    for (int i = 0; i < globalTopicCount; i++) {
        MQTTAsync_subscribe(client, globalTopics[i], QOS, NULL);
    }
}

void onConnectFailure(void* context, MQTTAsync_failureData* response) {
    fprintf(stderr, "Connect failed, rc %d\n", response ? response->code : 0);
}

void connectionLost(void *context, char *cause) {
    fprintf(stderr, "Connection lost, cause: %s\n", cause);
    initializeMqtt(globalTopics, globalTopicCount, globalOnMessageCallback);
}

int messageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message) {
    char* payloadStr = strndup(message->payload, message->payloadlen);
    printf("Received message on topic '%s': %s\n", topicName, payloadStr);
    globalOnMessageCallback(topicName, payloadStr);
    free(payloadStr);

    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}

void initializeMqtt(const char* topics[], int topicCount, void(*onMessageCallback)(char*, char*)) {
    globalOnMessageCallback = onMessageCallback;  // Setzen der globalen Callback-Funktion
    MQTTAsync_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTAsync_setCallbacks(client, NULL, connectionLost, messageArrived, NULL);

    globalTopicCount = topicCount;  // Store topic count globally if needed
    globalTopics = topics;          // Store topics array globally if needed
    globalOnMessageCallback = onMessageCallback;

    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.onSuccess = onConnect;
    conn_opts.onFailure = onConnectFailure;
    conn_opts.context = client;
    MQTTAsync_connect(client, &conn_opts);
}

void publishMessage(const char* topic, const char* message) {
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    pubmsg.payload = (void*)message;
    pubmsg.payloadlen = strlen(message);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;

    MQTTAsync_sendMessage(client, topic, &pubmsg, NULL);
}

void destroyMqtt() {
    MQTTAsync_disconnect(client, NULL);
    MQTTAsync_destroy(&client);
}