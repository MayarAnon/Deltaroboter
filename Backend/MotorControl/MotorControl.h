#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <pigpio.h>
#include "config.h" 

void initialize_motors();
void execute_interpolated_sequence(int* pulses, int pulseWidthUs, int pauseBetweenPulsesUs, int directionChangeDelayUs);

#endif