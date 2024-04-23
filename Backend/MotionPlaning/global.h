#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdbool.h>

typedef struct {
    float theta1, theta2, theta3;
} Angles;

typedef struct {
    int Motor1, Motor2, Motor3, Motor4;
} Steps;

typedef struct {
    float x, y, z, phi;
} Coordinate;

typedef enum {
    parallel,
    complient,
    magnet,
    vaccum
} Gripper;

typedef enum {
    XY_PLANE,
    YZ_PLANE,
    ZX_PLANE
} Plane;

// Definition und Initialisierung globaler Variablen, die den Zustand und die Konfiguration des Deltaroboters steuern.
extern Coordinate currentPosition;
extern Angles currentAngles;
extern Steps currentSteps;
extern Plane currentPlane;
extern Gripper currentGripper;
extern int speedSetting;
extern bool stopFlag;
extern double errorAccumulator1, errorAccumulator2, errorAccumulator3, errorAccumulator4;

#define STEPSPERREVOLUTION 800
#define GEARRATIO 20
#define PI 3.14159265358979323846

//subscribe Topics
#define ROBOTSTATETOPIC "robot/state"
#define LOADPROGRAMMTOPIC "pickandplace/program"
#define MANUELCONTROLTOPIC "manual/control"
#define STOPTOPIC "motors/stop"
//publish Topics
#define MOTORCONTROLLTOPIC "motors/sequence"
#define GRIPPERCONTROLLTOPIC "gripper/control"
#define COORDINATESTOPIC "current/coordinates"
#define ANGLESTOPIC "current/angles"

//Mqtt defines
#define ADDRESS     "tcp://localhost:1883"
#define CLIENTID    "MotionPlaning"
#define QOS         0  // Set Quality of Service Level to 0 (At most once)

#endif
