#ifndef UPDATEROBOTSTATE_H
#define UPDATEROBOTSTATE_H


#include "global.h"

// Function to parse the gripper mode from a string and return the corresponding enum
Gripper parseGripperMode(const char* mode);

// Function to parse the robot state from a JSON formatted string
void parseRobotState(const char *payloadStr);

#endif // UPDATEROBOTSTATE_H
