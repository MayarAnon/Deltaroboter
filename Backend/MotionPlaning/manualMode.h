#ifndef MANUALMODE_H
#define MANUALMODE_H

#include "global.h"  //Globale Variablen einbinden


// Verarbeitet die Koordinaten aus einem JSON-String und führt gegebenenfalls eine Bewegung aus.
// Parameter:
//   - char *payloadStr: JSON-kodierter String, der Koordinaten enthält.
void manualModeCoordinates(char *payloadStr);

// Verarbeitet den Gripper-Wert aus einem JSON-String und führt gegebenenfalls eine Aktion aus.
// Parameter:
//   - char *payloadStr: JSON-kodierter String, der den Gripper-Wert enthält.
void manualModeGripper(char *payloadStr);

#endif 
