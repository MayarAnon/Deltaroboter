#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

typedef struct {
    float x,y,z,phi;
} Coordinate;

typedef enum {
    XY_PLANE,
    YZ_PLANE,
    ZX_PLANE
} Plane;



// Funktion zur Berechnung des Startwinkels basierend auf der aktuellen Flugebene
float calculateInitialAngle(Coordinate start, Coordinate center, Plane plane) {
    switch (plane) {
        case XY_PLANE:
            return atan2(start.y - center.y, start.x - center.x);
        case YZ_PLANE:
            return atan2(start.z - center.z, start.y - center.y);
        case ZX_PLANE:
            return atan2(start.x - center.x, start.z - center.z);
    }
    return 0;  // Standard-Rückgabewert, falls keine passende Ebene gefunden wird (Sicherheitsmaßnahme)
}

// Hauptfunktion zur Erzeugung der Kreisinterpolation
Coordinate* circularInterpolation(Coordinate start, Coordinate center, Plane plane, float angle, int steps) {
    Coordinate* points = (Coordinate*)malloc(steps * sizeof(Coordinate));
    if (points == NULL) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    float radius = sqrt(pow(start.x - center.x, 2) + pow(start.y - center.y, 2) + pow(start.z - center.z, 2));
    float angleRadians = angle * M_PI / 180;
    float angleIncrement = angleRadians / (steps - 1);
    float currentAngle = calculateInitialAngle(start, center, plane);
    //Interpolation of PHI Motor
    float incrementalPhi = (center.phi - start.phi)/steps;

    for (int i = 0; i < steps; i++) {
        switch (plane) {
            case XY_PLANE:
                float t = (steps == 1) ? 0 : (float)i / (steps - 1);
                points[i].x = center.x + radius * cos(currentAngle);
                points[i].y = center.y + radius * sin(currentAngle);
                points[i].z = start.z; // Z-coordinate remains constant
                points[i].phi = start.phi + t * (center.phi - start.phi);
                break;
            case YZ_PLANE:
                points[i].y = center.y + radius * cos(currentAngle);
                points[i].z = center.z + radius * sin(currentAngle);
                points[i].x = start.x; // X-coordinate remains constant
                points[i].phi = center.phi + i*incrementalPhi;
                break;
            case ZX_PLANE:
                points[i].z = center.z + radius * cos(currentAngle);
                points[i].x = center.x + radius * sin(currentAngle);
                points[i].y = start.y; // Y-coordinate remains constant
                points[i].phi = center.phi + i*incrementalPhi;
                break;
        }
        //printf("Step %d: (%f, %f, %f)\n", i, points[i].x, points[i].y, points[i].z);  // Debug output for each step
        currentAngle += angleIncrement;
    }

    return points;
}

// Implementierung der Funktion
Coordinate* linearInterpolation(Coordinate start, Coordinate end, int steps) {
    if (steps < 2) {
        // Mindestens zwei Punkte (Start und Ende) sind erforderlich
        fprintf(stderr, "Fehler: Es sind mindestens zwei Schritte erforderlich.\n");
        exit(EXIT_FAILURE);
    }

    Coordinate* points = (Coordinate*)malloc(steps * sizeof(Coordinate));
    if (points == NULL) {
        perror("Speicherzuweisungsfehler");
        exit(EXIT_FAILURE);
    }



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

