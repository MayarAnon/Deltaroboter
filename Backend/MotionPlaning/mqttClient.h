#ifndef MQTTCLIENT1_H
#define MQTTCLIENT1_H
#include "MQTTClient.h" // Füge diese Zeile hinzu

// Typdefinition für den Funktionszeiger
typedef void (*MessageCallback)(char *topicName, char *payloadStr);
extern MQTTClient client;
// Prototypen
void destroyMqtt();
void initializeMqtt(const char* topics[], int topicCount, MessageCallback onMessageCallback);
int messageArrived(void *context, char *topicName, int topicLen, MQTTClient_message *message);
void publishMessage(const char* topic, const char* message);

#endif