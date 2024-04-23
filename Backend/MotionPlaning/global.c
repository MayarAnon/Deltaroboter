#include "global.h"

Coordinate currentPosition = {0.0, 0.0, -280.0, 0.0};
Angles currentAngles = {-41.489, -41.489, -41.489};
Steps currentSteps = {0};
Plane currentPlane = XY_PLANE;
Gripper currentGripper = parallel;
int speedSetting = 50;
bool stopFlag = false;
double errorAccumulator1 = 0.0, errorAccumulator2 = 0.0, errorAccumulator3 = 0.0, errorAccumulator4 = 0.0;
