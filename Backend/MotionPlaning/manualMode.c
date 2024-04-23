#include "cJSON.h"
#include "global.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "gcodeParser.h"
#include "mqttClient.h"

void manualMode(char *payloadStr){
    cJSON *json = cJSON_Parse(payloadStr);
        if (json == NULL) {
            fprintf(stderr, "Error parsing JSON\n");
            return;
        }
        cJSON *coordinates = cJSON_GetObjectItemCaseSensitive(json, "coordinates");
        if (!cJSON_IsArray(coordinates) || cJSON_GetArraySize(coordinates) != 4) {
            fprintf(stderr, "Coordinates must be an array of four elements\n");
            cJSON_Delete(json);
            return;
        }

        Coordinate new_position;
        float *np = (float*)&new_position;
        cJSON *item;
        for (int i = 0; i < 4; i++) {
            item = cJSON_GetArrayItem(coordinates, i);
            if (!cJSON_IsNumber(item)) {
                fprintf(stderr, "All coordinates must be numbers\n");
                cJSON_Delete(json);
                return;
            }
            np[i] = item->valuedouble;
        }

        if (memcmp(&currentPosition, &new_position, sizeof(Coordinate))) {
            
            char command[120];  // Buffer für den zu sendenden String
            sprintf(command, "G1 X%.1f Y%.1f Z%.1f A%.1f F%d\n",
                    new_position.x, new_position.y, new_position.z, new_position.phi,speedSetting);
            processLine(command);
        } else {
            printf("No change in position.\n");  // Debug-Ausgabe, falls keine Änderung
        }
        currentPosition = new_position;  // Update the global currentPosition

        // Parse the "gripper" element
        cJSON *gripper = cJSON_GetObjectItemCaseSensitive(json, "gripper");
        if (gripper == NULL || !cJSON_IsNumber(gripper)) {
            fprintf(stderr, "Gripper must be a number\n");
            cJSON_Delete(json);
            return;
        }
        int gripperValue = gripper->valueint;  // Assuming gripper is an integer

         // Respond based on gripper value
        switch (currentGripper) {
            char command[15];

            case parallel:

                if(0 <= gripperValue && gripperValue <= 100){
                    sprintf(command, "M100 S%d",gripperValue);
                    processLine(command);
                }
                break;
            case complient:
                
                if(-1 <= gripperValue && gripperValue <= 1){
                    sprintf(command, "M200 S%d",gripperValue);
                    processLine(command);
                }
                break;
            case magnet:
              
                if(0 <= gripperValue && gripperValue <= 1){
                    sprintf(command, "M300 S%d",gripperValue);
                    processLine(command);
                }
                break;
            case vaccum:

                if(-1 <= gripperValue && gripperValue <= 1){
                    sprintf(command, "M400 S%d",gripperValue);
                    processLine(command);
                }
                break;
        }


        cJSON_Delete(json);
}