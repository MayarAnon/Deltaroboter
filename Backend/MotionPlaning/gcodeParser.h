#ifndef GCODEPARSER_H
#define GCODEPARSER_H

#include "global.h"    // Include global definitions for structs and enums


// Function prototypes for processing G-code lines

// Verarbeitet eine einzelne Zeile des G-Code-Befehls.
// Parameter:
//   - const char* line: Die Zeile des G-Codes, die verarbeitet werden soll.
void processLine(const char* line); 

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

#endif // GCODEPARSER_H
