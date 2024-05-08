

#include "mqttClient.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "global.h"


MQTTAsync client;

// Die Funktion `onConnect` wird aufgerufen, wenn eine Verbindung erfolgreich hergestellt wurde.
// In dieser Funktion werden die Themen abonniert, auf die der Client hören soll.
// Parameter:
//   - void* context: Kontext, der bei der Initialisierung übergeben wurde
//   - MQTTAsync_successData* response: Enthält Informationen über die Verbindung
void onConnect(void* context, MQTTAsync_successData* response) {
    printf("Connected\n");
    // Subscribing to topics after successful connection
    for (int i = 0; i < globalTopicCount; i++) {
        MQTTAsync_subscribe(client, globalTopics[i], QOS, NULL);
    }
}

// Die Funktion `onConnectFailure` wird aufgerufen, wenn der Verbindungsversuch fehlschlägt.
// Sie gibt eine Fehlermeldung und den zugehörigen Code aus.
// Parameter:
//   - void* context: Kontext, der bei der Initialisierung übergeben wurde
//   - MQTTAsync_failureData* response: Enthält Fehlercode der fehlgeschlagenen Verbindung
void onConnectFailure(void* context, MQTTAsync_failureData* response) {
    fprintf(stderr, "Connect failed, rc %d\n", response ? response->code : 0);
}

// Die Funktion `connectionLost` wird aufgerufen, wenn die Verbindung unerwartet verloren geht.
// Sie druckt die Ursache des Verlusts und versucht, die Verbindung neu zu initialisieren.
// Parameter:
//   - void *context: Kontext, der bei der Initialisierung übergeben wurde
//   - char *cause: Beschreibung der Ursache für den Verlust der Verbindung
void connectionLost(void *context, char *cause) {
    fprintf(stderr, "Connection lost, cause: %s\n", cause);
    usleep(1000); // 1 Sekunden Wartezeit
    initializeMqtt(globalTopics, globalTopicCount, globalOnMessageCallback); // Versucht, die MQTT-Verbindung neu zu initialisieren
}

// Die Funktion `messageArrived` wird aufgerufen, wenn eine Nachricht für ein abonniertes Thema eintrifft.
// Sie verarbeitet die Nachricht und ruft die entsprechende Callback-Funktion auf.
// Parameter:
//   - void *context: Kontext, der bei der Initialisierung übergeben wurde
//   - char *topicName: Name des Themas, für das die Nachricht empfangen wurde
//   - int topicLen: Länge des Themas
//   - MQTTAsync_message *message: Enthält die Nachrichtendaten
// Rückgabewert:
//   - int: Statuscode (immer 1 für erfolgreiche Verarbeitung)
int messageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message) {
    char* payloadStr = strndup(message->payload, message->payloadlen);
    globalOnMessageCallback(topicName, payloadStr); // Ruft die globale Callback-Funktion auf
    free(payloadStr);

    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}

// Die Funktion `initializeMqtt` initialisiert den MQTT-Client, setzt Callbacks und startet den Verbindungsprozess.
// Parameter:
//   - const char* topics[]: Array von Themen, die abonniert werden sollen
//   - int topicCount: Anzahl der Themen im Array
//   - void(*onMessageCallback)(char*, char*): Callback-Funktion, die bei eingehenden Nachrichten aufgerufen wird
void initializeMqtt(const char* topics[], int topicCount, void(*onMessageCallback)(char*, char*)) {
    globalOnMessageCallback = onMessageCallback;  // Setzen der globalen Callback-Funktion
    MQTTAsync_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL); // Erstellt den MQTT-Client
    MQTTAsync_setCallbacks(client, NULL, connectionLost, messageArrived, NULL); // Setzt die Callback-Funktionen

    globalTopicCount = topicCount;  // Store topic count globally if needed
    globalTopics = topics;          // Store topics array globally if needed
    globalOnMessageCallback = onMessageCallback;

    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;  // Initialisiert die Verbindungsoptionen
    conn_opts.keepAliveInterval = 20; // Setzt das Keep-Alive-Intervall
    conn_opts.cleansession = 1; // Setzt die Session auf "clean"
    conn_opts.automaticReconnect = 1;  // Aktiviert automatische Wiederverbindung
    conn_opts.onSuccess = onConnect; // Setzt die Erfolgs-Callback-Funktion
    conn_opts.onFailure = onConnectFailure; // Setzt die Fehler-Callback-Funktion
    conn_opts.context = client; // Übergibt den Client als Kontext
    MQTTAsync_connect(client, &conn_opts); // Startet den Verbindungsprozess
}

// Die Funktion `publishMessage` sendet eine Nachricht an ein bestimmtes Thema.
// Parameter:
//   - const char* topic: Thema, an das die Nachricht gesendet wird
//   - const char* message: Nachricht, die gesendet wird
void publishMessage(const char* topic, const char* message) {
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer; // Initialisiert die Nachrichtenstruktur
    pubmsg.payload = (void*)message; // Setzt die Nachricht als Payload
    pubmsg.payloadlen = strlen(message); // Setzt die Länge der Nachricht
    pubmsg.qos = QOS; // Setzt die Qualität der Dienstleistung
    pubmsg.retained = 0; // Setzt das Retained-Flag

    MQTTAsync_sendMessage(client, topic, &pubmsg, NULL);
}

// Die Funktion `destroyMqtt` beendet die MQTT-Verbindung und räumt auf.
void destroyMqtt() {
    MQTTAsync_disconnect(client, NULL); // Trennt die Verbindung
    MQTTAsync_destroy(&client); // Zerstört den MQTT-Client
}