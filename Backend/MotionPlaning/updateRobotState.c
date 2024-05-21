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

// Funktion zum Aktualisieren der Motorwerte und Ausgeben der neuen Werte
void UpdateStepError(const char *json_data) {
    // Parsen des JSON-Strings
    cJSON *json = cJSON_Parse(json_data);
    if (json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Fehler vor: %s\n", error_ptr);
        }
        return;
    }

    // Sicherstellen, dass die JSON-Objekte vorhanden sind und Werte subtrahieren
    cJSON *item = cJSON_GetObjectItem(json, "Motor 0");
    if (item != NULL) errorAccumulator1 -= item->valuedouble;

    item = cJSON_GetObjectItem(json, "Motor 1");
    if (item != NULL) errorAccumulator2 -= item->valuedouble;

    item = cJSON_GetObjectItem(json, "Motor 2");
    if (item != NULL) errorAccumulator3 -= item->valuedouble;
    /*
    item = cJSON_GetObjectItem(json, "Motor 3");
    if (item != NULL) errorAccumulator4 -= item->valuedouble;
    */
    // Aufräumen
    cJSON_Delete(json);
}

// Die Funktion `parseGripperMode` konvertiert einen String in einen entsprechenden Enum-Wert für Greifermodi.
// Parameter:
//   - const char* mode: Zeichenkette, die den Modus beschreibt (z.B. "parallelGripper")
// Rückgabewert:
//   - Gripper: Enum-Wert des Greifers, z.B. parallel, complient, magnet, vaccum. Bei unbekanntem Modus wird -1 zurückgegeben.
Gripper parseGripperMode(const char* mode) {
    if (strcmp(mode, "parallelGripper") == 0) return parallel;
    else if (strcmp(mode, "complientGripper") == 0) return complient;
    else if (strcmp(mode, "magnetGripper") == 0) return magnet;
    else if (strcmp(mode, "vacuumGripper") == 0) return vaccum;
    else {
        fprintf(stderr, "Unknown gripper mode: %s\n", mode);
        return -1; // Undefined behavior, could define an 'unknown' in the enum
    }
}

// Die Funktion `parseMotionProfile` konvertiert einen String in einen entsprechenden Enum-Wert für Bewegungsprofile.
// Parameter:
//   - const char* profile: Zeichenkette, die das Bewegungsprofil beschreibt (z.B. "RectangleProfil")
// Rückgabewert:
//   - MotionProfile: Enum-Wert des Bewegungsprofils, z.B. RectangleProfil, TrapezProfil. Bei unbekanntem Profil wird -1 zurückgegeben.
MotionProfile parseMotionProfile(const char* profile) {
    if (strcmp(profile, "RectangleProfil") == 0) return RectangleProfil;
    else if (strcmp(profile, "TrapezProfil") == 0) return TrapezProfil;
    else if (strcmp(profile, "SigmoidProfil") == 0) return SigmoidProfil;
    else {
        fprintf(stderr, "Unknown motion profile: %s\n", profile);
        return UnknownProfil; // Verwendung des neuen Enum-Wertes für unbekannte Profile
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

    cJSON *powerstageItem = cJSON_GetObjectItemCaseSensitive(json, "powerstage");
    if (powerstageItem != NULL && cJSON_IsBool(powerstageItem)) {
        bool powerstageValue = cJSON_IsTrue(powerstageItem);

        // Flankenerkennung
        if ((powerstageValue && currentPowerstageMode != On) || (!powerstageValue && currentPowerstageMode != Off)) {
            currentPowerstageMode = powerstageValue ? On : Off;
            printf("Powerstage mode set to %s.\n", currentPowerstageMode == On ? "ON" : "OFF");
            fflush(stdout);  // Sicherstellen, dass die Ausgabe sofort erfolgt
        }
    }


    // Parsing des Greifermodus und Konvertierung zu Enum
    char *gripperModeStr = cJSON_GetObjectItemCaseSensitive(json, "gripperMode")->valuestring;
    Gripper gripperModeValue = parseGripperMode(gripperModeStr);


     cJSON *motionProfileItem = cJSON_GetObjectItemCaseSensitive(json, "motionProfil");
    if (motionProfileItem == NULL || motionProfileItem->valuestring == NULL) {
        fprintf(stderr, "Missing or invalid motion profile\n");
    } else {
        MotionProfile motionProfileValue = parseMotionProfile(motionProfileItem->valuestring);
        if (motionProfileValue == UnknownProfil) {
            fprintf(stderr, "Invalid motion profile provided: %s\n", motionProfileItem->valuestring);
        }
        currentMotionProfil = motionProfileValue; // Setzen des Bewegungsprofils, unabhängig davon, ob es unbekannt ist
    }

    // Parsing eines Integer-Werts für die Geschwindigkeit der Motoren
    cJSON *motorsSpeed = cJSON_GetObjectItemCaseSensitive(json, "motorsSpeed");
    int motorsSpeedValue = motorsSpeed->valueint;


    // Anwendung des Homing-Werts, wenn wahr
    if(homingValue && homingFlag != homingValue ){
        printf("DeltaRoboter wird Referenziert! \n");
        robotRequiersHoming = false;
        fflush(stdout); 
        currentPosition = (Coordinate){0.0, 0.0, -280.0, 0.0};
        currentSteps = (Steps){0};
        errorAccumulator1 = 0.0;
        errorAccumulator2 = 0.0;
        errorAccumulator3 = 0.0;
        errorAccumulator4 = 0.0;

    }

    homingFlag = homingValue;
    speedSetting = motorsSpeedValue;   // Setzt die globale Geschwindigkeitseinstellung
    currentGripper = gripperModeValue; // Setzt den aktuellen Greifermodus
    
    cJSON_Delete(json); // Bereinigung des JSON-Objekts
}