#ifndef PATHINTERPOLATION_H
#define PATHINTERPOLATION_H

typedef struct {
    float x,y,z,phi;
} Coordinate;

typedef enum {
    XY_PLANE,
    YZ_PLANE,
    ZX_PLANE
} Plane;
Coordinate* circularInterpolation(Coordinate start, Coordinate end, Coordinate center, float radius, Plane plane, int direction, int *numSteps);
Coordinate* linearInterpolation(Coordinate start, Coordinate end, int steps);

#endif