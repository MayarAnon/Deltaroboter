//  gcc -o MotionPlaning ./main.c  -I/usr/local/include/cjson -L/usr/local/lib/cjson mqttClient.c global.c gcodeParser.c manualMode.c updateRobotState.c pathInterpolation.c -lpaho-mqtt3c inverseKinematic.c calcMotion.c -lm -lcjson



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "pathInterpolation.h"
#include "inverseKinematic.h"
#include "cJSON.h"
#include "mqttClient.h"
#include <ctype.h>
#include <stdbool.h>
#include <pthread.h>
#include "global.h"
#include "gcodeParser.h"
#include "updateRobotState.h"
#include "manualMode.h"
#include <signal.h>




void readFile(const char* filename);
void processLine(const char* line);
void processInterpolationAndCreateJSON(Coordinate* coordinates, int InterpolationSteps, float f);
void removeNonPrintable(char *str);
void manualMode(char *payloadStr);
void parseRobotState(const char *payloadStr);
void publishCurrentState(Coordinate pos, Angles ang);





// Diese Funktion liest Dateien im Hintergrund aus. Sie wird als separater Thread gestartet,
// um die Hauptausführung des Programms nicht zu blockieren.
// Parameter:
//   - void* filename: Zeiger auf den Dateinamen als String. Der String wird im Thread freigegeben.
void* readFileThread(void* filename) {
    readFile((const char*)filename); // Ruft readFile mit dem übergebenen Dateinamen auf.
    free(filename);  // Geben Sie den duplizierten String frei
    return NULL;
}

// Diese Callback-Funktion verarbeitet eingehende MQTT-Nachrichten und leitet entsprechende Aktionen ein,
// abhängig vom Topic der Nachricht.
// Parameter:
//   - char *topicName: Name des Topics, auf dem die Nachricht empfangen wurde.
//   - char *payloadStr: Inhalt der Nachricht als String.
void onMessage(char *topicName, char *payloadStr) {
    pthread_t thread_id; // Thread-Identifikator für Hintergrundoperationen.

    if(strcmp(topicName, ERRORTOPIC) == 0){
        // Umwandeln des Payload-Strings in eine Zahl
        int payload = atoi(payloadStr);
        if(payload != 0){
            stopFlag = true;
            robotRequiersHoming = true;
            printf("ERROR: %s \n", payloadStr);
            printf("Homing Required!\n");
            fflush(stdout);
        } 
    }
    else if (strcmp(topicName, MANUELCONTROLCOORDINATESTOPIC) == 0 && !robotRequiersHoming) {
        manualModeCoordinates(payloadStr);
    }
    else if (strcmp(topicName, MANUELCONTROLGRIPPERTOPIC) == 0 && !robotRequiersHoming) {

        manualModeGripper(payloadStr);
    }
    else if (strcmp(topicName, STOPTOPIC) == 0) {
        if(strcmp("true", payloadStr) == 0){
            stopFlag = true; // Setzt das stopFlag, wenn die Nachricht "true" ist.
            printf("Stop Program! \n");
            fflush(stdout); 
        }
    }
    else if (strcmp(topicName, LOADPROGRAMMTOPIC) == 0 && !robotRequiersHoming) {
        
        stopFlag = false;
        char* safePayload = strdup(payloadStr);  // Dupliziere den String, um sicherzustellen, dass er nicht überschrieben wird
        printf("Lade Program: %s",payloadStr);
        fflush(stdout); // Sorgt dafür, dass "Hallo" sofort ausgegeben wird
        pthread_create(&thread_id, NULL, readFileThread, safePayload); // Startet einen Thread zum Dateilesen.
        pthread_detach(thread_id);  // Löst den Thread vom Hauptthread.
    }
    else if (strcmp(topicName, ROBOTSTATETOPIC) == 0) {
        parseRobotState(payloadStr);
    }
    else if (strcmp(topicName, PULSECHECKER) == 0) {
        printf("Update Pulse: %s",payloadStr);
        fflush(stdout); // Sorgt dafür, dass "Hallo" sofort ausgegeben wird
        UpdateStepError(payloadStr);
    }
    
}

// Funktion zum sicheren Beenden des Programms und Aufrufen der MQTT-Zerstörungsfunktion
void handle_signal(int sig) {
    printf("Signal erhalten (%d), beende MQTT und schließe Programm...\n", sig);
    destroyMqtt();
    exit(0);
}

// Hauptfunktion des Programms. Initialisiert den MQTT-Client, subscribt zu bestimmten Topics
// und tritt in eine Endlosschleife ein, um das Programm am Laufen zu halten.
int main() {
    fflush(stdout); // Sorgt dafür, dass "Hallo" sofort ausgegeben wird
    
    // Initialisiert den MQTT-Client, subscribt zu den oben definierten Topics und setzt die Callback-Funktion.
    initializeMqtt(globalTopics, globalTopicCount, onMessage);

    // Initialisiere Signalhandler
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    // Veröffentlicht eine Nachricht auf "Topic1".
    while (1) {
        usleep(1000); // Verzögert die Schleife, um CPU-Ressourcen zu sparen.
    } 
    destroyMqtt();
    return 0;
}

