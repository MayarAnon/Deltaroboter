

#include "MQTTClient.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // Für usleep-Funktionen

// MQTT-Definitions
#define ADDRESS     "tcp://localhost:1883" // Die Adresse des MQTT-Brokers.
#define CLIENTID    "MotionControll"       // Die Client-ID für die MQTT-Verbindung.
#define QOS         1                      // Das Quality of Service Level für die Nachrichtenübertragung.
MQTTClient client;                        // Eine Instanz eines MQTTClient.

typedef void (*MessageCallback)(char *topicName, char *payloadStr);


// Funktion: destroyMqtt
// Zweck: Trennt die Verbindung zum MQTT-Broker und zerstört den MQTT-Client.
void destroyMqtt(){
    MQTTClient_disconnect(client, 1000);
    MQTTClient_destroy(&client);
}


// Funktion: messageArrived
// Parameter:
// - context: Ein Zeiger auf den Benutzerdefinierten Kontext (hier der Funktionszeiger auf die Callback-Funktion).
// - topicName: Der Name des Topics, für das die Nachricht empfangen wurde.
// - topicLen: Die Länge des Topic-Namens.
// - message: Ein Zeiger auf die empfangene MQTT-Nachricht.
// Rückgabewert: Ein Integer-Wert (1), der signalisiert, dass die Nachricht erfolgreich verarbeitet wurde.
// Zweck: Wird automatisch aufgerufen, wenn eine Nachricht für ein abonniertes Topic empfangen wird. Ruft die benutzerdefinierte Callback-Funktion auf.
int messageArrived(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    char* payloadStr = malloc(message->payloadlen + 1);
    memcpy(payloadStr, message->payload, message->payloadlen);
    payloadStr[message->payloadlen] = '\0';

    // Rufen Sie hier Ihre angepasste onMessage Funktion auf
    ((MessageCallback)context)(topicName, payloadStr);

    free(payloadStr);
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

// Funktion: publishMessage
// Parameter:
// - topic: Der Name des Topics, unter dem die Nachricht veröffentlicht werden soll.
// - message: Die zu veröffentlichende Nachricht als String.
// Zweck: Veröffentlicht eine Nachricht unter einem bestimmten Topic beim MQTT-Broker.
void publishMessage(const char* topic, const char* message) {
    // Initialisiert die Struktur für die zu sendende Nachricht.
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    pubmsg.payload = (void*)message;
    pubmsg.payloadlen = strlen(message);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;

    // Veröffentlicht die Nachricht auf dem angegebenen Topic.
    MQTTClient_deliveryToken token;
    MQTTClient_publishMessage(client, topic, &pubmsg, &token);
    // Wartet auf die Bestätigung, dass die Nachricht veröffentlicht wurde.
    MQTTClient_waitForCompletion(client, token, 1000L);
    printf("Nachricht '%s' wurde zu Topic '%s' gesendet\n", message, topic);
}

// Funktion: initializeMqtt
// Parameter:
// - topics[]: Ein Array von Topic-Strings, zu denen der Client subscriben soll.
// - topicCount: Die Anzahl der Topics im `topics[]` Array.
// - onMessageCallback: Ein Funktionszeiger auf die Callback-Funktion, die bei eingehenden Nachrichten aufgerufen wird.
// Zweck: Initialisiert den MQTT-Client, verbindet sich mit dem Broker und subscribt zu den gewünschten Topics.
void initializeMqtt(const char* topics[], int topicCount, MessageCallback onMessageCallback) {
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);

    // Registriert die Wrapper-Funktion als Callback und übergibt onMessageCallback als Kontext
    MQTTClient_setCallbacks(client, (void*)onMessageCallback, NULL, messageArrived, NULL);

    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
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