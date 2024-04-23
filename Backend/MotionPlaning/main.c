//  gcc -o program ./main.c  -I/usr/local/include/cjson -L/usr/local/lib/cjson mqttClient.c global.c gcodeParser.c manualMode.c updateRobotState.c pathInterpolation.c -lpaho-mqtt3c inverseKinematic.c -lm -lcjson



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




void readFile(const char* filename);
void processLine(const char* line);
void processInterpolationAndCreateJSON(Coordinate* coordinates, int InterpolationSteps, float f);
void removeNonPrintable(char *str);
void manualMode(char *payloadStr);
void parseRobotState(const char *payloadStr);
void publishCurrentState(Coordinate pos, Angles ang);






void* readFileThread(void* filename) {
    readFile((const char*)filename);
    free(filename);  // Geben Sie den duplizierten String frei
    return NULL;
}


void onMessage(char *topicName, char *payloadStr) {
    pthread_t thread_id;
    
    if (strcmp(topicName, MANUELCONTROLTOPIC) == 0) {
        manualMode(payloadStr);
    }
    else if (strcmp(topicName, STOPTOPIC) == 0) {
        if(strcmp("true", payloadStr) == 0){
            stopFlag = true;
        }
    }
    else if (strcmp(topicName, LOADPROGRAMMTOPIC) == 0) {
        stopFlag = false;
        char* safePayload = strdup(payloadStr);  // Dupliziere den String, um sicherzustellen, dass er nicht überschrieben wird
        pthread_create(&thread_id, NULL, readFileThread, safePayload);
        pthread_detach(thread_id);  // Optional: Detach the thread
    }
    else if (strcmp(topicName, ROBOTSTATETOPIC) == 0) {
        parseRobotState(payloadStr);
    }
}



int main() {
    // Topics, zu denen wir subscriben möchten.
    const char* topics[] = {MANUELCONTROLTOPIC, LOADPROGRAMMTOPIC,ROBOTSTATETOPIC,STOPTOPIC};
    int topicCount = sizeof(topics) / sizeof(topics[0]);

    
    // Initialisiert den MQTT-Client, subscribt zu den oben definierten Topics und setzt die Callback-Funktion.
    initializeMqtt(topics, topicCount, onMessage);

    // Veröffentlicht eine Nachricht auf "Topic1".
    while (1) {
        usleep(100000);
    }
    destroyMqtt();
    return 0;
}

void readFile(const char* filename) {
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    // Pfad zusammenbauen: Gehe einen Ordner hoch und dann in den Ordner GCodeFiles
    char path[1024];  // Pfadgröße anpassen, falls nötig
    snprintf(path, sizeof(path), "../GCodeFiles/%s", filename);
    
    FILE* file = fopen(path, "r");
    if (!file) {
        
        perror(path);
        return;
    }

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



