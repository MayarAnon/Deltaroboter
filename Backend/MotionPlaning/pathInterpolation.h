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
float calculateInitialAngle(Coordinate start, Coordinate center, Plane plane);
Coordinate* circularInterpolation(Coordinate start, Coordinate center, Plane plane, float angle, int steps);
Coordinate* linearInterpolation(Coordinate start, Coordinate end, int steps);

#endif