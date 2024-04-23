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


void parseRobotState(const char *payloadStr) {
    cJSON *json = cJSON_Parse(payloadStr);
    if (json == NULL) {
        fprintf(stderr, "Error parsing JSON\n");
        return;
    }

    // Parsing boolean using stdbool.h
    cJSON *homing = cJSON_GetObjectItemCaseSensitive(json, "homing");
    bool homingValue = cJSON_IsTrue(homing);

    // Parsing array of doubles
    cJSON *currentCoordinates = cJSON_GetObjectItemCaseSensitive(json, "currentCoordinates");
    double coordinates[3];  // Assuming there are always three coordinates
    if (cJSON_IsArray(currentCoordinates)) {
        for (int i = 0; i < 3; i++) {
            cJSON *coord = cJSON_GetArrayItem(currentCoordinates, i);
            coordinates[i] = coord->valuedouble;
        }
    }

    // Parsing array of doubles
    cJSON *currentAngles = cJSON_GetObjectItemCaseSensitive(json, "currentAngles");
    double angles[3];  // Assuming there are always three angles
    if (cJSON_IsArray(currentAngles)) {
        for (int i = 0; i < 3; i++) {
            cJSON *angle = cJSON_GetArrayItem(currentAngles, i);
            angles[i] = angle->valuedouble;
        }
    }

    // Parsing boolean using stdbool.h
    cJSON *gripperFeedback = cJSON_GetObjectItemCaseSensitive(json, "gripperFeedback");
    bool gripperFeedbackValue = cJSON_IsTrue(gripperFeedback);

    // Parse gripper mode and convert to enum
    char *gripperModeStr = cJSON_GetObjectItemCaseSensitive(json, "gripperMode")->valuestring;
    Gripper gripperModeValue = parseGripperMode(gripperModeStr);

    // Parsing integer
    cJSON *motorsSpeed = cJSON_GetObjectItemCaseSensitive(json, "motorsSpeed");
    int motorsSpeedValue = motorsSpeed->valueint;

    // Parsing array of integers
    cJSON *robotWorkspace = cJSON_GetObjectItemCaseSensitive(json, "robotWorkspace");
    int workspace[2];  // Assuming there are always two elements
    if (cJSON_IsArray(robotWorkspace)) {
        for (int i = 0; i < 2; i++) {
            cJSON *space = cJSON_GetArrayItem(robotWorkspace, i);
            workspace[i] = space->valueint;
        }
    }

    if(homingValue){
        currentPosition = (Coordinate){0.0, 0.0,-280.0,0.0};
        
    }
    speedSetting = motorsSpeedValue;
    currentGripper = gripperModeValue;
    /*
    // Optional: Print values for debugging
    printf("Homing: %s\n", homingValue ? "true" : "false");
    printf("Coordinates: [%.1f, %.1f, %.1f]\n", coordinates[0], coordinates[1], coordinates[2]);
    printf("Angles: [%.1f, %.1f, %.1f]\n", angles[0], angles[1], angles[2]);
    printf("Gripper Feedback: %s\n", gripperFeedbackValue ? "true" : "false");
    printf("Gripper Mode: %d\n", gripperModeValue);
    printf("Motor Speed: %d\n", motorsSpeedValue);
    printf("Workspace: [%d, %d]\n", workspace[0], workspace[1]);
    */
    cJSON_Delete(json);
}