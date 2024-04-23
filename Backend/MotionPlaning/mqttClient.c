

#include "MQTTClient.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "global.h"


MQTTClient client;

// `destroyMqtt` trennt die Verbindung des MQTT-Clients und zerstört das Client-Objekt.
void destroyMqtt() {
    MQTTClient_disconnect(client, 1000);
    MQTTClient_destroy(&client);
}


// `messageArrived` wird aufgerufen, wenn eine Nachricht auf einem abonnierten Topic ankommt.
// Parameter:
//   - void *context: Kontext für Callback-Funktionen, hier zur Verarbeitung der Nachricht verwendet
//   - char *topicName: Name des Topics, auf dem die Nachricht empfangen wurde
//   - int topicLen: Länge des Topic-Namens
//   - MQTTClient_message *message: Struktur, die die empfangene Nachricht enthält
// Rückgabewert:
//   - int: Gibt immer 1 zurück, um den erfolgreichen Empfang der Nachricht zu bestätigen
int messageArrived(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    char* payloadStr = malloc(message->payloadlen + 1);
    memcpy(payloadStr, message->payload, message->payloadlen);
    payloadStr[message->payloadlen] = '\0'; // Sicherstellen, dass die Zeichenkette korrekt terminiert ist
    ((void(*)(char*, char*))context)(topicName, payloadStr); // Aufruf der Callback-Funktion mit Topic und Nachricht
    free(payloadStr); // Freigabe des Speichers für die Nachricht
    MQTTClient_freeMessage(&message); // Freigabe der Nachrichtenstruktur
    MQTTClient_free(topicName); // Freigabe des Topic-Namens
    return 1;
}

// `publishMessage` veröffentlicht eine Nachricht auf einem spezifischen MQTT-Topic.
// Parameter:
//   - const char* topic: Name des Topics, auf dem die Nachricht veröffentlicht werden soll
//   - const char* message: Nachricht, die veröffentlicht werden soll
void publishMessage(const char* topic, const char* message) {
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    pubmsg.payload = (void*)message;
    pubmsg.payloadlen = strlen(message);
    pubmsg.qos = QOS; // Qualität der Service-Einstellung aus global.h
    pubmsg.retained = 0; // Nachricht wird nicht im Broker gespeichert
    MQTTClient_deliveryToken token;
    int rc = MQTTClient_publishMessage(client, topic, &pubmsg, &token);  // Senden der Nachricht
    if (rc != MQTTCLIENT_SUCCESS) {
        printf("Failed to publish message, return code: %d\n", rc);
    }
}

// `initializeMqtt` initialisiert den MQTT-Client, setzt die Callbacks und subscribt zu definierten Topics.
// Parameter:
//   - const char* topics[]: Array von Topics, zu denen der Client subscriben wird
//   - int topicCount: Anzahl der Topics
//   - void(*onMessageCallback)(char*, char*): Callback-Funktion, die bei eingehenden Nachrichten aufgerufen wird
void initializeMqtt(const char* topics[], int topicCount, void(*onMessageCallback)(char*, char*)) {
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL); // Erstellung des MQTT-Client-Objekts
    MQTTClient_setCallbacks(client, onMessageCallback, NULL, messageArrived, NULL); // Setzen der Callback-Funktionen
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;  // Initialisierung der Verbindungsoptionen
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1; // Startet eine neue Session bei Verbindungsaufbau
    MQTTClient_connect(client, &conn_opts);
    for (int i = 0; i < topicCount; i++) {
        MQTTClient_subscribe(client, topics[i], QOS);  // Subscriben zu den angegebenen Topics
    }
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