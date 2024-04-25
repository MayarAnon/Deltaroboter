#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "cJSON.h"
#include "mqttClient.h"
#include <ctype.h>
#include <stdbool.h>
#include "global.h"

// Die Funktion `parseGripperMode` konvertiert einen String in einen entsprechenden Enum-Wert für Greifermodi.
// Parameter:
//   - const char* mode: Zeichenkette, die den Modus beschreibt (z.B. "parallelGripper")
// Rückgabewert:
//   - Gripper: Enum-Wert des Greifers, z.B. parallel, complient, magnet, vaccum. Bei unbekanntem Modus wird -1 zurückgegeben.
Gripper parseGripperMode(const char* mode) {
    if (strcmp(mode, "parallelGripper") == 0) return parallel;
    else if (strcmp(mode, "complientGripper") == 0) return complient;
    else if (strcmp(mode, "magnetGripper") == 0) return magnet;
    else if (strcmp(mode, "vakuumGripper") == 0) return vaccum;
    else {
        fprintf(stderr, "Unknown gripper mode: %s\n", mode);
        return -1; // Undefined behavior, could define an 'unknown' in the enum
    }
}


// Die Funktion `parseRobotState` parst den Zustand eines Roboters aus einem JSON-String.
// Parameter:
//   - const char *payloadStr: JSON-String, der den Zustand des Roboters beschreibt
// Diese Funktion setzt globale Variablen basierend auf den geparsten Daten.
void parseRobotState(const char *payloadStr) {
    cJSON *json = cJSON_Parse(payloadStr);
    if (json == NULL) {
        fprintf(stderr, "Error parsing JSON\n");
        return;
    }

    // Parsing eines Booleschen Werts für Homing-Status
    cJSON *homing = cJSON_GetObjectItemCaseSensitive(json, "homing");
    bool homingValue = cJSON_IsTrue(homing);

    // Parsing eines Arrays von Double-Werten für aktuelle Koordinaten
    cJSON *currentCoordinates = cJSON_GetObjectItemCaseSensitive(json, "currentCoordinates");
    double coordinates[3];  // Annahme: Es gibt immer drei Koordinaten
    if (cJSON_IsArray(currentCoordinates)) {
        for (int i = 0; i < 3; i++) {
            cJSON *coord = cJSON_GetArrayItem(currentCoordinates, i);
            coordinates[i] = coord->valuedouble;
        }
    }

    // Parsing eines Arrays von Double-Werten für aktuelle Winkel
    cJSON *currentAngles = cJSON_GetObjectItemCaseSensitive(json, "currentAngles");
    double angles[3];  // Annahme: Es gibt immer drei Winkel
    if (cJSON_IsArray(currentAngles)) {
        for (int i = 0; i < 3; i++) {
            cJSON *angle = cJSON_GetArrayItem(currentAngles, i);
            angles[i] = angle->valuedouble;
        }
    }

    // Parsing eines Booleschen Werts für Greifer-Feedback
    cJSON *gripperFeedback = cJSON_GetObjectItemCaseSensitive(json, "gripperFeedback");
    
    timeFlagGripper = cJSON_IsTrue(gripperFeedback); //wird global gepublisht
    
    // Parsing des Greifermodus und Konvertierung zu Enum
    char *gripperModeStr = cJSON_GetObjectItemCaseSensitive(json, "gripperMode")->valuestring;
    Gripper gripperModeValue = parseGripperMode(gripperModeStr);

    // Parsing eines Integer-Werts für die Geschwindigkeit der Motoren
    cJSON *motorsSpeed = cJSON_GetObjectItemCaseSensitive(json, "motorsSpeed");
    int motorsSpeedValue = motorsSpeed->valueint;

    // Parsing eines Arrays von Integer-Werten für den Arbeitsbereich des Roboters
    cJSON *robotWorkspace = cJSON_GetObjectItemCaseSensitive(json, "robotWorkspace");
    int workspace[2];  // Annahme: Es gibt immer zwei Elemente
    if (cJSON_IsArray(robotWorkspace)) {
        for (int i = 0; i < 2; i++) {
            cJSON *space = cJSON_GetArrayItem(robotWorkspace, i);
            workspace[i] = space->valueint;
        }
    }

    // Anwendung des Homing-Werts, wenn wahr
    if(homingValue){
        printf("Robot wird gehomed! \n");
        fflush(stdout); 
        currentPosition = (Coordinate){0.0, 0.0,-280.0,0.0};
        currentSteps = (Steps){0};
        errorAccumulator1 = 0;
        errorAccumulator2 = 0;
        errorAccumulator3 = 0;
        errorAccumulator4 = 0;

    }
    speedSetting = motorsSpeedValue;   // Setzt die globale Geschwindigkeitseinstellung
    currentGripper = gripperModeValue; // Setzt den aktuellen Greifermodus
    
    cJSON_Delete(json); // Bereinigung des JSON-Objekts
}