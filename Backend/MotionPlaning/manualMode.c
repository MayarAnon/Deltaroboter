#include "cJSON.h"
#include "global.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "gcodeParser.h"
#include "mqttClient.h"


// Verarbeitet die Koordinaten aus einem JSON-String und führt gegebenenfalls eine Bewegung aus.
// Parameter:
//   - char *payloadStr: JSON-kodierter String, der Koordinaten enthält.
void manualModeCoordinates(char *payloadStr) {
    cJSON *json = cJSON_Parse(payloadStr);  
    if (!json || !cJSON_IsArray(json) || cJSON_GetArraySize(json) != 4) { 
        fprintf(stderr, "Coordinates must be an array of four elements\n");
        if (json) cJSON_Delete(json);
        return;
    }

    Coordinate new_position;
    float *np = (float*)&new_position; // Zeiger auf die neue Position für direkte Zuweisungen
    cJSON *item;
    for (int i = 0; i < 4; i++) {
        item = cJSON_GetArrayItem(json, i);
        if (!cJSON_IsNumber(item)) { // Überprüfung, ob jedes Item eine Zahl ist
            fprintf(stderr, "All coordinates must be numbers\n");
            cJSON_Delete(json);
            return;
        }
        np[i] = item->valuedouble; // Zuweisung des Werts an die neue Position
    }

    if (memcmp(&currentPosition, &new_position, sizeof(Coordinate))) { // Vergleich mit der aktuellen Position
        char command[120];
        sprintf(command, "G1 X%.1f Y%.1f Z%.1f A%.1f F%d\n",
                new_position.x, new_position.y, new_position.z, new_position.phi, speedSetting); // Senden des G-Code-Kommandos
        processLine(command);
    } else {
        printf("No change in position.\n");
    }
    currentPosition = new_position; // Aktualisieren der globalen currentPosition
    cJSON_Delete(json);
}

// Verarbeitet den Gripper-Wert aus einem JSON-String und führt gegebenenfalls eine Aktion aus.
// Parameter:
//   - char *payloadStr: JSON-kodierter String, der den Gripper-Wert enthält.
void manualModeGripper(char *payloadStr) {
    int gripperValue = atoi(payloadStr); // Konvertiert Payload zu Integer

    if (gripperValue != currentGripperValue) { // Vergleich des neuen Gripper-Werts mit dem aktuellen Wert
        char command[15]; // Befehlsstring für den G-Code
        switch (currentGripper) {  // Auswahl des Befehls basierend auf dem aktuellen Greifertyp
            case parallel:
                if (0 <= gripperValue && gripperValue <= 100) {
                    sprintf(command, "M100 S%d", gripperValue);
                    processLine(command); // Senden des G-Code-Kommandos
                }
                break;
            case complient:
                if (-1 <= gripperValue && gripperValue <= 1) {
                    sprintf(command, "M200 S%d", gripperValue);
                    processLine(command); // Senden des G-Code-Kommandos
                }
                break;
            case magnet:
                if (0 <= gripperValue && gripperValue <= 1) {
                    sprintf(command, "M300 S%d", gripperValue);
                    processLine(command); // Senden des G-Code-Kommandos
                }
                break;
            case vaccum:
                if (-1 <= gripperValue && gripperValue <= 1) {
                    sprintf(command, "M400 S%d", gripperValue);
                    processLine(command); // Senden des G-Code-Kommandos
                }
                break;
        }
    } else {
        printf("No change in GripperValue.\n");
    }
    
}