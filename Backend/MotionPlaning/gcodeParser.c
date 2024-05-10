#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "pathInterpolation.h"
#include "cJSON.h"
#include "mqttClient.h"
#include <ctype.h>
#include <stdbool.h>
#include "calcMotion.h"
#include "global.h"





// Verarbeitet eine einzelne Zeile des G-Code-Befehls.
// Parameter:
//   - const char* line: Die Zeile des G-Codes, die verarbeitet werden soll.
void processLine(const char* line) {
    char command[4];
    float x, y,z,phi, i, j, f,t,r;
    int numParams;

    // Initialisiere Parameter mit Standardwerten
    x = y  = i = j = phi = t = r = 0.0;
    f = 75;
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
        sscanf(p, "X%f", &params.x) || sscanf(p, "Y%f", &params.y) || sscanf(p, "Z%f", &params.z) || sscanf(p, "A%f", &params.phi) || sscanf(p, "F%f", &params.f);
        }
        
        coordinates[0] = currentPosition;
        coordinates[1] = (Coordinate){params.x,params.y,params.z,params.phi};

        processInterpolationAndCreateJSON(coordinates,2, params.f);
    
    }
    else if (strcmp(command, "G1") == 0) {
        //Parameter aus string lesen 
        for(const char *p = line; *p; ++p) {
        sscanf(p, "X%f", &params.x) || sscanf(p, "Y%f", &params.y) || sscanf(p, "Z%f", &params.z) || sscanf(p, "A%f", &params.phi) || sscanf(p, "F%f", &params.f);
        }
        
        float diffX = fabs(params.x - currentPosition.x);
        float diffY = fabs(params.y - currentPosition.y);
        float diffZ = fabs(params.z - currentPosition.z);

        Coordinate targetPosition = {params.x,params.y,params.z,params.phi};
        
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
        processInterpolationAndCreateJSON(coordinates,InterpolationSteps,params.f);
        
        
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
        

        Coordinate center; 
        Coordinate end;
        //Parameter X: X-Achse Y: Y-Achse Z: Z-Achese A: Rotationsachse Endeffektor F: Speed
        switch(currentPlane) {
        case XY_PLANE:
            
            for(const char *p = line; *p; ++p) {
            sscanf(p, "X%f", &params.x) || sscanf(p, "Y%f", &params.y) || sscanf(p, "I%f", &params.i) || sscanf(p, "J%f", &params.j) || sscanf(p, "R%f", &params.r)|| sscanf(p, "A%f", &params.phi) || sscanf(p, "F%f", &params.f);
            }
            center = (Coordinate){currentPosition.x + params.i,currentPosition.y + params.j,currentPosition.z,currentPosition.phi};
            end = (Coordinate){params.x,params.y,currentPosition.z,params.phi};
            int numSteps = 0;
            

            break;
        case YZ_PLANE:
            for(const char *p = line; *p; ++p) {
            sscanf(p, "Y%f", &params.y) || sscanf(p, "Z%f", &params.z) || sscanf(p, "I%f", &params.i) || sscanf(p, "J%f", &params.j) || sscanf(p, "R%f", &params.r)|| sscanf(p, "A%f", &params.phi) || sscanf(p, "F%f", &params.f);
            }
            center = (Coordinate){currentPosition.x ,currentPosition.y + params.i,currentPosition.z + params.j,currentPosition.phi};
            end = (Coordinate){currentPosition.x,params.y,params.z,params.phi};
            break;
        case ZX_PLANE:
            
            for(const char *p = line; *p; ++p) {
            sscanf(p, "Z%f", &params.z) || sscanf(p, "X%f", &params.x) || sscanf(p, "I%f", &params.i) || sscanf(p, "J%f", &params.j) || sscanf(p, "R%f", &params.r)|| sscanf(p, "A%f", &params.phi) || sscanf(p, "F%f", &params.f);
            }
            center = (Coordinate){currentPosition.x + params.j,currentPosition.y ,currentPosition.z + params.i,currentPosition.phi};
            end = (Coordinate){params.x,currentPosition.y,params.z,params.phi};
            break;
        }
        
        int numSteps = 0;
        
        Coordinate* coordinates = circularInterpolation(currentPosition, end, center,params.r, currentPlane,direction, &numSteps);
        processInterpolationAndCreateJSON(coordinates,numSteps,params.f);
    }
    else if (strcmp(command, "G4") == 0) {
        int numParams = sscanf(line, "%*s P%f", &params.t);

        if (numParams < 1) {
            fprintf(stderr, "Failed to read sleep time for G4 command\n");
            return;
        } else {
            usleep(params.t * 1000);  // Perform the sleep
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

        for(const char *p = line; *p; ++p) {
            sscanf(p, "F%f", &params.f);
        }
    
        targetPosition[0] = currentPosition;
        
        // Korrigiere die Definition des zweiten Elements des Arrays
        targetPosition[1] = (Coordinate){0.0, 0.0, -280, 0.0};
            
        processInterpolationAndCreateJSON(targetPosition, 2, params.f);
    
    }
    else if (strcmp(command, "M100") == 0) {
        if(currentGripper == parallel){
                processGripperCommand("M100", line); // Befehl und Parameter
        }else{
            printf("wrong Gripper Typ \n");
        }
    }
    else if (strcmp(command, "M200") == 0) {
        if(currentGripper == complient){
                processGripperCommand("M200", line); // Befehl und Parameter
        }else{
            printf("wrong Gripper Typ \n");
        }
    }
    else if (strcmp(command, "M300") == 0) {
        if(currentGripper == magnet){
                processGripperCommand("M300", line); // Befehl und Parameter
        }else{
            printf("wrong Gripper Typ \n");
        }
    }
    else if (strcmp(command, "M400") == 0) {
        if(currentGripper == vaccum){
                processGripperCommand("M400", line); // Befehl und Parameter
        }else{
            printf("wrong Gripper Typ \n");
        }
    }
    else if (strcmp(command, ";") == 0) {
        
    }
    else {
        printf("Unsupported command: %s\n", command);
    }
}



