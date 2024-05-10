#ifndef CALC_MOTION_H
#define CALC_MOTION_H

#include "inverseKinematic.h"
#include "mqttClient.h"
#include "global.h"

// Declares a function to calculate the pulse width for a trapezoidal profile.
int calculateTrapezoidalPulsewidth(int basePulsewidth, int currentStep, int totalSteps);

// Verarbeitet die Interpolation erzeugt eine entsprechende JSON-Nachricht und publisht diese an MotorControll.
// Parameter:
//   - Coordinate* coordinates: Array von Koordinaten für die Interpolation.
//   - int InterpolationSteps: Anzahl der Schritte in einer Interpolation.
//   - float f: Geschwindigkeitsfaktor
void processInterpolationAndCreateJSON(Coordinate* coordinates, int InterpolationSteps, float f);

// Veröffentlicht den aktuellen Zustand der Koordinaten und Winkel des Roboters.
// Parameter:
//   - Coordinate pos: Die aktuellen Koordinaten.
//   - Angles ang: Die aktuellen Winkel.
void publishCurrentState(Coordinate pos, Angles ang); // Publishes the current state of the robot


void processGripperCommand(char* command, const char* line);

#endif 
