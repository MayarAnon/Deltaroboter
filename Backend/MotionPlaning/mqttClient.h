#ifndef MQTTCLIENT1_H
#define MQTTCLIENT1_H
#include "MQTTClient.h" // Füge diese Zeile hinzu

// Typdefinition für den Funktionszeiger
typedef void (*MessageCallback)(char *topicName, char *payloadStr);

//Client global verfügbar machen 
extern MQTTClient client;
// Prototypen

// `destroyMqtt` trennt die Verbindung des MQTT-Clients und zerstört das Client-Objekt.
void destroyMqtt();

// `initializeMqtt` initialisiert den MQTT-Client, setzt die Callbacks und subscribt zu definierten Topics.
// Parameter:
//   - const char* topics[]: Array von Topics, zu denen der Client subscriben wird
//   - int topicCount: Anzahl der Topics
//   - void(*onMessageCallback)(char*, char*): Callback-Funktion, die bei eingehenden Nachrichten aufgerufen wird
void initializeMqtt(const char* topics[], int topicCount, MessageCallback onMessageCallback);

// `messageArrived` wird aufgerufen, wenn eine Nachricht auf einem abonnierten Topic ankommt.
// Parameter:
//   - void *context: Kontext für Callback-Funktionen, hier zur Verarbeitung der Nachricht verwendet
//   - char *topicName: Name des Topics, auf dem die Nachricht empfangen wurde
//   - int topicLen: Länge des Topic-Namens
//   - MQTTClient_message *message: Struktur, die die empfangene Nachricht enthält
// Rückgabewert:
//   - int: Gibt immer 1 zurück, um den erfolgreichen Empfang der Nachricht zu bestätigen
int messageArrived(void *context, char *topicName, int topicLen, MQTTClient_message *message);

// `publishMessage` veröffentlicht eine Nachricht auf einem spezifischen MQTT-Topic.
// Parameter:
//   - const char* topic: Name des Topics, auf dem die Nachricht veröffentlicht werden soll
//   - const char* message: Nachricht, die veröffentlicht werden soll
void publishMessage(const char* topic, const char* message);

#endif