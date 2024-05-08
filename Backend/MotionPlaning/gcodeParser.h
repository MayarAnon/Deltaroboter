#ifndef GCODEPARSER_H
#define GCODEPARSER_H

#include "global.h"    // Include global definitions for structs and enums

// Function prototypes for processing G-code lines

// Verarbeitet eine einzelne Zeile des G-Code-Befehls.
// Parameter:
//   - const char* line: Die Zeile des G-Codes, die verarbeitet werden soll.
void processLine(const char* line); 



#endif // GCODEPARSER_H
