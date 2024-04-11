#ifndef PATHINTERPOLATION_H
#define PATHINTERPOLATION_H

typedef struct {
    float x, y, z;
} Coordinate;

typedef enum {
    XY_PLANE,
    YZ_PLANE,
    ZX_PLANE
} Plane;

Coordinate* circularInterpolation(Coordinate start, Coordinate center, Plane plane, float angle, int steps);
Coordinate* linearInterpolation(Coordinate start, Coordinate end, int steps);

#endif