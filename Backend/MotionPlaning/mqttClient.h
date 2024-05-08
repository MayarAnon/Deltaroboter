#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#include "MQTTAsync.h"

// Typdefinition für den Funktionszeiger für die Nachrichten-Callback-Funktion
typedef void (*MessageCallback)(char *topicName, char *payloadStr);

// Externe Variablen, die die Topics, Topic-Count und die Callback-Funktion speichern



// Funktionen, die vom MQTT-Client verwendet werden
void initializeMqtt(const char* topics[], int topicCount, MessageCallback onMessageCallback);
void publishMessage(const char* topic, const char* message);
void destroyMqtt();

#endif
