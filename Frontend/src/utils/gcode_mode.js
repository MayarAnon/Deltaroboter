/*
  This script defines the syntax highlighting rules for the G-code syntax in the ACE editor.
  It divides the G-code into various token types to enable appropriate highlighting in the editor.

  The G-code highlighting rules include the detection of comments, commands, parameters, and other elements.

  The syntax highlighting rule set is divided into two main parts:
  1. "start": Defines the main rules for the G-code syntax.
  2. "home_position" and "greifer_mode": Define specific rules for the G28 and M commands.

  Token types:
  - comment: Comments in the G-code.
  - command: G-code commands like G0, G1, etc.
  - error: Faulty or missing values in the G-code.
  - warning: Warnings for specific G-code commands, e.g., feedrate F must not exceed 105.
  - text: Standard text token.
  - parameter: G-code parameters like XYZ.
  - interpolation: Interpolation parameters like I, J, P.
  - radius: Radius parameter (R).
  - feedrate: Feedrate parameter (F).
  - speed: Speed parameter (S).
  - axis_a: Axis parameter (A).
  - home: G28 command.
  - greifer: Gripper commands (M100, M200, M300, M400).
*/

import ace from "ace-builds/src-noconflict/ace";
ace.define(
  "ace/mode/gcode_highlight_rules",
  [
    "require",
    "exports",
    "module",
    "ace/lib/oop",
    "ace/mode/text_highlight_rules",
  ],
  function (require, exports, module) {
    var oop = require("ace/lib/oop");
    var TextHighlightRules =
      require("ace/mode/text_highlight_rules").TextHighlightRules;

    var GCodeHighlightRules = function () {
      this.$rules = {
        start: [
          {
            token: "comment",
            regex: ";.*$",
          },
          {
            token: "command",
            regex: "\\b(G0|G1|G2|G3|G4|G17|G18|G19|G28|M100|M200|M300|M400)\\b",
          },
          {
            token: "error", // Detect missing values
            regex: "\\b([XYZ])\\b(?![\\s-]*\\d)",
          },
          {
            token: "warning", // Feedrate F must not exceed 105
            regex: "\\bF(\\d+\\.?\\d*)\\b",
            onMatch: function (value, currentState, stack, line) {
              var speed = parseFloat(value.substring(1)); // Extract the number after 'F'
              if (speed > 105) {
                return "error"; // Set class to 'error' if limit is exceeded
              }
              return "parameter"; // Otherwise, normal parameter highlighting
            },
          },
          {
            token: "text", // Standard-Token
            regex: "\\b([XYZ])\\s*(-?\\d+\\.?\\d*)\\b",
            onMatch: function (value, currentState, stack, line) {
              const match = value.match(/([XYZ])\s*(-?[\d.]+)/);
              const axis = match[1];
              const position = parseFloat(match[2]);

              // Z-axis limits
              const zLimits = { min: -480, max: -280 };
              // Cylinder radius
              const cylinderRadius = 200;

              // Check for Z-axis
              if (
                axis === "Z" &&
                (position < zLimits.min || position > zLimits.max)
              ) {
                return "error"; // Outside Z-axis limits
              }

              // Check for X and Y axes if they are on the same line
              const xyMatch = line.match(
                /X\s*(-?\d+\.?\d*)\s*Y\s*(-?\d+\.?\d*)/
              );
              if (xyMatch) {
                const x = parseFloat(xyMatch[1]);
                const y = parseFloat(xyMatch[2]);
                if (x * x + y * y > cylinderRadius * cylinderRadius) {
                  return "error"; // out of workingspace
                }
              }

              // If no errors found
              return "parameter";
            },
          },

          {
            token: "parameter",
            regex: "\\b([XYZ])\\s*-?\\d+\\.?\\d*\\b",
          },
          {
            token: "interpolation",
            regex: "\\b([IJP])\\s*-?\\d+\\.?\\d*\\b",
          },
          {
            token: "radius", // For the R Parameter
            regex: "\\bR\\s*-?\\d+\\.?\\d*\\b",
          },
          {
            token: "feedrate", // For the F Parameter
            regex: "\\bF\\s*-?\\d+\\.?\\d*\\b",
          },
          {
            token: "speed", // For the S Parameter
            regex: "\\bS\\s*-?\\d+\\b",
          },
          {
            token: "axis_a", // For the A Parameter
            regex: "\\bA\\s*-?\\d+\\.?\\d*\\b",
          },
          {
            token: "home",
            regex: "\\b(G28)\\b",
            next: "home_position",
          },
          {
            token: "greifer",
            regex: "\\b(M100|M200|M300|M400)\\b",
            next: "greifer_mode",
          },
        ],
        home_position: [
          {
            token: "parameter",
            regex: "\\bF\\s*-?\\d*\\.?\\d*\\b",
            next: "start",
          },
          {
            token: "invalid",
            regex: "$",
            next: "start",
          },
        ],
        greifer_mode: [
          {
            token: "parameter",
            regex: "\\bS\\s*-?\\d+\\b",
            next: "start",
          },
          {
            token: "invalid",
            regex: "$",
            next: "start",
          },
        ],
      };
      this.normalizeRules();
    };

    oop.inherits(GCodeHighlightRules, TextHighlightRules);
    exports.GCodeHighlightRules = GCodeHighlightRules;
  }
);
// Definition of the G-code mode for the ACE Editor
ace.define(
  "ace/mode/gcode",
  [
    "require",
    "exports",
    "module",
    "ace/lib/oop",
    "ace/mode/text",
    "ace/mode/gcode_highlight_rules",
  ],
  function (require, exports, module) {
    var oop = require("ace/lib/oop");
    var TextMode = require("ace/mode/text").Mode;
    var GCodeHighlightRules =
      require("ace/mode/gcode_highlight_rules").GCodeHighlightRules;

    var Mode = function () {
      this.HighlightRules = GCodeHighlightRules;
    };
    oop.inherits(Mode, TextMode);

    (function () {
      // Additional configuration and methods can be defined here if needed
      this.$id = "ace/mode/gcode";
    }).call(Mode.prototype);

    exports.Mode = Mode;
  }
);
