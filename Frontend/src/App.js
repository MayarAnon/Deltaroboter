import React, { useState, useEffect } from "react";
import "./App.css";
import { RecoilRoot } from "recoil";
import { BrowserRouter as Router, Routes, Route } from 'react-router-dom';
import SettingsPage from "./Components/Settings";
import Header from "./Components/Header";
import ManuellMode from "./Components/manualMode";
import Debugger from "./Components/debugger"
import GCode from "./Components/GCode";
import DigitalTwin from "./Components/DigitalTwin";

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
      </RecoilRoot>
      </Router>
  );
};

export default App;



  //hier ist das aktuell Geladene Programm gespeichert
  // const [items, setItems] = useState([]);

  // useEffect(() => {
  //   console.log(items);
  // }, [items]);
//  {/* <Route path="/pick-and-place" element={<PickPlaceMode
//               color={Settings.color}
//               items={items}
//               setItems={setItems} // Übergeben Sie den setState direkt
//               coordinates={coordinates}
//             />} /> /*} */}