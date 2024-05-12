#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "global.h"
#include <string.h>
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
Coordinate* circularInterpolation(Coordinate start, Coordinate end, Coordinate center, float radius, Plane plane, int direction, int *numSteps) {
    float pointSpacing = 1;//angabe in mm
    float angleStart, angleEnd, angle, phiStart, phiEnd, phi, stepSize,totalAngle;
    int i;

    if (radius <= 0) {
        radius = sqrt(pow(start.x - center.x, 2) + pow(start.y - center.y, 2) + pow(start.z - center.z, 2));
    }

    switch (plane) {
        case XY_PLANE:
            angleStart = atan2(start.y - center.y, start.x - center.x);
            angleEnd = atan2(end.y - center.y, end.x - center.x);
            break;
        case YZ_PLANE:
            angleStart = atan2(start.z - center.z, start.y - center.y);
            angleEnd = atan2(end.z - center.z, end.y - center.y);
            break;
        case ZX_PLANE:
            angleStart = atan2(start.z - center.z, start.x - center.x);
            angleEnd = atan2(end.z - center.z, end.x - center.x);
            break;
    }
    
    // Überprüfung, ob Start und Endpunkt identisch sind
    if (memcmp(&start, &end, sizeof(Coordinate)) == 0) {
        totalAngle = 2 * PI;  // Ein vollständiger Kreis
        angleEnd = angleStart + totalAngle * direction;
    } else {
        if ((direction == 1 && angleEnd < angleStart) || (direction == -1 && angleEnd > angleStart)) {
            angleEnd += 2 * PI * direction;
        }
        totalAngle = angleEnd - angleStart;
    }

    // Berechnung des Schrittwinkels basierend auf dem gewünschten Punktabstand
    stepSize = pointSpacing / radius;  // Winkel in Radiant
    *numSteps = (int)(fabs(totalAngle) / stepSize) + 1;  // +1 um den Endpunkt genau einzuschließen

    Coordinate *points = malloc(*numSteps * sizeof(Coordinate));
    if (!points) {
        perror("Not enough memory to allocate points");
        exit(1);
    }

    for (i = 0; i < *numSteps; i++) {
        angle = angleStart + i * (totalAngle / (*numSteps - 1));
        phi = phiStart + i * ((phiEnd - phiStart) / (*numSteps - 1));
        switch (plane) {
            case XY_PLANE:
                points[i].x = center.x + radius * cos(angle);
                points[i].y = center.y + radius * sin(angle);
                points[i].z = center.z;
                break;
            case YZ_PLANE:
                points[i].y = center.y + radius * cos(angle);
                points[i].z = center.z + radius * sin(angle);
                points[i].x = center.x;
                break;
            case ZX_PLANE:
                points[i].x = center.x + radius * cos(angle);
                points[i].z = center.z + radius * sin(angle);
                points[i].y = center.y;
                break;
        }
        points[i].phi = phi;
    }

    points[0] = start;
    points[*numSteps - 1] = end;

    return points;
}



// Die Funktion `linearInterpolation` berechnet eine lineare Interpolation zwischen zwei Punkten `start` und `end`.
// Parameter:
//   - Coordinate start: Startpunkt der Interpolation
//   - Coordinate end: Endpunkt der Interpolation
//   - int steps: Anzahl der gewünschten Interpolationsschritte
// Rückgabewert:
//   - Coordinate*: Zeiger auf ein Array von Koordinaten, das die interpolierten Punkte enthält
Coordinate* linearInterpolation(Coordinate start, Coordinate end, int steps) {
    if (steps < 2) {
        // Sicherstellen, dass mindestens zwei Punkte (Start und Ende) vorhanden sind
        fprintf(stderr, "Fehler: Es sind mindestens zwei Schritte erforderlich.\n");
        exit(EXIT_FAILURE);
    }

    Coordinate* points = (Coordinate*)malloc(steps * sizeof(Coordinate));
    if (points == NULL) {
        perror("Speicherzuweisungsfehler");
        exit(EXIT_FAILURE);
    }


    // Berechnet die Punkte für die lineare Interpolation
    for (int i = 0; i < steps; i++) {
        // Für Start- und Endpunkt werden t = 0 bzw. t = 1 direkt gesetzt
        float t = (steps == 1) ? 0 : (float)i / (steps - 1);
        points[i].x = start.x + t * (end.x - start.x);
        points[i].y = start.y + t * (end.y - start.y);
        points[i].z = start.z + t * (end.z - start.z);
        points[i].phi = start.phi + t * (end.phi - start.phi);
    }

    return points;
}

