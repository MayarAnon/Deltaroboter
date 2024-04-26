import React, { useState, useEffect } from "react";
import AceEditor from "react-ace";
import {GCodeViewer} from "react-gcode-viewer";
import { v4 as uuidv4 } from "uuid";
import "../styles/aceEditorStyles.css";
import * as sequenceStorage from "../utils/sequenceStorage";
import {
  parseGCodeToSequence,
  convertSequencesToGCode,
} from "../utils/parseGCode";
import "ace-builds/src-noconflict/theme-github";
import RobotStateDisplay from "./robotstate";
require("../utils/gcode_mode");
const url = "https://github.com/MayarAnon/Deltaroboter/blob/main/Backend/GCodeFiles/6127a7f9aa32f718b8c1ab4f.gcode"

const style = {
    top: 0,
    left: 0,
    width: '100vw',
    height: '100vh',
}
const GCodeEditor = () => {
  const [gCode, setGCode] = useState("");
  const [sequenceName, setSequenceName] = useState("");
  const [originalName, setOriginalName] = useState("");

  const sequenceToEdit = 0;

  useEffect(() => {
    if (sequenceToEdit) {
      const gCode = convertSequencesToGCode(sequenceToEdit.steps);
      setGCode(gCode);
      setSequenceName(sequenceToEdit.name || "");
      setOriginalName(sequenceToEdit.name || "");
    }
  }, [sequenceToEdit]);

  const handleGCodeChange = (newValue) => {
    setGCode(newValue);
  };

  const handleNameChange = (event) => {
    setSequenceName(event.target.value);
  };

  const saveSequence = () => {
    try {
      const steps = parseGCodeToSequence(gCode);
      const sequenceData = {
        id: sequenceToEdit.id || uuidv4(),
        name: sequenceName,
        steps,
      };

      if (
        sequenceToEdit &&
        sequenceToEdit.id &&
        originalName === sequenceName
      ) {
        sequenceStorage.updateSequence(sequenceToEdit.id, { ...sequenceData });
        console.log(`Sequence updated: ${JSON.stringify(sequenceData)}`);
      } else {
        sequenceStorage.saveSequence({ ...sequenceData });
        console.log(`New sequence saved: ${JSON.stringify(sequenceData)}`);
      }
      setSequenceName("");
    } catch (error) {
      console.error(
        "Error saving G-Code sequence:",
        error.message,
        error.stack
      );
    }
  }

  return (
    <>
      <div className="gcode-viewer">
          <GCodeViewer 
            gcode={gCode} 
            orbitControls
            showAxes
            style={style}
            url={url}/>
      </div>
      <div className="gcode-editor">
        <h2>G-Code Editor</h2>
        <AceEditor
          mode="gcode"
          theme="github"
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
      <div className="save-section">
        <button onClick={saveSequence}>Save Sequence</button>
        <input
          type="text"
          placeholder="Sequence Name"
          value={sequenceName}
          onChange={handleNameChange}
          style={{ marginBottom: "10px" }}
        />
      </div>
      <RobotStateDisplay  />
    </>
  );
};

export default GCodeEditor;
