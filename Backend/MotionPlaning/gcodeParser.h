#ifndef GCODEPARSER_H
#define GCODEPARSER_H

#include "global.h"    // Include global definitions for structs and enums


// Function prototypes for processing G-code lines
void processLine(const char* line); 
void processInterpolationAndCreateJSON(Coordinate* coordinates, int InterpolationSteps, float f);
void publishCurrentState(Coordinate pos, Angles ang); // Publishes the current state of the robot

#endif // GCODEPARSER_H
