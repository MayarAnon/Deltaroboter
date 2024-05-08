//  gcc -o MotionPlaning ./main.c  -I/usr/local/include/cjson -L/usr/local/lib/cjson mqttClient.c global.c gcodeParser.c manualMode.c updateRobotState.c pathInterpolation.c -lpaho-mqtt3as inverseKinematic.c -lm -lcjson



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
    
    if (strcmp(topicName, MANUELCONTROLCOORDINATESTOPIC) == 0) {
        manualModeCoordinates(payloadStr);
    }
    else if (strcmp(topicName, MANUELCONTROLGRIPPERTOPIC) == 0) {

        manualModeGripper(payloadStr);
    }
    else if (strcmp(topicName, STOPTOPIC) == 0) {
        if(strcmp("true", payloadStr) == 0){
            printf("Stop Program \n");
            fflush(stdout); 
            stopFlag = true; // Setzt das stopFlag, wenn die Nachricht "true" ist.
        }
    }
    else if (strcmp(topicName, LOADPROGRAMMTOPIC) == 0) {
        
        stopFlag = false;
        char* safePayload = strdup(payloadStr);  // Dupliziere den String, um sicherzustellen, dass er nicht überschrieben wird
        printf("Lade Program: %s",payloadStr);
        fflush(stdout); // Sorgt dafür, dass "Hallo" sofort ausgegeben wird
        pthread_create(&thread_id, NULL, readFileThread, safePayload); // Startet einen Thread zum Dateilesen.
        pthread_detach(thread_id);  // Löst den Thread vom Hauptthread.
    }
    else if (strcmp(topicName, ROBOTSTATETOPIC) == 0) {
        
        fflush(stdout);
        parseRobotState(payloadStr);

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
    
    printf("MotionPlaning online\n");
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

// Liest eine Datei und verarbeitet jede Zeile durch Aufruf der Funktion processLine.
// Parameter:
//   - const char* filename: Pfad der Datei, die gelesen werden soll.
void readFile(const char* filename) {
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    // Pfad zusammenbauen: Gehe einen Ordner hoch und dann in den Ordner GCodeFiles
    char path[1024];  // Pfadgröße anpassen, falls nötig
    snprintf(path, sizeof(path), "../GCodeFiles/%s", filename); // Baut den vollständigen Pfad zur Datei.
    
    FILE* file = fopen(path, "r");
    if (!file) {
        
        perror(path);
        return;
    }
    printf("Programm wird ausgeführt. \n");
    while ((read = getline(&line, &len, file)) != -1) {
        if (stopFlag) {
            printf("load Program wurde Abgebrochen");
            break;
        }
        
        processLine(line);
    }

    free(line);  // Wichtig, um den von getline zugewiesenen Speicher freizugeben
    fclose(file);
}



