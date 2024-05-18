#include <stdio.h>
#include <math.h>




// Definition der geometrischen Konstanten des Roboters
const float e = 173.21;     // Länge des Endeffektors
const float f = 346.41;    // Länge der Basis
const float re = 400.0; // Länge der Verbindungsglieder des Endeffektors
const float rf = 150.0; // Länge der Verbindungsglieder der Basis

// Definition trigonometrischer Konstanten
const float sqrt3 = sqrt(3.0);
const float pi = 3.141592653;
const float sin120 = sqrt3 / 2.0;
const float cos120 = -0.5;
const float tan60 = sqrt3;
const float sin30 = 0.5;
const float tan30 = 1 / sqrt3;
 
// Funktion zur Berechnung des Winkels theta für die YZ-Ebene. Diese Funktion ist ein Hilfsmittel für die inverse Kinematik.
// Parameter:
//   - float x0, y0, z0: Die Positionskoordinaten, für die der Winkel theta berechnet werden soll.
//   - float *theta: Zeiger auf die Variable, in der das Ergebnis gespeichert wird.
// Rückgabewert:
//   - int: Status der Berechnung (0=OK, -1=nicht existierende Position)
int delta_calcAngleYZ(float x0, float y0, float z0, float *theta) {
     float y1 = -0.5 * 0.57735 * f; // Berechnung basierend auf f/2 * tan(30)
     y0 -= 0.5 * 0.57735    * e;    // Verschiebung vom Zentrum zur Kante des Endeffektors
     // Berechnung der Koeffizienten für die lineare Gleichung z = a + b*y
     float a = (x0*x0 + y0*y0 + z0*z0 +rf*rf - re*re - y1*y1)/(2*z0);
     float b = (y1-y0)/z0;
     // Diskriminante für die Lösung der quadratischen Gleichung
     float d = -(a+b*y1)*(a+b*y1)+rf*(b*b*rf+rf); 
     if (d < 0) return -1; // Nicht existierender Punkt, wenn d < 0
     float yj = (y1 - a*b - sqrt(d))/(b*b + 1); // Auswahl des äußeren Punktes
     float zj = a + b*yj;
     *theta = 180.0*atan(-zj/(y1 - yj))/pi + ((yj>y1)?180.0:0.0);
     return 0;
 }
 
// Inverse Kinematik: Transformiert die Koordinaten (x0, y0, z0) in die Gelenkwinkel (theta1, theta2, theta3).
// Parameter:
//   - float x0, y0, z0: Die Positionskoordinaten des Endeffektors.
//   - float *theta1, *theta2, *theta3: Zeiger auf die Variablen, in denen die Winkel gespeichert werden.
// Rückgabewert:
//   - int: Status der Berechnung (0=OK, -1=nicht existierende Position)
int delta_calcInverse(float x0, float y0, float z0, float *theta1, float *theta2, float *theta3) {
    *theta1 = *theta2 = *theta3 = 0; // Initialisierung der Winkel auf 0
    //Checken ob im Arbeitsraum: 
    int radius_squared = 200 * 200;  // Quadrat des Radius für Vergleich, um die Wurzelberechnung zu vermeiden

    // Überprüfe, ob x und y im Kreis liegen
    if ((x0 * x0 + y0 * y0) > radius_squared) {
        return -1;
    }
    // Überprüfe, ob z im gültigen Bereich liegt
    if (z0 > -280 || z0 < -480) {
        return -1;
    }
    int status = delta_calcAngleYZ(x0, y0, z0, theta1);
    if (status == 0) status = delta_calcAngleYZ(x0 * cos120 + y0 * sin120, y0 * cos120 - x0 * sin120, z0, theta2); // Rotation um +120° für theta2
    if (status == 0) status = delta_calcAngleYZ(x0 * cos120 - y0 * sin120, y0 * cos120 + x0 * sin120, z0, theta3); // Rotation um -120° für theta3
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