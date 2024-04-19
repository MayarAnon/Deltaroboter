//  gcc -o gcodeInterpreter ./gcodeInterpreter.c  -I/usr/local/include/cjson -L/usr/local/lib/cjson mqttClient.c pathInterpolation.c -lpaho-mqtt3c inverseKinematic.c -lm -lcjson



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
Plane currentPlane = XY_PLANE;
Gripper currentGripper = parallel;

bool stopFlag = false;

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
double calculateAngle(double xs, double ys, double xm, double ym, double xe, double ye, int direction);
void processInterpolationAndCreateJSON(Coordinate* coordinates, int InterpolationSteps, float f);
void removeNonPrintable(char *str);


void onMessage(char *topicName, char *payloadStr) {
    if (strcmp(topicName, MANUELCONTROLTOPIC) == 0) {
        
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
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        return;
    }

    while ((read = getline(&line, &len, file)) != -1) {
        if(stopFlag){
            break;
        }
        processLine(line);
    }

    free(line);  // Wichtig, um den von getline zugewiesenen Speicher freizugeben
    fclose(file);
}

void processLine(const char* line) {
    char command[4];
    float x, y,z,phi, i, j, f,t;
    int numParams;

    // Initialize parameters
    x = y  = i = j = phi = t = 0.0;
    f = 100;
    z = -280.0;
    // Determine the type of command
    numParams = sscanf(line, "%s", command);

    if (numParams < 1) {  // No command read
        return;
    }

    if (strcmp(command, "G0") == 0) {
        // Read G0/G1  Point to Point
        //Parameter aus string lesen 
        Coordinate* coordinates = (Coordinate*)malloc(2 * sizeof(Coordinate));
        numParams = sscanf(line, "%*s X%f Y%f Z%f A%f F%f", &x, &y,&z,&phi,&f);
        if (numParams >= 1) {  // At least X, Y, Z must be present
            coordinates[0] = currentPosition;
            coordinates[1].x = x;
            coordinates[1].y = y;
            coordinates[1].z = z;
            coordinates[1].phi = phi;
            
            printf("(%f, %f, %f)\n", x, y, z);
        } else {
            printf("Error reading parameters for command in line");
        }
        
        processInterpolationAndCreateJSON(coordinates,2, f);
    
    }
    else if (strcmp(command, "G1") == 0) {
        // Read G0/G1 command parameters
        numParams = sscanf(line, "%*s X%f Y%f Z%f A%f F%f", &x, &y,&z,&phi,&f);
        if (numParams < 2) {
            printf("Error reading parameters for %s command\n", command);
            return;
        }
        float diffX = fabs(x - currentPosition.x);
        float diffY = fabs(y - currentPosition.y);
        float diffZ = fabs(z - currentPosition.z);

        Coordinate targetPosition = {x,y,z,phi};
        
        float maxDiff = fmax(diffX, fmax(diffY, diffZ));
        int InterpolationSteps = (int)maxDiff;

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
        switch(currentPlane) {
        case XY_PLANE:
            
            numParams = sscanf(line, "%*s X%f Y%f I%f J%f A%f F%f", &x, &y, &i, &j, &phi, &f);
            angle = calculateAngle(currentPosition.x,currentPosition.y,currentPosition.x + i,currentPosition.y + j,x,y, direction);
            radius = hypot(i, j);
            center = (Coordinate){currentPosition.x + i, currentPosition.y + j, currentPosition.z,phi};
            break;
        case YZ_PLANE:
            numParams = sscanf(line, "%*s X%f Y%f I%f J%f A%f F%f", &x, &y, &i, &j, &phi, &f);
            angle = calculateAngle(currentPosition.y,currentPosition.z,currentPosition.y + i,currentPosition.z + j,y,z, direction);
            radius = hypot(i, j);
            center = (Coordinate){currentPosition.x, currentPosition.y + i, currentPosition.z + j,phi};
            break;
        case ZX_PLANE:
            
            numParams = sscanf(line, "%*s X%f Y%f I%f J%f A%f F%f", &x, &y, &i, &j, &phi, &f);
            angle = calculateAngle(currentPosition.z,currentPosition.x,currentPosition.z + i,currentPosition.x + j,z,x, direction);
            radius = hypot(i, j);
            center = (Coordinate){currentPosition.x + j, currentPosition.y, currentPosition.z + i,phi};
            break;
        }

        //Prüfen ob Parameter vollständig
        if (numParams < 2) {
            printf("Error reading parameters for %s command\n", command);
            return;
        }
        
        //Berechnen des zurückzulegenden winkels
        if (angle != 0) {
        //("Winkel: %f Grad Radius %f \n", angle,radius);
        }
        // Berechnen der Interpolationsschritte über die Distanz welche zurückgelegt wird 
        double angleRadient = (angle * PI)/180;
        double distance = fabs(angleRadient * radius); //distance in mm
        int InterpolationSteps = (int)(distance);
        
        
        Coordinate* coordinates =  circularInterpolation(currentPosition,center,currentPlane,(float)(angle),InterpolationSteps);
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
    

    /*     
    for(int i=0;i<InterpolationSteps;i++){
            char jsonString[40];  // Ensure the buffer is large enough
            snprintf(jsonString, sizeof(jsonString),
                     "(%f,%f,%f),",coordinates[i].x, coordinates[i].y, coordinates[i].z + 280);
            publishMessage("current/coordinates",jsonString);
    }
    */ 
    Steps* steps = malloc(InterpolationSteps * sizeof(Steps));
    
    cJSON* jsonRoot = cJSON_CreateArray();

    delta_calcInverse(currentPosition.x, currentPosition.y, currentPosition.z, &currentAngles.theta1, &currentAngles.theta2, &currentAngles.theta3); 

    float errorAccumulator1 = 0.0, errorAccumulator2 = 0.0, errorAccumulator3 = 0.0 , errorAccumulator4 = 0.0;

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

            // Update error accumulators with the fractional part that was cut off
            errorAccumulator1 = stepCalc1 - steps[i].Motor1;
            errorAccumulator2 = stepCalc2 - steps[i].Motor2;
            errorAccumulator3 = stepCalc3 - steps[i].Motor3;
            errorAccumulator4 = stepCalc4 - steps[i].Motor4;
            if (steps[i].Motor1 != 0 || steps[i].Motor2 != 0 || steps[i].Motor3 != 0) {
                cJSON* stepObj = cJSON_CreateObject();
                cJSON_AddItemToObject(stepObj, "motorpulses", cJSON_CreateIntArray((int[]){steps[i].Motor1, steps[i].Motor2, steps[i].Motor3,steps[i].Motor4}, 4));
                cJSON_AddItemToObject(stepObj, "timing", cJSON_CreateIntArray((int[]){(int)f, (int)f, 5}, 3));
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
// Funktion zur Berechnung des Winkels in Grad
double calculateAngle(double xs, double ys, double xm, double ym, double xe, double ye, int direction) {
    // Prüfen, ob Start- und Endpunkt identisch sind
    if (xs == xe && ys == ye) {
        if (xm != xs || ym != ys) {
            // Komplette Kreisbewegung, wenn Mittelpunkt nicht gleich Start/Endpunkt
            return direction * 360.0;
        }
    }

    double sm_x = xs - xm;
    double sm_y = ys - ym;
    double em_x = xe - xm;
    double em_y = ye - ym;
    
    double sm_mag = sqrt(sm_x * sm_x + sm_y * sm_y);
    double em_mag = sqrt(em_x * em_x + em_y * em_y);

    // Überprüfen, ob beide Punkte den gleichen Abstand zum Mittelpunkt haben
    if (fabs(sm_mag - em_mag) > 1e-6) {  // Verwenden einer kleinen Toleranz für Gleitkomma-Vergleiche
        printf("Die Punkte liegen nicht auf demselben Kreis.\n");
        return 0;  // Kein gültiger Winkel kann berechnet werden, wenn es kein Kreis ist
    }

    double dot = sm_x * em_x + sm_y * em_y;
    double cos_angle = dot / (sm_mag * em_mag);
    double raw_angle = acos(cos_angle) * 180 / PI;  // Rohwinkel in Grad

    // Überprüfung der Drehrichtung anhand des Kreuzprodukts
    double cross_product = sm_x * em_y - sm_y * em_x;
    if ((direction == 1 && cross_product < 0) || (direction == -1 && cross_product > 0)) {
        // Winkel korrigieren, falls die Bewegung entgegen der gewünschten Richtung ist
        raw_angle = 360 - raw_angle;
    }
    
    // Richtungskorrektur: -1 für im Uhrzeigersinn, 1 für gegen den Uhrzeigersinn
    return direction * raw_angle;
}