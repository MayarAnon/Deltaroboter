#ifndef MANUALMODE_H
#define MANUALMODE_H

#include "global.h"  //Globale Variablen einbinden


// Die Funktion `manualMode` verarbeitet JSON-formatierte Steuerbefehle für die manuelle Steuerung des Deltaroboters.
// Parameter:
//   - char *payloadStr: JSON-kodierter String, der Koordinaten und Greiferbefehle enthält.
void manualMode(char *payloadStr);

#endif 
