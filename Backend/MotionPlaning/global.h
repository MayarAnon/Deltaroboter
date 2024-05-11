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

typedef enum {
    RectangleProfil,
    TrapezProfil,
    SigmoidProfil,
    UnknownProfil = -1  // Hinzugefügter Wert für unbekannte Profile
} MotionProfile;

typedef struct {
    float x, y, z, phi, f, i, j, r, t;
} Parameter;

// Global MQTT Variables
extern const char** globalTopics;
extern int globalTopicCount;
extern void (*globalOnMessageCallback)(char *topicName, char *payloadStr);

// Definition und Initialisierung globaler Variablen, die den Zustand und die Konfiguration des Deltaroboters steuern.
extern MotionProfile currentMotionProfil;
extern Coordinate currentPosition;
extern Angles currentAngles;
extern Steps currentSteps;
extern Plane currentPlane;
extern Gripper currentGripper;
extern int speedSetting;
extern bool stopFlag;
extern bool timeFlagGripper;
extern bool homingFlag;
extern double errorAccumulator1, errorAccumulator2, errorAccumulator3, errorAccumulator4;
extern int currentGripperValue;
extern Parameter params;

#define STEPSPERREVOLUTION 800
#define GEARRATIO 20
#define PI 3.14159265358979323846

//subscribe Topics
#define ROBOTSTATETOPIC "robot/state"
#define LOADPROGRAMMTOPIC "pickandplace/program"
#define MANUELCONTROLCOORDINATESTOPIC "manual/control/coordinates"
#define MANUELCONTROLGRIPPERTOPIC "manual/control/gripper"
#define STOPTOPIC "motors/stop"

//publish Topics
#define MOTORCONTROLLTOPIC "motors/sequence"
#define GRIPPERCONTROLLTOPIC "gripper/control"
#define COORDINATESTOPIC "current/coordinates"
#define ANGLESTOPIC "current/angles"

//Mqtt defines
#define ADDRESS     "tcp://localhost:1883"
#define CLIENTID    "MotionPlaning"
#define QOS         1  // Set Quality of Service Level to 0 (At most once)

//MotionProfil Defines

#define INTERPOLATIONSTEPCUTOF 5
#define MINIMUMP2PCUTOF 10
#define RISEPERCENTAGE 0.15
#define CONSTSPEEDPERCENTAGE 0.7

#define START_PULSEWIDTH 530  // Definiert die Anfangspulsweite des Motors
#define ACCELERATION 12

#endif
