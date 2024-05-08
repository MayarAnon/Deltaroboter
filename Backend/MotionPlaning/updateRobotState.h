#ifndef UPDATEROBOTSTATE_H
#define UPDATEROBOTSTATE_H


#include "global.h"

// Die Funktion `parseGripperMode` konvertiert einen String in einen entsprechenden Enum-Wert für Greifermodi.
// Parameter:
//   - const char* mode: Zeichenkette, die den Modus beschreibt (z.B. "parallelGripper")
// Rückgabewert:
//   - Gripper: Enum-Wert des Greifers, z.B. parallel, complient, magnet, vaccum. Bei unbekanntem Modus wird -1 zurückgegeben.
Gripper parseGripperMode(const char* mode);

// Die Funktion `parseMotionProfile` konvertiert einen String in einen entsprechenden Enum-Wert für Bewegungsprofile.
// Parameter:
//   - const char* profile: Zeichenkette, die das Bewegungsprofil beschreibt (z.B. "RectangleProfil")
// Rückgabewert:
//   - MotionProfile: Enum-Wert des Bewegungsprofils, z.B. RectangleProfil, TrapezProfil. Bei unbekanntem Profil wird -1 zurückgegeben.
MotionProfile parseMotionProfile(const char* profile);

// Die Funktion `parseRobotState` parst den Zustand eines Roboters aus einem JSON-String.
// Parameter:
//   - const char *payloadStr: JSON-String, der den Zustand des Roboters beschreibt
// Diese Funktion setzt globale Variablen basierend auf den geparsten Daten.
void parseRobotState(const char *payloadStr);

#endif // UPDATEROBOTSTATE_H
