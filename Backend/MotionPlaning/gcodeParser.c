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



void processLine(char* line);

// Liest eine Datei und verarbeitet jede Zeile durch Aufruf der Funktion processLine.
// Parameter:
//   - const char* filename: Pfad der Datei, die gelesen werden soll.
void readFile(const char* filename) {
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    // Pfad zusammenbauen: Gehe einen Ordner hoch und dann in den Ordner GCodeFiles
    char path[1024];  // Pfadgröße anpassen, falls nötig
    snprintf(path, sizeof(path), "../GCodeFiles/%s", filename); // Baut den vollständigen Pfad zur Datei.
    
    FILE* file = fopen(path, "r");
    if (!file) {
        
        perror(path);
        return;
    }
    printf("Programm wird ausgeführt. \n");
    params =(Parameter){0.0,0.0,-280.0,0.0,0,0.0,0.0,0.0,0.0};
    currentCoordinateMode = Absolut;
    while ((read = getline(&line, &len, file)) != -1) {
        if (stopFlag) {
            printf("load Program wurde Abgebrochen");
            break;
        }
        
        processLine(line);
    }

    free(line);  // Wichtig, um den von getline zugewiesenen Speicher freizugeben
    fclose(file);
}


// Verarbeitet eine einzelne Zeile des G-Code-Befehls.
// Parameter:
//   - const char* line: Die Zeile des G-Codes, die verarbeitet werden soll.
void processLine(char* line) {
    char command[4];
    int numParams;

    // Überspringe die GCode-Nummern am Anfang, falls vorhanden.
    char* copyOfLine = line;
    if (sscanf(copyOfLine, "N%d", &(int){0}) == 1) {
        while (*copyOfLine && !isspace(*copyOfLine)) copyOfLine++;  // Überspringe die Nummer
        while (*copyOfLine && isspace(*copyOfLine)) copyOfLine++;  // Überspringe Leerzeichen
    }
    strcpy(line, copyOfLine);  // Überschreibe das Original mit dem bereinigten String
    
    // Extrahiere den Befehlstyp aus der Zeile
    numParams = sscanf(line, "%s", command);

    if (numParams < 1) {  //Kein Command erkannt
        return;
    }

    if (strcmp(command, "G0") == 0 || strcmp(command, "G00") == 0) {
        //Parameter aus string lesen 
        Coordinate* coordinates = (Coordinate*)malloc(2 * sizeof(Coordinate));
        
        coordinates[0] = currentPosition;
        //Unterscheiden zwischen absoluten und Relativen Kordinatensystem
        if(currentCoordinateMode == Absolut){
            for(const char *p = line; *p; ++p) {
                sscanf(p, "X%f", &params.x) || sscanf(p, "Y%f", &params.y) || sscanf(p, "Z%f", &params.z) || sscanf(p, "A%f", &params.phi) || sscanf(p, "F%f", &params.f);
            }
            coordinates[1] = (Coordinate){params.x,params.y,params.z,params.phi};
        }
        else if(currentCoordinateMode == Relativ){
            Parameter volitaleParams = {0};
            for(const char *p = line; *p; ++p) {
                sscanf(p, "X%f", &volitaleParams.x) || sscanf(p, "Y%f", &volitaleParams.y) || sscanf(p, "Z%f", &volitaleParams.z) || sscanf(p, "A%f", &volitaleParams.phi) || sscanf(p, "F%f", &volitaleParams.f);
            }
            coordinates[1] = (Coordinate){volitaleParams.x + currentPosition.x,volitaleParams.y + currentPosition.y,volitaleParams.z + currentPosition.z,volitaleParams.phi + currentPosition.phi};
        }
        
        processInterpolationAndCreateJSON(coordinates,2, params.f);
    
    }
    else if (strcmp(command, "G1") == 0 || strcmp(command, "G01") == 0) {
        
        
        float diffX = 0.0, diffY = 0.0,diffZ = 0.0;

        Coordinate targetPosition = {0.0, 0.0, -280.0, 0.0};

        if(currentCoordinateMode == Absolut){

            for(const char *p = line; *p; ++p) {
                sscanf(p, "X%f", &params.x) || sscanf(p, "Y%f", &params.y) || sscanf(p, "Z%f", &params.z) || sscanf(p, "A%f", &params.phi) || sscanf(p, "F%f", &params.f);
            }
            diffX = params.x - currentPosition.x;
            diffY = params.y - currentPosition.y;
            diffZ = params.z - currentPosition.z;

            targetPosition = (Coordinate){params.x,params.y,params.z,params.phi};
        }
        else if(currentCoordinateMode == Relativ){
            Parameter volitaleParams = {0};
            for(const char *p = line; *p; ++p) {
                sscanf(p, "X%f", &volitaleParams.x) || sscanf(p, "Y%f", &volitaleParams.y) || sscanf(p, "Z%f", &volitaleParams.z) || sscanf(p, "A%f", &volitaleParams.phi) || sscanf(p, "F%f", &volitaleParams.f);
            }
            diffX = volitaleParams.x;
            diffY = volitaleParams.y;
            diffZ = volitaleParams.z;

            targetPosition = (Coordinate){volitaleParams.x + currentPosition.x,volitaleParams.y + currentPosition.y,volitaleParams.z + currentPosition.z,volitaleParams.phi + currentPosition.phi};
        }
        else{
            printf("Fehler ist aufgetreten currentCoordinateMode konnte nicht gelesen werden! \n");
            fflush(stdout);
        }

        
        
        float distance = sqrt(diffX * diffX + diffY * diffY + diffZ * diffZ);

        int InterpolationSteps = (int)distance < 2 ? 2 : (int)distance;
    
        Coordinate* coordinates = linearInterpolation(currentPosition, targetPosition, InterpolationSteps);
        /*
        for(int i=0;i<InterpolationSteps;i++){
            printf("(%f,%f,%f),\n",coordinates[i].x, coordinates[i].y, coordinates[i].z +310);
            fflush(stdout);
        }
        */
        processInterpolationAndCreateJSON(coordinates,InterpolationSteps,params.f);
        
        
    }
    // Verarbeitet Kreisbewegungen gemäß den G-Code-Befehlen G2 (Kreis im Uhrzeigersinn) und G3 (Kreis gegen den Uhrzeigersinn).
    else if (strcmp(command, "G2") == 0 ||  strcmp(command, "G3" ) == 0 || strcmp(command, "G02") == 0 || strcmp(command, "G03") == 0) {
        // Bestimme die Drehrichtung basierend auf dem G-Code-Befehl
        int direction = 0;
        if(strcmp(command, "G2") == 0 || strcmp(command, "G02") == 0){
            direction = 1; // Uhrzeigersinn
        }
        else if(strcmp(command, "G3") == 0 || strcmp(command, "G03") == 0){
            direction = -1; // Gegen den Uhrzeigersinn
        }
        

        Coordinate center; 
        Coordinate end;

        params.i = params.j = params.r = 0.0;
        //Parameter X: X-Achse Y: Y-Achse Z: Z-Achese A: Rotationsachse Endeffektor F: Speed
        switch(currentPlane) {
        case XY_PLANE:
            if(currentCoordinateMode == Absolut){
                for(const char *p = line; *p; ++p) {
                    sscanf(p, "X%f", &params.x) || sscanf(p, "Y%f", &params.y) || sscanf(p, "I%f", &params.i) || sscanf(p, "J%f", &params.j) || sscanf(p, "R%f", &params.r)|| sscanf(p, "A%f", &params.phi) || sscanf(p, "F%f", &params.f);
                }
                center = (Coordinate){currentPosition.x + params.i,currentPosition.y + params.j,currentPosition.z,currentPosition.phi};
                end = (Coordinate){params.x,params.y,currentPosition.z,params.phi};
            }else if(currentCoordinateMode == Relativ){
                Parameter volitaleParams = {0};
                for(const char *p = line; *p; ++p) {
                    sscanf(p, "X%f", &volitaleParams.x) || sscanf(p, "Y%f", &volitaleParams.y) || sscanf(p, "I%f", &volitaleParams.i) || sscanf(p, "J%f", &volitaleParams.j) || sscanf(p, "R%f", &volitaleParams.r)|| sscanf(p, "A%f", &volitaleParams.phi) || sscanf(p, "F%f", &volitaleParams.f);
                }
                center = (Coordinate){currentPosition.x + volitaleParams.i,currentPosition.y + volitaleParams.j,currentPosition.z,currentPosition.phi};
                end = (Coordinate){volitaleParams.x + currentPosition.x,volitaleParams.y + currentPosition.y,currentPosition.z,volitaleParams.phi + currentPosition.phi};
            }

            break;
        case YZ_PLANE:
            
            if(currentCoordinateMode == Absolut){  
                for(const char *p = line; *p; ++p) {
                sscanf(p, "Y%f", &params.y) || sscanf(p, "Z%f", &params.z) || sscanf(p, "I%f", &params.i) || sscanf(p, "J%f", &params.j) || sscanf(p, "R%f", &params.r)|| sscanf(p, "A%f", &params.phi) || sscanf(p, "F%f", &params.f);
                }
                center = (Coordinate){currentPosition.x ,currentPosition.y + params.i,currentPosition.z + params.j,currentPosition.phi};
                end = (Coordinate){currentPosition.x,params.y,params.z,params.phi};
            }else if(currentCoordinateMode == Relativ){
                Parameter volitaleParams = {0};
                for(const char *p = line; *p; ++p) {
                sscanf(p, "Y%f", &volitaleParams.y) || sscanf(p, "Z%f", &volitaleParams.z) || sscanf(p, "I%f", &volitaleParams.i) || sscanf(p, "J%f", &volitaleParams.j) || sscanf(p, "R%f", &volitaleParams.r)|| sscanf(p, "A%f", &volitaleParams.phi) || sscanf(p, "F%f", &volitaleParams.f);
                }
                center = (Coordinate){currentPosition.x ,currentPosition.y + volitaleParams.i,currentPosition.z + volitaleParams.j,currentPosition.phi};
                end = (Coordinate){currentPosition.x,volitaleParams.y + currentPosition.y,volitaleParams.z + currentPosition.z,volitaleParams.phi + currentPosition.phi};
            }
            break;
        case ZX_PLANE:
            if(currentCoordinateMode == Absolut){
                for(const char *p = line; *p; ++p) {
                    sscanf(p, "Z%f", &params.z) || sscanf(p, "X%f", &params.x) || sscanf(p, "I%f", &params.i) || sscanf(p, "J%f", &params.j) || sscanf(p, "R%f", &params.r)|| sscanf(p, "A%f", &params.phi) || sscanf(p, "F%f", &params.f);
                }

                center = (Coordinate){currentPosition.x + params.j,currentPosition.y ,currentPosition.z + params.i,currentPosition.phi};
                end = (Coordinate){params.x,currentPosition.y,params.z,params.phi};
            }else if(currentCoordinateMode == Relativ){
                Parameter volitaleParams = {0};
                for(const char *p = line; *p; ++p) {
                    sscanf(p, "Z%f", &volitaleParams.z) || sscanf(p, "X%f", &volitaleParams.x) || sscanf(p, "I%f", &volitaleParams.i) || sscanf(p, "J%f", &volitaleParams.j) || sscanf(p, "R%f", &volitaleParams.r)|| sscanf(p, "A%f", &volitaleParams.phi) || sscanf(p, "F%f", &volitaleParams.f);
                }
                center = (Coordinate){currentPosition.x + volitaleParams.j,currentPosition.y ,currentPosition.z + volitaleParams.i,currentPosition.phi};
                end = (Coordinate){volitaleParams.x + currentPosition.x,currentPosition.y,volitaleParams.z + currentPosition.z,volitaleParams.phi + currentPosition.phi};
            }
            break;
        }
        
        int numSteps = 0;
        /*
        printf("currentPosition(%f,%f,%F)\n",currentPosition.x,currentPosition.y,currentPosition.z);
        printf("center(%f,%f,%F)\n",center.x,center.y,center.z);
        printf("end(%f,%f,%F)\n",end.x,end.y,end.z);
        printf("radius: %f \n",params.r);
        */
        Coordinate* coordinates = circularInterpolation(currentPosition, end, center,params.r, currentPlane,direction, &numSteps);
        /*
        for(int i=0;i<numSteps;i++){
            printf("(%f,%f,%f),\n",coordinates[i].x, coordinates[i].y, coordinates[i].z + 400);
        }
        */
        processInterpolationAndCreateJSON(coordinates,numSteps,params.f);
    }
    else if (strcmp(command, "G4") == 0 || strcmp(command, "G04") == 0) {
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
    else if (strcmp(command, "G90") == 0) {
        currentCoordinateMode = Absolut;
    }
    else if (strcmp(command, "G91") == 0) {
        currentCoordinateMode = Relativ;
    }
    // Ruft Unterprogramme auf, die in separaten Dateien definiert sind
    else if (strcmp(command, "M98") == 0) {
        
        char filename[1024];
        if (sscanf(line, "%*s %s", filename) == 1) {
            printf("Unterprogram %s wird Aufgerufen \n",filename);
            readFile(filename); // Ruft die Unterprogrammdatei auf
        } else {
            printf("Syntaxfehler in M98 Befehl, kein Dateiname angegeben.\n");
        }
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
    else if (strcmp(command, "M500") == 0) {
        //Magnet Wechselsystem
        char parameter[100];
        // Versuche, den Parameter aus dem Befehlsstring zu extrahieren
        if (sscanf(line, "%*s %s", parameter) == 1) {
            // Prüfe, ob der Parameter "enabled" oder "disabled" ist
            if (strcmp(parameter, "enable") == 0 || strcmp(parameter, "disable") == 0) {
                // Veröffentliche den Parameter unter "magnet/control"
                char formattedMessage[100];
                sprintf(formattedMessage, "{\"magneticGripperAttachment\":\"%s\"}", parameter); 
                publishMessage(GRIPPERCONTROLLTOPIC, formattedMessage);
            } else {
                printf("Ungültiger Parameter: %s. Erwarte 'enable' oder 'disable'.\n", parameter);
            }
        } else {
            printf("Kein Parameter gefunden in der Eingabe.\n");
        }
    }
    else if (strcmp(command, "M600") == 0) {
       char grippertyp[100];
        Gripper gripper = unknown;
        
        if (sscanf(line, "%*s %s", grippertyp) == 1) {
            //printf("%s \n",grippertyp);
            // Direkte Prüfung des Gripper-Modus ohne separate Funktion
            if (strcmp(grippertyp, "parallelGripper") == 0) gripper = parallel;
            else if (strcmp(grippertyp, "complientGripper") == 0) gripper = complient;
            else if (strcmp(grippertyp, "magnetGripper") == 0) gripper = magnet;
            else if (strcmp(grippertyp, "vacuumGripper") == 0) gripper = vaccum;
            else fprintf(stderr, "Unknown gripper mode: %s\n", grippertyp);

            if (gripper != unknown) {
                // Setzen des aktuellen Greifertyps auf den gelesenen Wert
                // Hier könnten weitere Aktionen durchgeführt werden
                char formattedMessage[120];
                sprintf(formattedMessage, "\"%s\"", grippertyp); 
                publishMessage(GRIPPERMODETOPIC, formattedMessage);
                currentGripper = gripper;
            } else {
                printf("Kein bekannter Grippertyp: %s\n", grippertyp);
            }
        } else {
            printf("Kein Grippertyp in der Eingabe gefunden.\n");
        }
    }
    else if (strcmp(command, ";") == 0) {
        
    }
    else {
        printf("Unsupported command: %s\n", command);
    }
}



