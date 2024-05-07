#include "global.h"

// Definition und Initialisierung globaler Variablen, die den Zustand und die Konfiguration des Deltaroboters steuern.


// `currentPosition` speichert die aktuelle Position
Coordinate currentPosition = {0.0, 0.0, -280.0, 0.0}; 

// `currentAngles` speichert die aktuellen Winkel der Roboterarme.
Angles currentAngles = {-41.489, -41.489, -41.489};
// `currentSteps` speichert die aktuellen Schritte der Antriebsmotoren des Roboters.
Steps currentSteps = {0};

// `currentPlane` definiert die aktuelle Arbeitsebene des Roboters.
Plane currentPlane = XY_PLANE;

// `currentGripper` gibt den aktuellen Greifertyp
Gripper currentGripper = parallel;

// `currentGripperValue` beschreibt den aktuellen Wert von Gripper
int currentGripperValue = 0;

// `speedSetting` definiert die Geschwindigkeitseinstellung
int speedSetting = 50;

// `stopFlag` ist eine Boolesche Variable, die verwendet wird, um den Roboter zu stoppen
bool stopFlag = false;

// `timeFlagGripper` ist eine Boolesche Variable, die verwendet wird, um auf GripperControl in der Ausführung zu warten.
bool timeFlagGripper = true;
// 'homingFlag' ist eine Boolesche Variable, die für die Flankenerkennung von Homingsignalen in robot/state zuständig ist verwendet in updateRobotState
bool homingFlag = false;

// `errorAccumulator1` bis `errorAccumulator4` dienen der Fehlerakkumulation für Regelungszwecke.
double errorAccumulator1 = 0.0, errorAccumulator2 = 0.0, errorAccumulator3 = 0.0, errorAccumulator4 = 0.0;
