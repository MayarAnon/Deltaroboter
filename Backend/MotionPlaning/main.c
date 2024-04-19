//compile code gcc -o program main.c -I/usr/local/include/cjson -L/usr/local/lib/cjson mqttClient.c  pathInterpolation.c -lpaho-mqtt3c inverseKinematic.c -lm -lcjson
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


#define ROBOTSTATETOPIC "robot/state"
#define LOADPROGRAMMTOPIC "pickandplace/program"
#define GRIPPERSTATETOPIC "GripperState"
#define GRIPPERFEEDBACKTOPIC "GripperFeedback"
#define MANUELCONTROLTOPIC "manual/control"

#define STEPSPERREVOLUTION 800
#define GEARRATIO 20

typedef struct {
    float deltaTheta1, deltaTheta2, deltaTheta3;
} AngleDeltas;

typedef struct {
    float theta1, theta2, theta3;
} Angles;
typedef struct {
    int Motor1, Motor2, Motor3;
} Steps;
// Globale Variable für die aktuelle Position
Coordinate currentPosition = {0.0, 0.0,-280.0}; //Starten mit Homekoordinaten 
Angles currentAngles = {-41.489,-41.489,-41.489}; //Starten mit Homewinkeln
int speed = 100;
int directionChangeTime = 5;

//Parsen Nachricht x y z herausbekommen 
//Prüfen ob Koordinaten innerhalb Arbeitsraum, falls Fehler => ausgeben
//Coordinaten von aktuellen Koorinaten abziehe falls größer 1 linear Interpolieren mit Steps entspricht größtem delta 
//Neue Winkel durch inverse Kinematik berechen, falls Fehler => ausgeben
//Neue Winkel von alten Winkeln abziehen
 //aus Differenz von winkeln und (deltatheta/360)*STEPSPERREVOLUTION*GEARRATIO Steps berechnen
//Steps an Motor publishen mit Speed als timing 
//currentPosition aktualisieren und publishen 
void onManualControl(char *payloadStr) {
    cJSON *root = cJSON_Parse(payloadStr);
    if (root == NULL) {
        fprintf(stderr, "Error before: %s\n", cJSON_GetErrorPtr());
        return;
    }

    Coordinate coord;
    coord.x = (float)cJSON_GetArrayItem(root, 0)->valuedouble;
    coord.y = (float)cJSON_GetArrayItem(root, 1)->valuedouble;
    coord.z = (float)cJSON_GetArrayItem(root, 2)->valuedouble;
    cJSON_Delete(root);

    float diffX = fabs(coord.x - currentPosition.x);
    float diffY = fabs(coord.y - currentPosition.y);
    float diffZ = fabs(coord.z - currentPosition.z);

    if (diffX > 1.0 || diffY > 1.0 || diffZ > 1.0) {
        float maxDiff = fmax(diffX, fmax(diffY, diffZ));
        int InterpolationSteps = (int)maxDiff;

        Coordinate* coordinates = linearInterpolation(currentPosition, coord, InterpolationSteps);
        Steps* steps = malloc(InterpolationSteps * sizeof(Steps));
        float lastTheta1 = currentAngles.theta1;
        float lastTheta2 = currentAngles.theta2;
        float lastTheta3 = currentAngles.theta3;

        cJSON* jsonRoot = cJSON_CreateArray();

        for (int i = 0; i < InterpolationSteps; i++) {
            float theta1, theta2, theta3;
            if (delta_calcInverse(coordinates[i].x, coordinates[i].y, coordinates[i].z, &theta1, &theta2, &theta3) == 0) {
                steps[i].Motor1 = ((theta1 - lastTheta1)/360) * STEPSPERREVOLUTION * GEARRATIO;
                steps[i].Motor2 = ((theta2 - lastTheta2)/360) * STEPSPERREVOLUTION * GEARRATIO;
                steps[i].Motor3 = ((theta3 - lastTheta3)/360) * STEPSPERREVOLUTION * GEARRATIO;

                if (steps[i].Motor1 != 0 || steps[i].Motor2 != 0 || steps[i].Motor3 != 0) { // Check if all deltas are zero
                    cJSON* stepObj = cJSON_CreateObject();
                    cJSON_AddItemToObject(stepObj, "motorpulses", cJSON_CreateIntArray((int[]){steps[i].Motor1, steps[i].Motor2, steps[i].Motor3}, 3));
                    cJSON_AddItemToObject(stepObj, "timing", cJSON_CreateIntArray((int[]){speed, speed, directionChangeTime}, 3));
                    cJSON_AddItemToArray(jsonRoot, stepObj);

                    printf("Interpolation Step %d: DeltaTheta1 = %d, DeltaTheta2 = %d, DeltaTheta3 = %d\n",
                           i, steps[i].Motor1, steps[i].Motor2, steps[i].Motor3);
                }
                // Update lastTheta values for next iteration
                lastTheta1 = theta1;
                lastTheta2 = theta2;
                lastTheta3 = theta3;
            } else {
                printf("Punkt existiert nicht");
            }
        }
        currentPosition = coord;
        delta_calcInverse(coord.x, coord.y, coord.z, &currentAngles.theta1, &currentAngles.theta2, &currentAngles.theta3);
        char* jsonString = cJSON_Print(jsonRoot);  // Generate final JSON string
        publishMessage("motors/sequence", jsonString);
        free(jsonString);
        cJSON_Delete(jsonRoot);

        free(coordinates);
        free(steps);
    }
}



void onMessage(char *topicName, char *payloadStr) {
    printf("Empfangene Nachricht auf Topic '%s': %s\n", topicName, payloadStr);
    if (strcmp(topicName, MANUELCONTROLTOPIC) == 0) {
        onManualControl(payloadStr);
    }
      

}


int main() {
    // Topics, zu denen wir subscriben möchten.
    const char* topics[] = {SPEEDTOPIC,LOADPROGRAMMTOPIC,GRIPPERSTATETOPIC,GRIPPERFEEDBACKTOPIC,MANUELCONTROLTOPIC};
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