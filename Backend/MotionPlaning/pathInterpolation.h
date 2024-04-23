#ifndef PATHINTERPOLATION_H
#define PATHINTERPOLATION_H
#include "global.h"

Coordinate* circularInterpolation(Coordinate start, Coordinate end, Coordinate center, float radius, Plane plane, int direction, int *numSteps);
Coordinate* linearInterpolation(Coordinate start, Coordinate end, int steps);

#endif