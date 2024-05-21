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

void publishCurrentState(Coordinate pos, Angles ang);


// Funktion zur Berechnung der Pulsweite basierend auf dem aktuellen Schritt
int calculateTrapezoidalPulsewidth(int base, int step, int total, int accel) {
    int change = abs(START_PULSEWIDTH - base); // Berechnet die notwendige Änderung der Pulsweite
    int stepsNeeded = change / accel; // Ermittelt, wie viele Schritte nötig sind, um die Änderung durchzuführen
    int rampUp = (total > 2 * stepsNeeded) ? stepsNeeded : total / 2; // Bestimmt die Dauer der Beschleunigungsphase
    int rampDown = total - rampUp; // Beginn der Verzögerungsphase

    // Berechnet die Pulsweite basierend auf der aktuellen Phase des Bewegungsprofils
    if (step < rampUp) {
        // Beschleunigungsphase: Pulsweite verringert sich mit jedem Schritt
        return START_PULSEWIDTH - step * accel;
    } else if (step >= rampDown) {
        // Verzögerungsphase: Pulsweite beginnt sich wieder zu erhöhen
        return START_PULSEWIDTH - (total - step) * accel;
    } else {
        // Konstante Phase: Pulsweite bleibt auf dem Basiswert
        return base;
    }
}


// Funktion zur Berechnung der Pulsweite für zwei hintereinander folgende Sigmoid-Kurven
int calculateModifiedSigmoidPulsewidth(int basePulsewidth, int currentStep, int totalSteps, int maxAcceleration) {
    int change = abs(START_PULSEWIDTH - basePulsewidth);
    int stepsNeeded = change / maxAcceleration;  // Berechnet, wie viele Schritte für die max. Beschleunigung benötigt werden
    if (totalSteps < 2 * stepsNeeded) {
        // Reduziere den Ziel-Pulsweiten-Wechsel, wenn nicht genug Schritte vorhanden sind
        change = (totalSteps / 2) * maxAcceleration;
        basePulsewidth = START_PULSEWIDTH - change;
        stepsNeeded = totalSteps / 2;
    }
    int rampUp = stepsNeeded;
    int rampDown = totalSteps - stepsNeeded;
    int constantPhaseStart = rampUp;
    int constantPhaseEnd = rampDown;

    float k = 10.0 / rampUp;  // Skalierungsfaktor für die Steilheit der S-Kurve
    float t0 = rampUp / 2.0;  // Mittelpunkt der S-Kurve für die Beschleunigungsphase

    if (currentStep < rampUp) {
        // Beschleunigungsphase
        return START_PULSEWIDTH - (int)((START_PULSEWIDTH - basePulsewidth) / (1.0 + exp(-k * (currentStep - t0))));
    } else if (currentStep >= constantPhaseStart && currentStep < constantPhaseEnd) {
        // Konstante Phase
        return basePulsewidth;
    } else if (currentStep >= constantPhaseEnd) {
        // Verzögerungsphase
        float t1 = (totalSteps - rampDown) / 2.0;
        return basePulsewidth + (int)((START_PULSEWIDTH - basePulsewidth) / (1.0 + exp(-k * (currentStep - constantPhaseEnd - t1))));
    }
    return basePulsewidth;  // Sicheres Rückgabeverhalten, falls außerhalb der definierten Bereiche
}

// Verarbeitet die Interpolation erzeugt eine entsprechende JSON-Nachricht und publisht diese an MotorControll.
// Parameter:
//   - Coordinate* coordinates: Array von Koordinaten für die Interpolation.
//   - int InterpolationSteps: Anzahl der Schritte in einer Interpolation.
//   - float f: Geschwindigkeitsfaktor
void processInterpolationAndCreateJSON(Coordinate* coordinates, int InterpolationSteps, float f) {
    // Reserviert Speicher für die Interpolationschritte
    Steps* steps = malloc(InterpolationSteps * sizeof(Steps));
    // Lokale Kopie der aktuellen Position
    Coordinate localPosition = currentPosition;  
    
    // Berechnet Winkel basierend auf der aktuellen Position damit diese Richtig hinterlegt sind
    delta_calcInverse(localPosition.x, localPosition.y, localPosition.z, &currentAngles.theta1, &currentAngles.theta2, &currentAngles.theta3); //Winkel aktualisieren

    // Lokale Kopien der aktuellen Winkel und Schrittzahlen
    Angles localAngles = currentAngles;       
    Steps localSteps = currentSteps; 
    // Fehlerakkumulatoren für jeden Motor         
    double localErrorAccumulators[4] = {errorAccumulator1, errorAccumulator2, errorAccumulator3, errorAccumulator4};
    
    // Berechnet die maximale Geschwindigkeit basierend auf dem Faktor 'f'
    int maxSpeed = 530 - 5 * f;
    maxSpeed = maxSpeed < 15 ? 15 : maxSpeed;
    // Initialisiert die Pulsbreite 
    int pulsewidth = 530;

    // Erzeugt ein JSON Array zum Speichern der Motorsequenzen
    cJSON* jsonRoot = cJSON_CreateArray();
    long long totalDuration = 0;
    bool errorOccurred = false;

     // Iteriert über die Interpolationsschritte
    for (int i = 0; i < InterpolationSteps; i++) {

        // Anpassung der Pulsbreite für ein Trapezprofil
        if (currentMotionProfil == TrapezProfil && InterpolationSteps > INTERPOLATIONSTEPCUTOF) {
            pulsewidth = calculateTrapezoidalPulsewidth(maxSpeed, i, InterpolationSteps,ACCELERATION);
        }else if(currentMotionProfil == SigmoidProfil && InterpolationSteps > INTERPOLATIONSTEPCUTOF){
            pulsewidth = calculateModifiedSigmoidPulsewidth(maxSpeed, i, InterpolationSteps,ACCELERATION);
        }else {
            pulsewidth = maxSpeed;
        }
        float theta1, theta2, theta3;

        // Setzt lokale Position auf die aktuellen Koordinaten
        localPosition.x = coordinates[i].x;
        localPosition.y = coordinates[i].y;
        localPosition.z = coordinates[i].z;
    
        // Berechnen & Überprüft die Machbarkeit der neuen Position
        if (delta_calcInverse(localPosition.x, localPosition.y, localPosition.z, &theta1, &theta2, &theta3) == 0) {
            // Berechnet die Schrittzahlen für die Motoren
            double stepCalc1 = ((theta1 - localAngles.theta1)/360) * STEPSPERREVOLUTION * GEARRATIO + localErrorAccumulators[0];
            double stepCalc2 = ((theta2 - localAngles.theta2)/360) * STEPSPERREVOLUTION * GEARRATIO + localErrorAccumulators[1];
            double stepCalc3 = ((theta3 - localAngles.theta3)/360) * STEPSPERREVOLUTION * GEARRATIO + localErrorAccumulators[2];
            double stepCalc4 = ((coordinates[i].phi - localPosition.phi)/360) * STEPSPERREVOLUTION + localErrorAccumulators[3];

            // Speichert und summiert Schritte für jede Achse
            steps[i].Motor1 = round(stepCalc1);
            steps[i].Motor2 = round(stepCalc2);
            steps[i].Motor3 = round(stepCalc3);
            steps[i].Motor4 = round(stepCalc4);

            localSteps.Motor1 += steps[i].Motor1;
            localSteps.Motor2 += steps[i].Motor2;
            localSteps.Motor3 += steps[i].Motor3;
            localSteps.Motor4 += steps[i].Motor4;

            // Aktualisiert Fehlerakkumulatoren und lokale Winkel
            localErrorAccumulators[0] = stepCalc1 - steps[i].Motor1;
            localErrorAccumulators[1] = stepCalc2 - steps[i].Motor2;
            localErrorAccumulators[2] = stepCalc3 - steps[i].Motor3;
            localErrorAccumulators[3] = stepCalc4 - steps[i].Motor4;
            localAngles.theta1 = theta1;
            localAngles.theta2 = theta2;
            localAngles.theta3 = theta3;
            localPosition.phi = coordinates[i].phi;

            int pulses[4] = {steps[i].Motor1, steps[i].Motor2, steps[i].Motor3, steps[i].Motor4};
            int maxSteps = fmax(fmax(abs(steps[i].Motor1), abs(steps[i].Motor2)), fmax(abs(steps[i].Motor3), abs(steps[i].Motor4)));

            // Point to Point Verfahren 2 Nachrichten und maxStep größer 50 und Trapezprofil
            if (InterpolationSteps == 2 && maxSteps > MINIMUMP2PCUTOF && (currentMotionProfil == TrapezProfil || currentMotionProfil == SigmoidProfil)) {
                // Aufteilen in 20 Nachrichten, genaue Berechnung der Schritte
                //distanz Berechnen zwischen Punkten 
                float distance = sqrt(pow(coordinates[1].x - coordinates[0].x, 2) + pow(coordinates[1].y - coordinates[0].y, 2) + pow(coordinates[1].z - coordinates[0].z, 2));
                int devision = (int)distance < 2 ? 2 : (int)distance;
                // Zum Speichern der summierten Schritte für Genauigkeitsüberprüfung
                int totalSteps[4] = {0, 0, 0, 0};  

                //Durchiterieren durch die Unterteilungen 
                for (int i = 0; i < devision; i++) {
                    int currentPulses[4];
                    //Durchiterieren durch die Motorpulse
                    for (int j = 0; j < 4; j++) { 
                        //Pulse von Berechnungen übernehmen 
                        int originalPulses = pulses[j];
                         // Füge alle verbleibenden Schritte zur letzten Nachricht hinzu
                        if (i == devision - 1) {
                            currentPulses[j] = pulses[j];
                        } else {
                            // Gleichmäßige Aufteilung der verbleibenden Schritte
                            currentPulses[j] = (int)((double)(originalPulses) / (devision - i));  
                        }
                        //Abziehen Pulse
                        pulses[j] -= currentPulses[j];
                        totalSteps[j] += currentPulses[j];
                    }

                    // Berechnen der Pulsweite für diese Nachricht gemäß Trapezprofil oder Sigmoid
                    int messagePulsewidth = 530;
                    if (currentMotionProfil == TrapezProfil) {
                        messagePulsewidth = calculateTrapezoidalPulsewidth(maxSpeed, i, devision,ACCELERATION);
                    }else if(currentMotionProfil == SigmoidProfil){
                        messagePulsewidth = calculateModifiedSigmoidPulsewidth(maxSpeed, i, devision,ACCELERATION);
                    }

                    // Berechnet die maximale Anzahl von Schritten für das Stopen des Programmes für die Ausführungszeit
                    int maxSteps = fmax(fmax(abs(currentPulses[0]), abs(currentPulses[1])), fmax(abs(currentPulses[2]), abs(currentPulses[3])));
                    long long stepDuration = (long long)(maxSteps * 2 * messagePulsewidth);
                    totalDuration += stepDuration;

                    //Wenn Pulse nicht alle 0 dann hinzufügen zu JSON 
                    if (currentPulses[0] != 0 || currentPulses[1] != 0 || currentPulses[2] != 0 || currentPulses[3] != 0) {
                        cJSON* stepObj = cJSON_CreateObject();
                        cJSON_AddItemToObject(stepObj, "motorpulses", cJSON_CreateIntArray(currentPulses, 4));
                        cJSON_AddItemToObject(stepObj, "timing", cJSON_CreateIntArray((int[]){messagePulsewidth, messagePulsewidth, 5}, 3));
                        cJSON_AddItemToArray(jsonRoot, stepObj);
                    }
                }
                
            }else {
                // Bestehende Logik für normale Interpolation oder andere Profile

                // Überprüfung, ob eine Nachricht aufgeteilt werden muss
                int splitCount[4];
                int pulses[4] = {steps[i].Motor1, steps[i].Motor2, steps[i].Motor3, steps[i].Motor4};
                // Berechnet, wie viele Nachrichten nötig sind Maximale Anzahl an Pulsen pro Nachricht 5000
                for (int j = 0; j < 4; j++) {
                    //wenn unter 5000 ist dann splitCount = 1
                    splitCount[j] = (abs(pulses[j]) + 4999) / 5000; 
                }
                
                int maxSplit = fmax(fmax(splitCount[0], splitCount[1]), fmax(splitCount[2], splitCount[3]));
                //Pulse werden auf die Anzahl an Splits gleichmäßig aufgeteilt 
                for (int k = 0; k < maxSplit; k++) {
                    int currentPulses[4];
                    for (int j = 0; j < 4; j++) {
                        currentPulses[j] = pulses[j] / splitCount[j];
                        pulses[j] -= currentPulses[j];
                        splitCount[j]--;
                    }

                    // Berechnet die maximale Anzahl von Schritten für das Stopen des Programmes für die Ausführungszeit
                    int maxSteps = fmax(fmax(abs(currentPulses[0]), abs(currentPulses[1])), fmax(abs(currentPulses[2]), abs(currentPulses[3])));
                    long long stepDuration = (long long)(maxSteps * 2 * pulsewidth);
                    totalDuration += stepDuration;

                    if (currentPulses[0] != 0 || currentPulses[1] != 0 || currentPulses[2] != 0 || currentPulses[3] != 0) {
                        cJSON* stepObj = cJSON_CreateObject();
                        cJSON_AddItemToObject(stepObj, "motorpulses", cJSON_CreateIntArray(currentPulses, 4));
                        cJSON_AddItemToObject(stepObj, "timing", cJSON_CreateIntArray((int[]){(int)pulsewidth, (int)pulsewidth, 5}, 3));
                        cJSON_AddItemToArray(jsonRoot, stepObj);
                    }
                }
            }
        } else {
            // Wird ausgeführt wenn delta_calcInverse einen Fehler zurückgibt da der Wert nicht erreicht werden kann mit der Kinematik
            errorOccurred = true;
            printf("Punkt existiert nicht (%f,%f,%f),\n", coordinates[i].x, coordinates[i].y, coordinates[i].z);
            fflush(stdout); 
            break;
        }
    }

     // Verarbeitet die erfolgreiche Durchführung und sendet die Daten
    if (!errorOccurred) {
        currentPosition = localPosition;
        currentAngles = localAngles;
        currentSteps = localSteps;
        errorAccumulator1 = localErrorAccumulators[0];
        errorAccumulator2 = localErrorAccumulators[1];
        errorAccumulator3 = localErrorAccumulators[2];
        errorAccumulator4 = localErrorAccumulators[3];
        
        char* jsonString = cJSON_Print(jsonRoot);
        //printf(jsonString); 
        if(currentPowerstageMode == On){
            publishMessage("motors/sequence", jsonString);
        }
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
    snprintf(coordString, sizeof(coordString), "[%f, %f, %f, %f]", pos.x, pos.y, pos.z,pos.phi);
    snprintf(anglesString, sizeof(anglesString), "[%f, %f, %f]", ang.theta1, ang.theta2, ang.theta3);
    publishMessage("current/coordinates", coordString);
    publishMessage("current/angles", anglesString);
}

void processGripperCommand(char* command, const char* line) {
    if(currentPowerstageMode == On){
    
        int sValue = 0;
        int numParams = sscanf(line, "%*s S%d", &sValue);
        bool validValue = false;

        // Integrierte Validierung basierend auf dem aktuellen Greifertyp
        switch (currentGripper) {
            case parallel:
                validValue = (sValue >= 0 && sValue <= 100);
                break;
            case complient:
                validValue = (sValue == 1 || sValue == 0 || sValue == -1);
                break;
            case magnet:
                validValue = (sValue == 1 || sValue == 0);
                break;
            case vaccum:
                validValue = (sValue == 1 || sValue == 0 || sValue == -1);
                break;
            default:
                validValue = false;  // Sicherstellen, dass kein ungültiger Greifertyp verwendet wird
                break;
        }

        if (numParams < 1 || !validValue) {
            fprintf(stderr, "Invalid parameter for command %s\n", command);
            return;
        }

        // Erstellen des JSON-Strings basierend auf dem aktuellen Greifer
        int waitTime = 0;
        char jsonString[100];
        switch (currentGripper) {
            case parallel:
                snprintf(jsonString, sizeof(jsonString), "{\n\"parallelGripper\": %d\n}", sValue);
                waitTime = 5000;
                break;
            case complient:
                snprintf(jsonString, sizeof(jsonString), "{\n\"complientGripper\": %d\n}", sValue);
                waitTime = 2000;
                break;
            case magnet:
                snprintf(jsonString, sizeof(jsonString), "{\n\"magnetGripper\": %d\n}", sValue);
                waitTime = 500;
                break;
            case vaccum:
                snprintf(jsonString, sizeof(jsonString), "{\n\"vacuumGripper\": %d\n}", sValue);
                waitTime = 2000;
                break;
        }

        publishMessage(GRIPPERCONTROLLTOPIC, jsonString); // Nachricht senden
        usleep(waitTime * 1000);
        currentGripperValue = sValue; // Aktualisieren des aktuellen Greiferwertes
    }
}
