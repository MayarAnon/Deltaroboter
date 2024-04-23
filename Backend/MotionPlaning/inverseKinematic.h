#ifndef INVERSEKINEMATIC_H
#define INVERSEKINEMATIC_H


// Funktion zur Berechnung des Winkels theta für die YZ-Ebene. Diese Funktion ist ein Hilfsmittel für die inverse Kinematik.
// Parameter:
//   - float x0, y0, z0: Die Positionskoordinaten, für die der Winkel theta berechnet werden soll.
//   - float *theta: Zeiger auf die Variable, in der das Ergebnis gespeichert wird.
// Rückgabewert:
//   - int: Status der Berechnung (0=OK, -1=nicht existierende Position)
int delta_calcAngleYZ(float x0, float y0, float z0, float *theta);

// Inverse Kinematik: Transformiert die Koordinaten (x0, y0, z0) in die Gelenkwinkel (theta1, theta2, theta3).
// Parameter:
//   - float x0, y0, z0: Die Positionskoordinaten des Endeffektors.
//   - float *theta1, *theta2, *theta3: Zeiger auf die Variablen, in denen die Winkel gespeichert werden.
// Rückgabewert:
//   - int: Status der Berechnung (0=OK, -1=nicht existierende Position)
int delta_calcInverse(float x0, float y0, float z0, float *theta1, float *theta2, float *theta3);

#endif