

#include "MQTTClient.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define ADDRESS     "tcp://localhost:1883"
#define CLIENTID    "MotionControll"
#define QOS         0  // Set Quality of Service Level to 0 (At most once)
MQTTClient client;

void destroyMqtt() {
    MQTTClient_disconnect(client, 1000);
    MQTTClient_destroy(&client);
}

int messageArrived(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    char* payloadStr = malloc(message->payloadlen + 1);
    memcpy(payloadStr, message->payload, message->payloadlen);
    payloadStr[message->payloadlen] = '\0';
    ((void(*)(char*, char*))context)(topicName, payloadStr);
    free(payloadStr);
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

void publishMessage(const char* topic, const char* message) {
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    pubmsg.payload = (void*)message;
    pubmsg.payloadlen = strlen(message);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;
    MQTTClient_deliveryToken token;
    int rc = MQTTClient_publishMessage(client, topic, &pubmsg, &token);
    if (rc != MQTTCLIENT_SUCCESS) {
        printf("Failed to publish message, return code: %d\n", rc);
    }
}

void initializeMqtt(const char* topics[], int topicCount, void(*onMessageCallback)(char*, char*)) {
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTClient_setCallbacks(client, onMessageCallback, NULL, messageArrived, NULL);
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    MQTTClient_connect(client, &conn_opts);
    for (int i = 0; i < topicCount; i++) {
        MQTTClient_subscribe(client, topics[i], QOS);
    }
}



/*
void onMessage(char *topicName, char *payloadStr) {
    printf("Empfangene Nachricht auf Topic '%s': %s\n", topicName, payloadStr);
}



int main() {
    // Topics, zu denen wir subscriben möchten.
    const char* topics[] = {"Topic1", "Topic2"};
    int topicCount = sizeof(topics) / sizeof(topics[0]);

    
    // Initialisiert den MQTT-Client, subscribt zu den oben definierten Topics und setzt die Callback-Funktion.
    initializeMqtt(topics, topicCount, onMessage);

    // Veröffentlicht eine Nachricht auf "Topic1".
    publishMessage("Topic1", "Hello, MQTT World!");
    while (1) {
        usleep(100000);
    }
    destroyMqtt();

    return 0;
}
*/