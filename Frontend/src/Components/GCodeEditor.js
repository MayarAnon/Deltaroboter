import React, { useState, useEffect } from "react";
import AceEditor from "react-ace";
import { v4 as uuidv4 } from "uuid";
import "../styles/aceEditorStyles.css";
import * as sequenceStorage from "../utils/sequenceStorage";
import {
  parseGCodeToSequence,
  convertSequencesToGCode,
} from "../utils/parseGCode";
import "ace-builds/src-noconflict/theme-github";
import RobotStateDisplay from "./robotstate";
import { useRecoilState } from "recoil";
import { settingAtom,gCodeStringAtom } from "../utils/atoms";
require("../utils/gcode_mode");


const GCodeEditor = () => {
  
  const [sharedString, setSharedString ] =  useRecoilState(gCodeStringAtom);
  const [gCode, setGCode] = useState("");
 

  const sequenceToEdit = 1;

  useEffect(() => {
    if (sequenceToEdit) {
      const gCode = sharedString.content;
      setGCode(gCode);
    }
  }, [sequenceToEdit]);

  const handleGCodeChange = (newValue) => {
    setGCode(newValue);
    
      setSharedString((prevSettings) => ({
        ...prevSettings,
        content: newValue,
      }))
  };



  return (
    <>
      <div className="gcode-editor">
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
     <RobotStateDisplay  /> 
    </>
  );
};

export default GCodeEditor;
