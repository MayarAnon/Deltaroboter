#ifndef INVERSEKINEMATIC_H
#define INVERSEKINEMATIC_H

int delta_calcAngleYZ(float x0, float y0, float z0, float *theta);
int delta_calcInverse(float x0, float y0, float z0, float *theta1, float *theta2, float *theta3);

#endif