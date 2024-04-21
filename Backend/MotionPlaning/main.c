//  gcc -o program ./main.c  -I/usr/local/include/cjson -L/usr/local/lib/cjson mqttClient.c pathInterpolation.c -lpaho-mqtt3c inverseKinematic.c -lm -lcjson



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

typedef struct {
    float theta1, theta2, theta3;
} Angles;

typedef struct {
    int Motor1, Motor2, Motor3,Motor4;
} Steps;

typedef enum {
    parallel,
    complient,
    magnet,
    vaccum
} Gripper;

Coordinate currentPosition = {0.0, 0.0,-280.0,0.0};  // Startposition
Angles currentAngles = {-41.489,-41.489,-41.489}; //Startangle
Steps currentSteps = {0};
Plane currentPlane = XY_PLANE;
Gripper currentGripper = parallel;
int speedSetting = 50;
bool stopFlag = false;
float errorAccumulator1 = 0.0, errorAccumulator2 = 0.0, errorAccumulator3 = 0.0 , errorAccumulator4 = 0.0;

#define STEPSPERREVOLUTION 800
#define GEARRATIO 20
#define PI 3.14159265358979323846

//subscribe Topics
#define ROBOTSTATETOPIC "robot/state"
#define LOADPROGRAMMTOPIC "pickandplace/program"
#define MANUELCONTROLTOPIC "manual/control"
#define STOPTOPIC "motors/stop"
//publish Topics
#define MOTORCONTROLLTOPIC "motors/sequence"
#define GRIPPERCONTROLLTOPIC "gripper/control"
#define COORDINATESTOPIC "current/coordinates"
#define ANGLESTOPIC "current/angles"

void readFile(const char* filename);
void processLine(const char* line);
void processInterpolationAndCreateJSON(Coordinate* coordinates, int InterpolationSteps, float f);
void removeNonPrintable(char *str);
void manualMode(char *payloadStr);
void parseRobotState(const char *payloadStr);


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

void onMessage(char *topicName, char *payloadStr) {
    if (strcmp(topicName, MANUELCONTROLTOPIC) == 0) {
        manualMode(payloadStr);
    }
    else if (strcmp(topicName, STOPTOPIC) == 0) {
        if(strcmp("true",payloadStr ) == 0){
            stopFlag = true;
        }
    }
    else if (strcmp(topicName, LOADPROGRAMMTOPIC) == 0) {
        stopFlag = false;
        readFile(payloadStr);
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
        perror("Failed to open file");
        return;
    }

    while ((read = getline(&line, &len, file)) != -1) {
        if (stopFlag) {
            break;
        }
        processLine(line);
    }

    free(line);  // Wichtig, um den von getline zugewiesenen Speicher freizugeben
    fclose(file);
}

void processLine(const char* line) {
    char command[4];
    float x, y,z,phi, i, j, f,t,r;
    int numParams;

    // Initialize parameters
    x = y  = i = j = phi = t = r = 0.0;
    f = 100;
    z = -280.0;
    // Determine the type of command
    numParams = sscanf(line, "%s", command);

    if (numParams < 1) {  // No command read
        return;
    }

    if (strcmp(command, "G0") == 0) {
        //Parameter aus string lesen 
        Coordinate* coordinates = (Coordinate*)malloc(2 * sizeof(Coordinate));
        for(const char *p = line; *p; ++p) {
        sscanf(p, "X%f", &x) || sscanf(p, "Y%f", &y) || sscanf(p, "Z%f", &z) || sscanf(p, "A%f", &phi) || sscanf(p, "F%f", &f);
        }

        coordinates[0] = currentPosition;
        coordinates[1] = (Coordinate){x,y,z,phi};

        processInterpolationAndCreateJSON(coordinates,2, f);
    
    }
    else if (strcmp(command, "G1") == 0) {
        //Parameter aus string lesen 
        for(const char *p = line; *p; ++p) {
        sscanf(p, "X%f", &x) || sscanf(p, "Y%f", &y) || sscanf(p, "Z%f", &z) || sscanf(p, "A%f", &phi) || sscanf(p, "F%f", &f);
        }

        float diffX = fabs(x - currentPosition.x);
        float diffY = fabs(y - currentPosition.y);
        float diffZ = fabs(z - currentPosition.z);

        Coordinate targetPosition = {x,y,z,phi};
        
        float maxDiff = fmax(diffX, fmax(diffY, diffZ));

        int InterpolationSteps = (int)maxDiff < 2 ? 2 : (int)maxDiff;

        Coordinate* coordinates = linearInterpolation(currentPosition, targetPosition, InterpolationSteps);
        /*
        for(int i=0;i<InterpolationSteps;i++){
            printf("(%f,%f,%f),\n",coordinates[i].x, coordinates[i].y, coordinates[i].z + 280);
            currentPosition.x = coordinates[i].x;
            currentPosition.y = coordinates[i].y;
            currentPosition.z = coordinates[i].z;
        }
        */
        processInterpolationAndCreateJSON(coordinates,InterpolationSteps,f);
        
        
    }
    else if (strcmp(command, "G2") == 0 || strcmp(command, "G3") == 0) {

        int direction = 0;
        if(strcmp(command, "G2") == 0){
            direction = 1;
        }
        else if(strcmp(command, "G3") == 0){
            direction = -1;
        }
        
        //auf ein Achse zurückführen die X-Y Achse. wird nachher wieder auf Ursprüngliche Achese angewendet
        double angle, radius = 0;
        Coordinate center;
        Coordinate end;
        switch(currentPlane) {
        case XY_PLANE:
            
            for(const char *p = line; *p; ++p) {
            sscanf(p, "X%f", &x) || sscanf(p, "Y%f", &y) || sscanf(p, "I%f", &i) || sscanf(p, "J%f", &j) || sscanf(p, "R%f", &r)|| sscanf(p, "A%f", &phi) || sscanf(p, "F%f", &f);
            }
            center = (Coordinate){currentPosition.x + i,currentPosition.y + j,currentPosition.z,currentPosition.phi};
            end = (Coordinate){x,y,currentPosition.z,currentPosition.phi};
            int numSteps = 0;
            

            break;
        case YZ_PLANE:
            for(const char *p = line; *p; ++p) {
            sscanf(p, "Y%f", &y) || sscanf(p, "Z%f", &z) || sscanf(p, "I%f", &i) || sscanf(p, "J%f", &j) || sscanf(p, "R%f", &r)|| sscanf(p, "A%f", &phi) || sscanf(p, "F%f", &f);
            }
            center = (Coordinate){currentPosition.x ,currentPosition.y + i,currentPosition.z + j,currentPosition.phi};
            end = (Coordinate){currentPosition.x,y,z,currentPosition.phi};
            break;
        case ZX_PLANE:
            
            for(const char *p = line; *p; ++p) {
            sscanf(p, "Z%f", &z) || sscanf(p, "X%f", &x) || sscanf(p, "I%f", &i) || sscanf(p, "J%f", &j) || sscanf(p, "R%f", &r)|| sscanf(p, "A%f", &phi) || sscanf(p, "F%f", &f);
            }
            center = (Coordinate){currentPosition.x + j,currentPosition.y ,currentPosition.z + i,currentPosition.phi};
            end = (Coordinate){x,currentPosition.y,z,currentPosition.phi};
            break;
        }
        int numSteps = 0;
        Coordinate* coordinates = circularInterpolation(currentPosition, end, center,r, currentPlane,direction, &numSteps);
        processInterpolationAndCreateJSON(coordinates,numSteps,f);
    }
    else if (strcmp(command, "G4") == 0) {
        int numParams = sscanf(line, "%*s P%f", &t);

        if (numParams < 1) {
            fprintf(stderr, "Failed to read sleep time for G4 command\n");
            return;
        } else {
            usleep(t * 1000);  // Perform the sleep
        }
    }
    else if (strcmp(command, "G17") == 0) {
        
        currentPlane = XY_PLANE;
        //printf("changed Plane XY \n");
    }
    else if (strcmp(command, "G18") == 0) {
        
        currentPlane = ZX_PLANE;
        //printf("changed Plane ZX\n");

    }
    else if (strcmp(command, "G19") == 0) {
        
        currentPlane = YZ_PLANE;
        //printf("changed Plane YZ \n");

    }
    else if (strcmp(command, "G28") == 0) {
        Coordinate* targetPosition = (Coordinate*)malloc(2 * sizeof(Coordinate));
    
        targetPosition[0] = currentPosition;
        
        // Korrigiere die Definition des zweiten Elements des Arrays
        targetPosition[1] = (Coordinate){0.0, 0.0, -280, 0.0};
            
        processInterpolationAndCreateJSON(targetPosition, 2, f);
    
    }
    else if (strcmp(command, "M100") == 0) {
        int sValue = 0;
        int numParams = sscanf(line, "%*s S%d", &sValue);

        if (numParams < 1) {
            fprintf(stderr, "Failed to read sleep time for M100 command\n");
            return;
        } else {
            // Prepare the JSON string
            char jsonString[100];  // Ensure the buffer is large enough
            snprintf(jsonString, sizeof(jsonString),
                     "{\n"
                     "\"parallelGripper\": %d,\n"
                     "\"compliantGripper\": 0,\n"
                     "\"magnetGripper\": 0,\n"
                     "\"vacuumGripper\": 0\n"
                     "}", sValue);

            // Print the JSON string
            printf("%s\n", jsonString);
            publishMessage(GRIPPERCONTROLLTOPIC,jsonString);
        }   

    }
    else if (strcmp(command, "M200") == 0) {
        int sValue = 0;
        int numParams = sscanf(line, "%*s S%d", &sValue);

        if (numParams < 1) {
            fprintf(stderr, "Failed to read sleep time for M100 command\n");
            return;
        } else {
            // Prepare the JSON string
            char jsonString[100];  // Ensure the buffer is large enough
            snprintf(jsonString, sizeof(jsonString),
                     "{\n"
                     "\"parallelGripper\": 0,\n"
                     "\"compliantGripper\": %d,\n"
                     "\"magnetGripper\": 0,\n"
                     "\"vacuumGripper\": 0\n"
                     "}", sValue);

            // Print the JSON string
            printf("%s\n", jsonString);
            publishMessage(GRIPPERCONTROLLTOPIC,jsonString);
        }
   
    }
    else if (strcmp(command, "M300") == 0) {
        int sValue = 0;
        int numParams = sscanf(line, "%*s S%d", &sValue);

        if (numParams < 1) {
            fprintf(stderr, "Failed to read sleep time for M100 command\n");
            return;
        } else {
            // Prepare the JSON string
            char jsonString[100];  // Ensure the buffer is large enough
            snprintf(jsonString, sizeof(jsonString),
                     "{\n"
                     "\"parallelGripper\": 0,\n"
                     "\"compliantGripper\": 0,\n"
                     "\"magnetGripper\": %d,\n"
                     "\"vacuumGripper\": 0\n"
                     "}", sValue);

            // Print the JSON string
            printf("%s\n", jsonString);
            publishMessage(GRIPPERCONTROLLTOPIC,jsonString);
        }
    }
    else if (strcmp(command, "M400") == 0) {
        int sValue = 0;
        int numParams = sscanf(line, "%*s S%d", &sValue);

        if (numParams < 1) {
            fprintf(stderr, "Failed to read sleep time for M100 command\n");
            return;
        } else {
            // Prepare the JSON string
            char jsonString[100];  // Ensure the buffer is large enough
            snprintf(jsonString, sizeof(jsonString),
                     "{\n"
                     "\"parallelGripper\": 0,\n"
                     "\"compliantGripper\": 0,\n"
                     "\"magnetGripper\": 0,\n"
                     "\"vacuumGripper\": %d\n"
                     "}", sValue);

            // Print the JSON string
            printf("%s\n", jsonString);
            publishMessage(GRIPPERCONTROLLTOPIC,jsonString);
        }
    }
    else if (strcmp(command, ";") == 0) {
        
    }
    else {
        printf("Unsupported command: %s\n", command);
    }
}

void removeNonPrintable(char *str) {
    char *src = str, *dst = str;
    while(*src) {
        // Erlaube nur druckbare Zeichen ohne Zeilenumbrüche und Tabs
        if (*src >= 32 && *src <= 126) {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0'; // Null-terminate the cleaned string
}

void processInterpolationAndCreateJSON(Coordinate* coordinates, int InterpolationSteps, float f) {
    
    Steps* steps = malloc(InterpolationSteps * sizeof(Steps));
    
    cJSON* jsonRoot = cJSON_CreateArray();

    long long totalDuration = 0;  // Gesamtdauer der Bewegung in Mikrosekunden

    delta_calcInverse(currentPosition.x, currentPosition.y, currentPosition.z, &currentAngles.theta1, &currentAngles.theta2, &currentAngles.theta3); 


    for (int i = 0; i < InterpolationSteps; i++) {
        float theta1, theta2, theta3;
        
        currentPosition.x = coordinates[i].x;
        currentPosition.y = coordinates[i].y;
        currentPosition.z = coordinates[i].z;
        

        if (delta_calcInverse(coordinates[i].x, coordinates[i].y, coordinates[i].z, &theta1, &theta2, &theta3) == 0) {
            float stepCalc1 = ((theta1 - currentAngles.theta1)/360) * STEPSPERREVOLUTION * GEARRATIO + errorAccumulator1;
            float stepCalc2 = ((theta2 - currentAngles.theta2)/360) * STEPSPERREVOLUTION * GEARRATIO + errorAccumulator2;
            float stepCalc3 = ((theta3 - currentAngles.theta3)/360) * STEPSPERREVOLUTION * GEARRATIO + errorAccumulator3;
            float stepCalc4 = ((coordinates[i].phi - currentPosition.phi)/360) * STEPSPERREVOLUTION + errorAccumulator4;

            steps[i].Motor1 = (int)stepCalc1;
            steps[i].Motor2 = (int)stepCalc2;
            steps[i].Motor3 = (int)stepCalc3;
            steps[i].Motor4 = (int)stepCalc4;

            // Update der globalen Schrittanzahl
            currentSteps.Motor1 += steps[i].Motor1;
            currentSteps.Motor2 += steps[i].Motor2;
            currentSteps.Motor3 += steps[i].Motor3;
            currentSteps.Motor4 += steps[i].Motor4;

            // Berechnung der maximalen Schritte in diesem Zyklus
            int maxSteps = abs(steps[i].Motor1);
            maxSteps = fmax(maxSteps, abs(steps[i].Motor2));
            maxSteps = fmax(maxSteps, abs(steps[i].Motor3));
            maxSteps = fmax(maxSteps, abs(steps[i].Motor4));

            // Zeitdauer für diesen Zyklus
            long long stepDuration = (long long)(maxSteps * 2 * f);
            totalDuration += stepDuration;

            // Update error accumulators with the fractional part that was cut off
            errorAccumulator1 = stepCalc1 - steps[i].Motor1;
            errorAccumulator2 = stepCalc2 - steps[i].Motor2;
            errorAccumulator3 = stepCalc3 - steps[i].Motor3;
            errorAccumulator4 = stepCalc4 - steps[i].Motor4;
            if (steps[i].Motor1 != 0 || steps[i].Motor2 != 0 || steps[i].Motor3 != 0) {
                cJSON* stepObj = cJSON_CreateObject();
                cJSON_AddItemToObject(stepObj, "motorpulses", cJSON_CreateIntArray((int[]){steps[i].Motor1, steps[i].Motor2, steps[i].Motor3,steps[i].Motor4}, 4));
                cJSON_AddItemToObject(stepObj, "timing", cJSON_CreateIntArray((int[]){(int)f, (int)f,5}, 3));
                cJSON_AddItemToArray(jsonRoot, stepObj);
            }
            currentAngles.theta1 = theta1;
            currentAngles.theta2 = theta2;
            currentAngles.theta3 = theta3;
            currentPosition.phi = coordinates[i].phi;
        } else {
            printf("Punkt existiert nicht\n");
        }
    }

    char* jsonString = cJSON_Print(jsonRoot);
    
    removeNonPrintable(jsonString);
    printf("%s\n", jsonString);
    //Publish Motor Seqence 
    publishMessage("motors/sequence",jsonString);
    //printf("Gesamtdauer der Bewegung: %lld µs\n", totalDuration);
    usleep(totalDuration);
    // Ausgabe der aktuellen Gesamtschritte
    //printf("Current Steps: Motor1=%d, Motor2=%d, Motor3=%d, Motor4=%d\n",
    //       currentSteps.Motor1, currentSteps.Motor2, currentSteps.Motor3, currentSteps.Motor4);

    //Publish current Coordinates
    char coordinateString[50];  // Ensure the buffer is large enough
            snprintf(coordinateString, sizeof(coordinateString),
                     "(%f,%f,%f,%f ),",currentPosition.x, currentPosition.y, currentPosition.z,currentPosition.phi);
            publishMessage("current/coordinates",coordinateString);
    //Publish current Angles
    char anglesString[40];  // Ensure the buffer is large enough
            snprintf(anglesString, sizeof(anglesString),
                     "(%f,%f,%f),",currentAngles.theta1,currentAngles.theta2, currentAngles.theta3);
            publishMessage("current/angles",anglesString);
    free(jsonString);
    cJSON_Delete(jsonRoot);
    free(steps);
    free(coordinates);
}
