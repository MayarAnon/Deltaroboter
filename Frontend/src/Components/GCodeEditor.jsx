import React, { useState, useEffect } from "react";
import AceEditor from "react-ace";
import "../styles/aceEditorStyles.css";
import "ace-builds/src-noconflict/theme-terminal";
import "ace-builds/src-noconflict/theme-crimson_editor";
import { useRecoilState } from "recoil";
import { gCodeStringAtom, settingAtom } from "../utils/atoms";
require("../utils/gcode_mode"); // Importing custom GCode mode for Ace Editor

const GCodeEditor = () => {
  const [sharedString, setSharedString] = useRecoilState(gCodeStringAtom);
  const [settings, setSettings] = useRecoilState(settingAtom);
  const [gCode, setGCode] = useState("");

  // Update GCode content
  useEffect(() => {
      setGCode(sharedString.content);
  }, []);
  // Handler for GCode changes in Ace Editor
  const handleGCodeChange = (newValue) => {
    // Update local GCode state with new value
    setGCode(newValue);
    // Update shared GCode string in Recoil state
    setSharedString((prevSettings) => ({
      ...prevSettings,
      content: newValue,
    }));
  };
   // Dynamically set the theme based on settings.darkMode
   const editorTheme = settings.darkMode ? "terminal" : "crimson_editor";
  return (
    <>
      <div className="gcode-editor">
        <AceEditor
          mode="gcode"
          theme={editorTheme}
          value={gCode}
          onChange={handleGCodeChange}
          name="G_CODE_EDITOR"
          editorProps={{ $blockScrolling: true }}
          setOptions={{
            showLineNumbers: true,
            tabSize: 2,
          }}
        />
      </div>
    </>
  );
};

export default GCodeEditor;
