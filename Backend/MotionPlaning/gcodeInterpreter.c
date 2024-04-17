//  gcc -o gcodeInterpreter ./gcodeInterpreter.c  -I/usr/local/include/cjson -L/usr/local/lib/cjson mqttClient.c pathInterpolation.c -lpaho-mqtt3c inverseKinematic.c -lm -lcjson



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pathInterpolation.h"
#include "inverseKinematic.h"
#include "cJSON.h"

typedef struct {
    float theta1, theta2, theta3;
} Angles;

typedef struct {
    int Motor1, Motor2, Motor3;
} Steps;

Coordinate currentPosition = {0.0, 0.0,-280.0};  // Startposition
Angles currentAngles = {-41.489,-41.489,-41.489}; //Startangle
Plane currentPlane = XY_PLANE;

#define STEPSPERREVOLUTION 800
#define GEARRATIO 20
#define PI 3.14159265358979323846

void readFile(const char* filename);
void processLine(const char* line);
double calculateAngle(double xs, double ys, double xm, double ym, double xe, double ye, int direction);

int main() {
    
    readFile("test.gcode");
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
        processLine(line);
    }

    free(line);  // Wichtig, um den von getline zugewiesenen Speicher freizugeben
    fclose(file);
}



void processLine(const char* line) {
    char command[4];
    float x, y,z, i, j, f;
    int numParams;

    // Initialize parameters
    x = y = z = i = j = f = 0.0;

    // Determine the type of command
    numParams = sscanf(line, "%s", command);

    if (numParams < 1) {  // No command read
        return;
    }

    if (strcmp(command, "G0") == 0) {
        // Read G0/G1  Point to Point
        //Parameter aus string lesen 
        numParams = sscanf(line, "%*s X%f Y%f Z%f F%f", &x, &y,&z,&f);
        if (numParams < 2) {
            printf("Error reading parameters for %s command\n", command);
            return;
        }
        float theta1, theta2, theta3;
        
        if (delta_calcInverse(x, y, z, &theta1, &theta2, &theta3) == 0) {
        int Motor1 = (((theta1 - currentAngles.theta1)/360) * STEPSPERREVOLUTION * GEARRATIO);
        int Motor2 = (((theta2 - currentAngles.theta2)/360) * STEPSPERREVOLUTION * GEARRATIO);
        int Motor3 = (((theta3 - currentAngles.theta3)/360) * STEPSPERREVOLUTION * GEARRATIO);

        // Create the root node of the JSON as an array
        cJSON *jsonRoot = cJSON_CreateArray();
        
        // Create a JSON object
        cJSON *jsonObj = cJSON_CreateObject();
        
        // Adding motorpulses array
        cJSON_AddItemToObject(jsonObj, "motorpulses", cJSON_CreateIntArray((int[]){Motor1, Motor2, Motor3}, 3));

        // Adding timing array
        cJSON_AddItemToObject(jsonObj, "timing", cJSON_CreateIntArray((int[]){(int)f, (int)f, 5}, 3));
        
        // Add the object to the root array
        cJSON_AddItemToArray(jsonRoot, jsonObj);

        // Print and publish
        char* jsonString = cJSON_Print(jsonRoot);
        printf("%s\n", jsonString);  
        //update current Positon
        currentPosition.x = x;
        currentPosition.y = y;
        currentPosition.z = z;
        //printf("currentPosition (%f,%f,%f) \n",x,y,z);
        currentAngles.theta1 = theta1;
        currentAngles.theta2 = theta2;
        currentAngles.theta3 = theta3;
        //free storage
        free(jsonString);
        cJSON_Delete(jsonRoot);
        } else {
            fprintf(stderr, "Error calculating inverse kinematics\n");
        }
        
    }
        
    
    else if (strcmp(command, "G1") == 0) {
        // Read G0/G1 command parameters
        numParams = sscanf(line, "%*s X%f Y%f Z%f F%f", &x, &y,&z, &f);
        if (numParams < 2) {
            printf("Error reading parameters for %s command\n", command);
            return;
        }
        float diffX = fabs(x - currentPosition.x);
        float diffY = fabs(y - currentPosition.y);
        float diffZ = fabs(z - currentPosition.z);

        Coordinate targetPosition = {x,y,z};
        
        float maxDiff = fmax(diffX, fmax(diffY, diffZ));
        int InterpolationSteps = (int)maxDiff;

        Coordinate* coordinates = linearInterpolation(currentPosition, targetPosition, InterpolationSteps);

        for(int i=0;i<InterpolationSteps;i++){
            printf("(%f,%f,%f),\n",coordinates[i].x, coordinates[i].y, coordinates[i].z + 280);
            currentPosition.x = coordinates[i].x;
            currentPosition.y = coordinates[i].y;
            currentPosition.z = coordinates[i].z;
        }
        
        delta_calcInverse(currentPosition.x,currentPosition.y,currentPosition.z, &currentAngles.theta1, &currentAngles.theta2, &currentAngles.theta3);
        /*
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
                    cJSON_AddItemToObject(stepObj, "timing", cJSON_CreateIntArray((int[]){f, f,5}, 3));
                    cJSON_AddItemToArray(jsonRoot, stepObj);

                    //printf("Interpolation Step %d: DeltaTheta1 = %d, DeltaTheta2 = %d, DeltaTheta3 = %d\n",
                    //    i, steps[i].Motor1, steps[i].Motor2, steps[i].Motor3);
                }
                // Update lastTheta values for next iteration
                lastTheta1 = theta1;
                lastTheta2 = theta2;
                lastTheta3 = theta3;
            } else {
                printf("Punkt existiert nicht");
            }
        }
        currentPosition = targetPosition;
        delta_calcInverse(targetPosition.x, targetPosition.y, targetPosition.z, &currentAngles.theta1, &currentAngles.theta2, &currentAngles.theta3);
        char* jsonString = cJSON_Print(jsonRoot);  // Generate final JSON string
        printf("%s\n", jsonString); 
        free(jsonString);
        cJSON_Delete(jsonRoot);

        free(coordinates);
        free(steps);
        
        */
    }

    else if (strcmp(command, "G2") == 0) {
        
        //auf ein Achse zurückführen die X-Y Achse. wird nachher wieder auf Ursprüngliche Achese angewendet
        double angle, radius = 0;
        Coordinate center;
        switch(currentPlane) {
        case XY_PLANE:
            
            numParams = sscanf(line, "%*s X%f Y%f I%f J%f F%f", &x, &y, &i, &j, &f);
            angle = calculateAngle(currentPosition.x,currentPosition.y,currentPosition.x + i,currentPosition.y + j,x,y, 1);
            radius = sqrt(pow((currentPosition.x + i) - currentPosition.x, 2) + pow((currentPosition.y + j) - currentPosition.y, 2));
            center.x = currentPosition.x + i;
            center.y = currentPosition.y + j;
            center.z = currentPosition.z;
            break;
        case YZ_PLANE:
            numParams = sscanf(line, "%*s Y%f Z%f I%f J%f F%f", &y, &z, &i, &j, &f);
            angle = calculateAngle(currentPosition.y,currentPosition.z,currentPosition.y + i,currentPosition.z + j,y,z, 1);
            radius = sqrt(pow((currentPosition.y + i) - currentPosition.y, 2) + pow((currentPosition.z + j) - currentPosition.z, 2));
            center.x = currentPosition.x;
            center.y = currentPosition.y + i;
            center.z = currentPosition.z + j;
            break;
        case ZX_PLANE:
            
            numParams = sscanf(line, "%*s Z%f X%f I%f J%f F%f", &z, &x, &i, &j, &f);
            angle = calculateAngle(currentPosition.z,currentPosition.x,currentPosition.z + i,currentPosition.x + j,z,x, 1);
            radius = sqrt(pow((currentPosition.z + i) - currentPosition.z, 2) + pow((currentPosition.x + j) - currentPosition.x, 2));
            center.x = currentPosition.x + j;
            center.y = currentPosition.y;
            center.z = currentPosition.z + i;
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
        
        //printf("G2 circular move to (%f, %f) with center offsets (%f, %f) at feed rate %f\n", x, y, i, j, f);

        //printf("currentPosition (%f,%f,%f) center (%f,%f,%f) angle %f InterpolationSteps %d\n",currentPosition.x,currentPosition.y,currentPosition.z,center.x,center.y,center.z,angle,InterpolationSteps);
        
        Coordinate* coordinates =  circularInterpolation(currentPosition,center,currentPlane,(float)(angle),InterpolationSteps);

        for(int i=0;i<InterpolationSteps;i++){
            printf("(%f,%f,%f),\n",coordinates[i].x, coordinates[i].y, coordinates[i].z + 280);
            currentPosition.x = coordinates[i].x;
            currentPosition.y = coordinates[i].y;
            currentPosition.z = coordinates[i].z;
        }
        
        delta_calcInverse(currentPosition.x,currentPosition.y,currentPosition.z, &currentAngles.theta1, &currentAngles.theta2, &currentAngles.theta3);
        //printf("currentPosition (%f,%f,%f) \n",currentPosition.x,currentPosition.y,currentPosition.z);
        /*
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
                    cJSON_AddItemToObject(stepObj, "timing", cJSON_CreateIntArray((int[]){f, f,5}, 3));
                    cJSON_AddItemToArray(jsonRoot, stepObj);

                    //printf("Interpolation Step %d: DeltaTheta1 = %d, DeltaTheta2 = %d, DeltaTheta3 = %d\n",
                    //    i, steps[i].Motor1, steps[i].Motor2, steps[i].Motor3);
                }
                // Update lastTheta values for next iteration
                lastTheta1 = theta1;
                lastTheta2 = theta2;
                lastTheta3 = theta3;
            } else {
                printf("Punkt existiert nicht");
            }
        }
        //currentPosition = targetPosition;
        //delta_calcInverse(targetPosition.x, targetPosition.y, targetPosition.z, &currentAngles.theta1, &currentAngles.theta2, &currentAngles.theta3);
        char* jsonString = cJSON_Print(jsonRoot);  // Generate final JSON string
        printf("%s\n", jsonString); 
        free(jsonString);
        cJSON_Delete(jsonRoot);

        free(coordinates);
        free(steps);
        */
        // Optionally handle feed rate f and center (i, j) here
    }
    else if (strcmp(command, "G3") == 0) {
        //auf ein Achse zurückführen die X-Y Achse. wird nachher wieder auf Ursprüngliche Achese angewendet
        double angle, radius = 0;
        Coordinate center;
        switch(currentPlane) {
        case XY_PLANE:
            
            numParams = sscanf(line, "%*s X%f Y%f I%f J%f F%f", &x, &y, &i, &j, &f);
            angle = calculateAngle(currentPosition.x,currentPosition.y,currentPosition.x + i,currentPosition.y + j,x,y, -1);
            radius = sqrt(pow((currentPosition.x + i) - currentPosition.x, 2) + pow((currentPosition.y + j) - currentPosition.y, 2));
            center.x = currentPosition.x + i;
            center.y = currentPosition.y + j;
            center.z = currentPosition.z;
            break;
        case YZ_PLANE:
            numParams = sscanf(line, "%*s Y%f Z%f I%f J%f F%f", &y, &z, &i, &j, &f);
            angle = calculateAngle(currentPosition.y,currentPosition.z,currentPosition.y + i,currentPosition.z + j,y,z, -1);
            radius = sqrt(pow((currentPosition.y + i) - currentPosition.y, 2) + pow((currentPosition.z + j) - currentPosition.z, 2));
            center.x = currentPosition.x;
            center.y = currentPosition.y + i;
            center.z = currentPosition.z + j;
            break;
        case ZX_PLANE:
            
            numParams = sscanf(line, "%*s Z%f X%f I%f J%f F%f", &z, &x, &i, &j, &f);
            angle = calculateAngle(currentPosition.z,currentPosition.x,currentPosition.z + i,currentPosition.x + j,z,x, -1);
            radius = sqrt(pow((currentPosition.z + i) - currentPosition.z, 2) + pow((currentPosition.x + j) - currentPosition.x, 2));
            center.x = currentPosition.x + j;
            center.y = currentPosition.y;
            center.z = currentPosition.z + i;
            break;
        }

        //Prüfen ob Parameter vollständig
        if (numParams < 2) {
            printf("Error reading parameters for %s command\n", command);
            return;
        }
        
        //Berechnen des zurückzulegenden winkels
        if (angle != 0) {
        //printf("Winkel: %f Grad Radius %f \n", angle,radius);
        }
        // Berechnen der Interpolationsschritte über die Distanz welche zurückgelegt wird 
        double angleRadient = (angle * PI)/180;
        double distance = fabs(angleRadient * radius); //distance in mm
        int InterpolationSteps = (int)(distance);
        
        //printf("G2 circular move to (%f, %f) with center offsets (%f, %f) at feed rate %f\n", x, y, i, j, f);

        //printf("currentPosition (%f,%f,%f) center (%f,%f,%f) angle %f InterpolationSteps %d\n",currentPosition.x,currentPosition.y,currentPosition.z,center.x,center.y,center.z,angle,InterpolationSteps);
        
        Coordinate* coordinates =  circularInterpolation(currentPosition,center,currentPlane,(float)(angle),InterpolationSteps);

        for(int i=0;i<InterpolationSteps;i++){
            printf("(%f,%f,%f),\n",coordinates[i].x, coordinates[i].y, coordinates[i].z + 280);
            currentPosition.x = coordinates[i].x;
            currentPosition.y = coordinates[i].y;
            currentPosition.z = coordinates[i].z;
        }
        
        delta_calcInverse(currentPosition.x,currentPosition.y,currentPosition.z, &currentAngles.theta1, &currentAngles.theta2, &currentAngles.theta3);
        //printf("currentPosition (%f,%f,%f) \n",currentPosition.x,currentPosition.y,currentPosition.z);
        /*
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
                    cJSON_AddItemToObject(stepObj, "timing", cJSON_CreateIntArray((int[]){f, f,5}, 3));
                    cJSON_AddItemToArray(jsonRoot, stepObj);

                    //printf("Interpolation Step %d: DeltaTheta1 = %d, DeltaTheta2 = %d, DeltaTheta3 = %d\n",
                    //    i, steps[i].Motor1, steps[i].Motor2, steps[i].Motor3);
                }
                // Update lastTheta values for next iteration
                lastTheta1 = theta1;
                lastTheta2 = theta2;
                lastTheta3 = theta3;
            } else {
                printf("Punkt existiert nicht");
            }
        }
        //currentPosition = targetPosition;
        //delta_calcInverse(targetPosition.x, targetPosition.y, targetPosition.z, &currentAngles.theta1, &currentAngles.theta2, &currentAngles.theta3);
        char* jsonString = cJSON_Print(jsonRoot);  // Generate final JSON string
        printf("%s\n", jsonString); 
        free(jsonString);
        cJSON_Delete(jsonRoot);

        free(coordinates);
        free(steps);
        */
        // Optionally handle feed rate f and center (i, j) here
    }
    else if (strcmp(command, "G4") == 0) {

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
        
    }
    else if (strcmp(command, "M100") == 0) {
        
    }
    else if (strcmp(command, "M200") == 0) {
        
    }
    else if (strcmp(command, "M300") == 0) {
        
    }
    else if (strcmp(command, "M400") == 0) {
        
    }
    else {
        printf("Unsupported command: %s\n", command);
    }
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