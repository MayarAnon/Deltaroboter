import ace from 'ace-builds/src-noconflict/ace';
ace.define('ace/mode/gcode_highlight_rules', ['require', 'exports', 'module', 'ace/lib/oop', 'ace/mode/text_highlight_rules'], function(require, exports, module) {
    var oop = require("ace/lib/oop");
    var TextHighlightRules = require("ace/mode/text_highlight_rules").TextHighlightRules;

    var GCodeHighlightRules = function() {
        this.$rules = {
            "start": [
                {
                    token: "comment",
                    regex: ";.*$"
                },
                {
                    token: "command",
                    regex: "\\b(G0|G1|G2|G3|G4|G17|G18|G19|G28|M100|M200|M300|M400)\\b"
                },
                {
                    token: "error", // Fehlende Werte erkennen
                    regex: "\\b([XYZ])\\b(?![\\s-]*\\d)"
                },
                {
                    token: "warning", // Feedrate F darf 105 nicht überschreiten
                    regex: "\\bF(\\d+\\.?\\d*)\\b",
                    onMatch: function(value, currentState, stack, line) {
                        var speed = parseFloat(value.substring(1)); // Extrahiere die Zahl nach 'F'
                        if(speed > 105) {
                            return "error"; // Setze die Klasse auf 'error', wenn das Limit überschritten ist
                        }
                        return "parameter"; // Ansonsten normales Parameter-Highlighting
                    }
                },
                {
                    token: "text", // Standard-Token
                    regex: "\\b([XYZ])\\s*(-?\\d+\\.?\\d*)\\b",
                    onMatch: function(value, currentState, stack, line) {
                        const match = value.match(/([XYZ])\s*(-?[\d.]+)/);
                        const axis = match[1];
                        const position = parseFloat(match[2]);
                
                        // Grenzwerte für die Z-Achse
                        const zLimits = {min: -480, max: -280};
                        // Radius des Zylinders
                        const cylinderRadius = 200;
                
                        // Überprüfung für die Z-Achse
                        if (axis === 'Z' && (position < zLimits.min || position > zLimits.max)) {
                            return "error"; // Außerhalb der Z-Achsen-Grenzen
                        }
                
                        // Überprüfung für X und Y Achsen, wenn sie in der gleichen Zeile sind
                        const xyMatch = line.match(/X\s*(-?\d+\.?\d*)\s*Y\s*(-?\d+\.?\d*)/);
                        if (xyMatch) {
                            const x = parseFloat(xyMatch[1]);
                            const y = parseFloat(xyMatch[2]);
                            if (x*x + y*y > cylinderRadius*cylinderRadius) {
                                return "error"; // Außerhalb des Zylinderradius
                            }
                        }
                
                        // Wenn keine Fehler gefunden wurden
                        return "parameter";
                    }
                },
                
                {
                    token: "parameter",
                    regex: "\\b([XYZ])\\s*-?\\d+\\.?\\d*\\b"
                },
                {
                    token: "interpolation",
                    regex: "\\b([IJP])\\s*-?\\d+\\.?\\d*\\b"
                },
                {
                    token: "radius", // Für den R Parameter
                    regex: "\\bR\\s*-?\\d+\\.?\\d*\\b"
                },
                {
                    token: "feedrate", // Für den F Parameter
                    regex: "\\bF\\s*-?\\d+\\.?\\d*\\b"
                },
                {
                    token: "speed", // Für den S Parameter
                    regex: "\\bS\\s*-?\\d+\\b"
                },
                {
                    token: "axis_a", // Für den A Parameter
                    regex: "\\bA\\s*-?\\d+\\.?\\d*\\b"
                },
                {
                    token: "home",
                    regex: "\\b(G28)\\b",
                    next: "home_position"
                },
                {
                    token: "greifer",
                    regex: "\\b(M100|M200|M300|M400)\\b",
                    next: "greifer_mode"
                },
                
            ],
            "home_position": [
                {
                    token: "parameter",
                    regex: "\\bF\\s*-?\\d*\\.?\\d*\\b",
                    next: "start"
                },
                {
                    token: "invalid",
                    regex: "$",
                    next: "start"
                }
            ],
            "greifer_mode": [
                {
                    token: "parameter",
                    regex: "\\bS\\s*-?\\d+\\b",
                    next: "start"
                },
                {
                    token: "invalid",
                    regex: "$",
                    next: "start"
                }
            ]
        };
        this.normalizeRules();
    };

    oop.inherits(GCodeHighlightRules, TextHighlightRules);
    exports.GCodeHighlightRules = GCodeHighlightRules;
});

ace.define('ace/mode/gcode', ['require', 'exports', 'module', 'ace/lib/oop', 'ace/mode/text', 'ace/mode/gcode_highlight_rules'], function(require, exports, module) {
    var oop = require("ace/lib/oop");
    var TextMode = require("ace/mode/text").Mode;
    var GCodeHighlightRules = require("ace/mode/gcode_highlight_rules").GCodeHighlightRules;

    var Mode = function() {
        this.HighlightRules = GCodeHighlightRules;
    };
    oop.inherits(Mode, TextMode);

    (function() {
        // Additional configuration and methods can be defined here if needed
        this.$id = "ace/mode/gcode";
    }).call(Mode.prototype);

    exports.Mode = Mode;
});
