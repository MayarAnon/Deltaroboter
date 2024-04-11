//compile code gcc -o program main.c -I/usr/local/include/cjson -L/usr/local/lib/cjson mqttClient.c pathInterpolation.c -lpaho-mqtt3c inverseKinematic.c -lm -lcjson
//execute ./program

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "mqttClient.h"
#include "pathInterpolation.h"
#include "inverseKinematic.h"
#include "cJSON.h"


#define SPEEDTOPIC "speedcontroll"
#define LOADPROGRAMMTOPIC "loadProgramm"
#define GRIPPERSTATETOPIC "GripperState"
#define GRIPPERFEEDBACKTOPIC "GripperFeedback"

void onMessage(char *topicName, char *payloadStr) {
    printf("Empfangene Nachricht auf Topic '%s': %s\n", topicName, payloadStr);
}




int main() {
    // Topics, zu denen wir subscriben möchten.
    const char* topics[] = {SPEEDTOPIC,LOADPROGRAMMTOPIC,GRIPPERSTATETOPIC,GRIPPERFEEDBACKTOPIC};
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