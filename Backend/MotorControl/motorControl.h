#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <pigpio.h>

#define MOTOR_COUNT 3

extern int motor_gpios[MOTOR_COUNT];
extern int dir_gpios[MOTOR_COUNT];
extern int enb_gpios[MOTOR_COUNT];

void initialize_motors();
void execute_interpolated_sequence(int pulses[MOTOR_COUNT], int pulseWidthUs, int pauseBetweenPulsesUs, int directionChangeDelayUs);

#endif
