import React,{useEffect} from "react";
import * as THREE from "three";
import "./App.css";
import { RecoilRoot, useSetRecoilState } from "recoil";
import { BrowserRouter as Router, Routes, Route } from 'react-router-dom';
import SettingsPage from "./Components/Settings";
import Header from "./Components/Header";
import ManuellMode from "./Components/ManualMode";
import Debugger from "./Components/Debugger"
import GCode from "./Components/GCode";
import DigitalTwin from "./Components/DigitalTwin";
import { robotStateAtom,pathPointsAtom } from './utils/atoms';
import WebSocketConnection from "./Components/WebSocketConnection";
const App = () => {
  const [robotState,setRobotState] = useSetRecoilState(robotStateAtom);
  const setPathPoints = useSetRecoilState(pathPointsAtom);
  const WebSocketConnection = () => {
    
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
  

  useEffect(() => {
    if (robotState.currentCoordinates.length > 0) {
      setPathPoints(prev => [
        ...prev,
        new THREE.Vector3(...robotState.currentCoordinates),
      ]);
    }
  }, [robotState.currentCoordinates, setPathPoints]);
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
