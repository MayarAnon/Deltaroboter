import React,{useEffect} from "react";
import "./App.css";
import { RecoilRoot, useSetRecoilState } from "recoil";
import { BrowserRouter as Router, Routes, Route } from 'react-router-dom';
import SettingsPage from "./Components/Settings";
import Header from "./Components/Header";
import ManuellMode from "./Components/ManualMode";
import Debugger from "./Components/Debugger"
import GCode from "./Components/GCode";
import DigitalTwin from "./Components/DigitalTwin";
import { robotStateAtom } from './utils/atoms';
const App = () => {
  const WebSocketConnection = () => {
    const setRobotState = useSetRecoilState(robotStateAtom);
  
    useEffect(() => {
      const websocket = new WebSocket("ws://192.168.0.43:80");
  
      websocket.onopen = () => {
        console.log("WebSocket connected");
      };
  
      websocket.onmessage = (event) => {
        const data = JSON.parse(event.data);
        setRobotState(prevState => ({
          ...prevState,
          ...data
        }));
      };
  
      websocket.onerror = (error) => {
        console.error("WebSocket error:", error);
      };
  
      websocket.onclose = (event) => {
        console.log("WebSocket disconnected", event.reason);
        setTimeout(() => {
          websocket = new WebSocket("ws://192.168.0.43:80");
        }, 5000);
      };
  
      return () => {
        websocket.close();
      };
    }, [setRobotState]);
  
    return null;
  };

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
      </RecoilRoot>
      </Router>
  );
};

export default App;
