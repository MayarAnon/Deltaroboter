#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
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
Coordinate* circularInterpolation(Coordinate start, Coordinate end, Coordinate center, float radius, Plane plane, int direction, int *numSteps) {
    float angleStart, angleEnd, angle, phiStart, phiEnd, phi, stepSize = 0.1;
    int i;
    Coordinate *points;

    // Berechnet den Radius automatisch, falls dieser nicht explizit angegeben wurde (<= 0)
    if (radius <= 0) {
        radius = sqrt(pow(start.x - center.x, 2) + pow(start.y - center.y, 2) + pow(start.z - center.z, 2));
    }

    // Berechnet Start- und Endwinkel basierend auf der spezifizierten Ebene
    if (plane == XY_PLANE) {
        angleStart = atan2(start.y - center.y, start.x - center.x);
        angleEnd = atan2(end.y - center.y, end.x - center.x);
    } else if (plane == YZ_PLANE) {
        angleStart = atan2(start.z - center.z, start.y - center.y);
        angleEnd = atan2(end.z - center.z, end.y - center.y);
    } else { // ZX_PLANE
        angleStart = atan2(start.z - center.z, start.x - center.x);
        angleEnd = atan2(end.z - center.z, end.x - center.x);
    }

    // Anpassung des Endwinkels für kontinuierliche Interpolation über 360° Grenze hinweg
    if ((direction == 1 && angleEnd < angleStart) || (direction == -1 && angleEnd > angleStart)) {
        angleEnd += 2 * M_PI * direction;
    }

    float totalAngle = angleEnd - angleStart;
    phiStart = start.phi;
    phiEnd = end.phi;

    // Berechnung der notwendigen Anzahl von Interpolationsschritten
    *numSteps = (int)(fabs(totalAngle) / stepSize) + 1;  // +1 to include the end point exactly
    points = malloc(*numSteps * sizeof(Coordinate));
    if (!points) {
        perror("Not enough memory to allocate points");
        exit(1);
    }

    // Erzeugt die Punkte entlang der Kreisbahn
    for (i = 0; i < *numSteps; i++) {
        angle = angleStart + i * (totalAngle / (*numSteps - 1));  // evenly spaced
        phi = phiStart + i * ((phiEnd - phiStart) / (*numSteps - 1)); // linear interpolation of phi
        if (plane == XY_PLANE) {
            points[i].x = center.x + radius * cos(angle);
            points[i].y = center.y + radius * sin(angle);
            points[i].z = center.z;
            points[i].phi = phi;
        } else if (plane == YZ_PLANE) {
            points[i].y = center.y + radius * cos(angle);
            points[i].z = center.z + radius * sin(angle);
            points[i].x = center.x;
            points[i].phi = phi;
        } else {  // ZX_PLANE
            points[i].x = center.x + radius * cos(angle);
            points[i].z = center.z + radius * sin(angle);
            points[i].y = center.y;
            points[i].phi = phi;
        }
    }

    // Sicherstellen, dass Start- und Endpunkte exakt sind
    points[0] = start;          // Set the first point exactly to 'start'
    points[*numSteps - 1] = end;  // Set the last point exactly to 'end'

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

