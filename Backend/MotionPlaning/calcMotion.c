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

// Funktion zur Berechnung der Pulsweite für das Trapezprofil
int calculateTrapezoidalPulsewidth(int basePulsewidth, int currentStep, int totalSteps) {
    int rampUpSteps = totalSteps * RISEPERCENTAGE;   // 25% der Schritte für das Heruntermodulieren
    int constantSteps = totalSteps * CONSTSPEEDPERCENTAGE;  // 50% der Schritte konstant
    int rampDownSteps = totalSteps - rampUpSteps - constantSteps;  // 25% der Schritte für das Hochmodulieren

    // Startet mit dem höchsten Wert (530) und moduliert herunter auf basePulsewidth
    int startPulsewidth = 530;
    
    if (currentStep < rampUpSteps) {
        // Ramp-down Phase (Modulierung herunter)
        return startPulsewidth - (int)((startPulsewidth - basePulsewidth) * (currentStep / (float)rampUpSteps));
    } else if (currentStep < (rampUpSteps + constantSteps)) {
        // Konstante Phase
        return basePulsewidth;
    } else {
        // Ramp-up Phase (Modulierung hoch)
        return basePulsewidth + (int)((startPulsewidth - basePulsewidth) * ((currentStep - rampUpSteps - constantSteps) / (float)rampDownSteps));
    }
}

// Funktion zur Berechnung der Pulsweite für zwei hintereinander folgende Sigmoid-Kurven
int calculateSigmoidPulsewidth(int maxPulsewidth, int currentStep, int totalSteps) {
    int startPulsewidth =  530;
    int midPoint = totalSteps / 2;  // Mittelpunkt, teilt die Schritte in zwei Hälften
    float k = 10.0 / midPoint;  // Skalierungsfaktor für die Steilheit der S-Kurve
    float t0 = midPoint / 2.0;  // Mittelpunkt der S-Kurve für jede Phase

    if (currentStep <= midPoint) {
        // Erste Hälfte: Anstieg von startPulsewidth zu maxPulsewidth
        return startPulsewidth + (int)((maxPulsewidth - startPulsewidth) / (1.0 + exp(-k * (currentStep - t0))));
    } else {
        // Zweite Hälfte: Abfall von maxPulsewidth zu startPulsewidth
        return maxPulsewidth - (int)((maxPulsewidth - startPulsewidth) / (1.0 + exp(-k * (currentStep - midPoint - t0))));
    }
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
            pulsewidth = calculateTrapezoidalPulsewidth(maxSpeed, i, InterpolationSteps);
        }else if(currentMotionProfil == SigmoidProfil && InterpolationSteps > INTERPOLATIONSTEPCUTOF){
            pulsewidth = calculateSigmoidPulsewidth(maxSpeed, i, InterpolationSteps);
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
                int devision = P2PINTERPOLATIONSTEPS;
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
                        messagePulsewidth = calculateTrapezoidalPulsewidth(maxSpeed, i, devision);
                    }else if(currentMotionProfil == SigmoidProfil){
                        messagePulsewidth = calculateSigmoidPulsewidth(maxSpeed, i, devision);
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
    snprintf(coordString, sizeof(coordString), "[%f, %f, %f, %f]", pos.x, pos.y, pos.z,pos.phi);
    snprintf(anglesString, sizeof(anglesString), "[%f, %f, %f]", ang.theta1, ang.theta2, ang.theta3);
    publishMessage("current/coordinates", coordString);
    publishMessage("current/angles", anglesString);
}