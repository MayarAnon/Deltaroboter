// Ensure ace is imported in your project to have ace.define available
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
                    // This combines G, M, and T commands into one regex.
                    regex: "\\b([GM]\\d+|T\\d+)\\b"
                },
                
                {
                    token: "error",
                    regex: "\\b([XYZ])\\s+(-?\\d+\\.?\\d*)\\b"
                },
                {
                    token: "parameter",
                    regex: "\\b([XYZ])\\s*-?\\d+\\.?\\d*\\b"
                }
                
                // Add more rules as necessary
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
