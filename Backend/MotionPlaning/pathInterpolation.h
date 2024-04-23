#ifndef PATHINTERPOLATION_H
#define PATHINTERPOLATION_H
#include "global.h"

// Die Funktion `circularInterpolation` berechnet eine Reihe von Koordinatenpunkten auf einer kreisförmigen Bahn
// zwischen zwei Punkten `start` und `end` um ein Zentrum `center` auf einer spezifizierten Ebene `plane`.
// `radius` gibt den Radius der Kreisbahn an, `direction` die Drehrichtung (1 für positiv, -1 für negativ).
// `numSteps` wird die Anzahl der interpolierten Schritte/Punkte zurückgeben.
// Parameter:
//   - Coordinate start: Startpunkt der Interpolation
//   - Coordinate end: Endpunkt der Interpolation
//   - Coordinate center: Zentrum des Kreises
//   - float radius: Radius des Kreises, wenn <= 0, wird er automatisch berechnet
//   - Plane plane: die Ebene der Interpolation (XY_PLANE, YZ_PLANE, ZX_PLANE)
//   - int direction: Drehrichtung (1 für Uhrzeigersinn, -1 für gegen den Uhrzeigersinn)
//   - int *numSteps: Zeiger auf eine Variable, in der die Anzahl der Schritte gespeichert wird
// Rückgabewert:
//   - Coordinate*: Zeiger auf ein Array von Koordinaten, das die interpolierten Punkte enthält
Coordinate* circularInterpolation(Coordinate start, Coordinate end, Coordinate center, float radius, Plane plane, int direction, int *numSteps);

// Die Funktion `linearInterpolation` berechnet eine lineare Interpolation zwischen zwei Punkten `start` und `end`.
// Parameter:
//   - Coordinate start: Startpunkt der Interpolation
//   - Coordinate end: Endpunkt der Interpolation
//   - int steps: Anzahl der gewünschten Interpolationsschritte
// Rückgabewert:
//   - Coordinate*: Zeiger auf ein Array von Koordinaten, das die interpolierten Punkte enthält
Coordinate* linearInterpolation(Coordinate start, Coordinate end, int steps);

#endif