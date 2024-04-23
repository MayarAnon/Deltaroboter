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

#include "global.h"


void processLine(const char* line); 
void processInterpolationAndCreateJSON(Coordinate* coordinates, int InterpolationSteps, float f);

void publishCurrentState(Coordinate pos, Angles ang); 


// Verarbeitet eine einzelne Zeile des G-Code-Befehls.
// Parameter:
//   - const char* line: Die Zeile des G-Codes, die verarbeitet werden soll.
void processLine(const char* line) {
    char command[4];
    float x, y,z,phi, i, j, f,t,r;
    int numParams;

    // Initialisiere Parameter mit Standardwerten
    x = y  = i = j = phi = t = r = 0.0;
    f = speedSetting;
    z = -280.0;
    // Extrahiere den Befehlstyp aus der Zeile
    numParams = sscanf(line, "%s", command);

    if (numParams < 1) {  //Kein Command erkannt
        return;
    }

    if (strcmp(command, "G0") == 0) {
        //Parameter aus string lesen 
        Coordinate* coordinates = (Coordinate*)malloc(2 * sizeof(Coordinate));
        for(const char *p = line; *p; ++p) {
        sscanf(p, "X%f", &x) || sscanf(p, "Y%f", &y) || sscanf(p, "Z%f", &z) || sscanf(p, "A%f", &phi) || sscanf(p, "F%f", &f);
        }
        speedSetting = f;
        coordinates[0] = currentPosition;
        coordinates[1] = (Coordinate){x,y,z,phi};

        processInterpolationAndCreateJSON(coordinates,2, f);
    
    }
    else if (strcmp(command, "G1") == 0) {
        //Parameter aus string lesen 
        for(const char *p = line; *p; ++p) {
        sscanf(p, "X%f", &x) || sscanf(p, "Y%f", &y) || sscanf(p, "Z%f", &z) || sscanf(p, "A%f", &phi) || sscanf(p, "F%f", &f);
        }
        speedSetting = f;
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
    // Verarbeitet Kreisbewegungen gemäß den G-Code-Befehlen G2 (Kreis im Uhrzeigersinn) und G3 (Kreis gegen den Uhrzeigersinn).
    else if (strcmp(command, "G2") == 0 ||  strcmp(command, "G3" ) == 0) {
        // Bestimme die Drehrichtung basierend auf dem G-Code-Befehl
        int direction = 0;
        if(strcmp(command, "G2") == 0){
            direction = 1; // Uhrzeigersinn
        }
        else if(strcmp(command, "G3") == 0){
            direction = -1; // Gegen den Uhrzeigersinn
        }
        

        double angle, radius = 0;
        Coordinate center; 
        Coordinate end;
        //Parameter X: X-Achse Y: Y-Achse Z: Z-Achese A: Rotationsachse Endeffektor F: Speed
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
        speedSetting = f;
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
            // JSON-String vorbereiten 
            char jsonString[100];  // Buffer für String
            snprintf(jsonString, sizeof(jsonString),
                     "{\n"
                     "\"parallelGripper\": %d,\n"
                     "\"compliantGripper\": 0,\n"
                     "\"magnetGripper\": 0,\n"
                     "\"vacuumGripper\": 0\n"
                     "}", sValue);

            // Print den JSON string
            //printf("%s\n", jsonString);
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
            // JSON-String vorbereiten
            char jsonString[100];  // Buffer für String
            snprintf(jsonString, sizeof(jsonString),
                     "{\n"
                     "\"parallelGripper\": 0,\n"
                     "\"compliantGripper\": %d,\n"
                     "\"magnetGripper\": 0,\n"
                     "\"vacuumGripper\": 0\n"
                     "}", sValue);

            // Print den JSON string
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
            // JSON-String vorbereiten
            char jsonString[100];   // Buffer für String
            snprintf(jsonString, sizeof(jsonString),
                     "{\n"
                     "\"parallelGripper\": 0,\n"
                     "\"compliantGripper\": 0,\n"
                     "\"magnetGripper\": %d,\n"
                     "\"vacuumGripper\": 0\n"
                     "}", sValue);

            // Print den JSON string
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
            // JSON-String vorbereiten
            char jsonString[100];  // Buffer für String
            snprintf(jsonString, sizeof(jsonString),
                     "{\n"
                     "\"parallelGripper\": 0,\n"
                     "\"compliantGripper\": 0,\n"
                     "\"magnetGripper\": 0,\n"
                     "\"vacuumGripper\": %d\n"
                     "}", sValue);

            // Print den JSON string
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

// Verarbeitet die Interpolation erzeugt eine entsprechende JSON-Nachricht und publisht diese an MotorControll.
// Parameter:
//   - Coordinate* coordinates: Array von Koordinaten für die Interpolation.
//   - int InterpolationSteps: Anzahl der Schritte in einer Interpolation.
//   - float f: Geschwindigkeitsfaktor
void processInterpolationAndCreateJSON(Coordinate* coordinates, int InterpolationSteps, float f) {
    Steps* steps = malloc(InterpolationSteps * sizeof(Steps));
    Coordinate localPosition = currentPosition;  // Lokale Kopie der aktuellen Position
    Angles localAngles = currentAngles;       // Lokale Kopie der aktuellen Winkel
    Steps localSteps = currentSteps;          // Lokale Kopie der Schrittzahlen
    double localErrorAccumulators[4] = {errorAccumulator1, errorAccumulator2, errorAccumulator3, errorAccumulator4};
    
    cJSON* jsonRoot = cJSON_CreateArray();
    long long totalDuration = 0;
    bool errorOccurred = false;

    for (int i = 0; i < InterpolationSteps; i++) {
        float theta1, theta2, theta3;

        localPosition.x = coordinates[i].x;
        localPosition.y = coordinates[i].y;
        localPosition.z = coordinates[i].z;

        if (delta_calcInverse(localPosition.x, localPosition.y, localPosition.z, &theta1, &theta2, &theta3) == 0) {
            double stepCalc1 = ((theta1 - localAngles.theta1)/360) * STEPSPERREVOLUTION * GEARRATIO + localErrorAccumulators[0];
            double stepCalc2 = ((theta2 - localAngles.theta2)/360) * STEPSPERREVOLUTION * GEARRATIO + localErrorAccumulators[1];
            double stepCalc3 = ((theta3 - localAngles.theta3)/360) * STEPSPERREVOLUTION * GEARRATIO + localErrorAccumulators[2];
            double stepCalc4 = ((coordinates[i].phi - localPosition.phi)/360) * STEPSPERREVOLUTION + localErrorAccumulators[3];

            steps[i].Motor1 = round(stepCalc1);
            steps[i].Motor2 = round(stepCalc2);
            steps[i].Motor3 = round(stepCalc3);
            steps[i].Motor4 = round(stepCalc4);

            localSteps.Motor1 += steps[i].Motor1;
            localSteps.Motor2 += steps[i].Motor2;
            localSteps.Motor3 += steps[i].Motor3;
            localSteps.Motor4 += steps[i].Motor4;

            int maxSteps = abs(steps[i].Motor1);
            maxSteps = fmax(maxSteps, abs(steps[i].Motor2));
            maxSteps = fmax(maxSteps, abs(steps[i].Motor3));
            maxSteps = fmax(maxSteps, abs(steps[i].Motor4));

            long long stepDuration = (long long)(maxSteps * 2 * f);
            totalDuration += stepDuration;

            localErrorAccumulators[0] = stepCalc1 - steps[i].Motor1;
            localErrorAccumulators[1] = stepCalc2 - steps[i].Motor2;
            localErrorAccumulators[2] = stepCalc3 - steps[i].Motor3;
            localErrorAccumulators[3] = stepCalc4 - steps[i].Motor4;
            localAngles.theta1 = theta1;
            localAngles.theta2 = theta2;
            localAngles.theta3 = theta3;
            localPosition.phi = coordinates[i].phi;

            // Überprüfung, ob eine Nachricht aufgeteilt werden muss
            int splitCount[4];
            int pulses[4] = {steps[i].Motor1, steps[i].Motor2, steps[i].Motor3, steps[i].Motor4};
            for (int j = 0; j < 4; j++) {
                splitCount[j] = (abs(pulses[j]) + 4999) / 5000; // Berechnet, wie viele Nachrichten nötig sind Maximale Anzahl an Pulsen pro Nachricht 5000
            }
            int maxSplit = fmax(fmax(splitCount[0], splitCount[1]), fmax(splitCount[2], splitCount[3]));

            for (int k = 0; k < maxSplit; k++) {
                int currentPulses[4];
                for (int j = 0; j < 4; j++) {
                    currentPulses[j] = pulses[j] / splitCount[j];
                    pulses[j] -= currentPulses[j];
                    splitCount[j]--;
                }
                if (currentPulses[0] != 0 || currentPulses[1] != 0 || currentPulses[2] != 0 || currentPulses[3] != 0) {
                    cJSON* stepObj = cJSON_CreateObject();
                    cJSON_AddItemToObject(stepObj, "motorpulses", cJSON_CreateIntArray(currentPulses, 4));
                    cJSON_AddItemToObject(stepObj, "timing", cJSON_CreateIntArray((int[]){(int)f, (int)f, 5}, 3));
                    cJSON_AddItemToArray(jsonRoot, stepObj);
                }
            }
        } else {
            errorOccurred = true;
            printf("Punkt existiert nicht (%f,%f,%f),\n", coordinates[i].x, coordinates[i].y, coordinates[i].z);
            break;
        }
    }

    if (!errorOccurred) {
        currentPosition = localPosition;
        currentAngles = localAngles;
        currentSteps = localSteps;
        errorAccumulator1 = localErrorAccumulators[0];
        errorAccumulator2 = localErrorAccumulators[1];
        errorAccumulator3 = localErrorAccumulators[2];
        errorAccumulator4 = localErrorAccumulators[3];

        char* jsonString = cJSON_Print(jsonRoot);
        publishMessage("motors/sequence", jsonString);
        publishCurrentState(currentPosition, currentAngles);
        usleep(totalDuration);
        free(jsonString);
    }

    cJSON_Delete(jsonRoot);
    free(steps);
    free(coordinates);
}

// Veröffentlicht den aktuellen Zustand der Koordinaten und Winkel des Roboters.
// Parameter:
//   - Coordinate pos: Die aktuellen Koordinaten.
//   - Angles ang: Die aktuellen Winkel.
void publishCurrentState(Coordinate pos, Angles ang) {
    char coordString[50], anglesString[40];
    snprintf(coordString, sizeof(coordString), "(%f, %f, %f),", pos.x, pos.y, pos.z);
    snprintf(anglesString, sizeof(anglesString), "(%f, %f, %f),", ang.theta1, ang.theta2, ang.theta3);
    publishMessage("current/coordinates", coordString);
    publishMessage("current/angles", anglesString);
}