import React from "react";
import "./App.css";
import { RecoilRoot } from "recoil";
import { BrowserRouter as Router, Routes, Route } from 'react-router-dom';
import SettingsPage from "./Components/Settings";
import Header from "./Components/Header";
import ManuellMode from "./Components/ManualMode";
import Debugger from "./Components/Debugger"
import GCode from "./Components/GCode";
import DigitalTwin from "./Components/DigitalTwin";
import WebSocketConnection from "./Components/WebSocketConnection";
const App = () => {
  
  return (
     <Router>
      <RecoilRoot>
          <Header/>
          <Routes>
            <Route path="/" element={<ManuellMode/>} />
            <Route path="/debug-mode" element={<Debugger/>} />
            <Route path="/settings" element={<SettingsPage/>} />
            <Route path="/gcode-editor" element={<GCode/>} />
            <Route path="/digital-twin" element={<DigitalTwin/>} />          
          </Routes>
          <WebSocketConnection />
      </RecoilRoot>
      </Router>
  );
};

export default App;
