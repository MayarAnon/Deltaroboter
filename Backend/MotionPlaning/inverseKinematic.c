#include <stdio.h>
#include <math.h>




// robot geometry
const float e = 50;     // end effector
const float f = 100;     // base
const float re = 400.0;
const float rf = 150.0;

// trigonometric constants
const float sqrt3 = sqrt(3.0);
const float pi = 3.141592653;
const float sin120 = sqrt3 / 2.0;
const float cos120 = -0.5;
const float tan60 = sqrt3;
const float sin30 = 0.5;
const float tan30 = 1 / sqrt3;
 
 // inverse kinematics
 // helper functions, calculates angle theta1 (for YZ-pane)
int delta_calcAngleYZ(float x0, float y0, float z0, float *theta) {
     float y1 = -0.5 * 0.57735 * f; // f/2 * tg 30
     y0 -= 0.5 * 0.57735    * e;    // shift center to edge
     // z = a + b*y
     float a = (x0*x0 + y0*y0 + z0*z0 +rf*rf - re*re - y1*y1)/(2*z0);
     float b = (y1-y0)/z0;
     // discriminant
     float d = -(a+b*y1)*(a+b*y1)+rf*(b*b*rf+rf); 
     if (d < 0) return -1; // non-existing point
     float yj = (y1 - a*b - sqrt(d))/(b*b + 1); // choosing outer point
     float zj = a + b*yj;
     *theta = 180.0*atan(-zj/(y1 - yj))/pi + ((yj>y1)?180.0:0.0);
     return 0;
 }
 
// inverse kinematics: (x0, y0, z0) -> (theta1, theta2, theta3)
// returned status: 0=OK, -1=non-existing position
int delta_calcInverse(float x0, float y0, float z0, float *theta1, float *theta2, float *theta3) {
    *theta1 = *theta2 = *theta3 = 0; // Korrekte Initialisierung der Werte, auf die die Zeiger zeigen
    int status = delta_calcAngleYZ(x0, y0, z0, theta1);
    if (status == 0) status = delta_calcAngleYZ(x0 * cos120 + y0 * sin120, y0 * cos120 - x0 * sin120, z0, theta2); // Rotation um +120°
    if (status == 0) status = delta_calcAngleYZ(x0 * cos120 - y0 * sin120, y0 * cos120 + x0 * sin120, z0, theta3); // Rotation um -120°
    return status;
}


/*
int main() {
    float x0, y0, z0;
    float theta1, theta2, theta3;

    x0 = 5; y0 = 0; z0 = -280;

    // Beispiel für Rückwärtskinematik
    if (delta_calcInverse(x0, y0, z0, &theta1, &theta2, &theta3) == 0) {
        printf("Rückwärtskinematik:\n");
        printf("Theta1 = %f\nTheta2 = %f\nTheta3 = %f\n", theta1, theta2, theta3);
    } else {
        printf("Winkelposition existiert nicht.\n");
    }

    return 0;
}
*/