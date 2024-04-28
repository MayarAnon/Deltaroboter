import React, { useState, useEffect } from "react";
import "./App.css";
import { RecoilRoot } from "recoil";
import { BrowserRouter as Router, Routes, Route } from 'react-router-dom';
import SettingsPage from "./Components/Settings";
import Header from "./Components/Header";
import ManuellMode from "./Components/manualMode";
import PickPlaceMode from "./Components/pickPlaceMode";
import Game from "./Components/game"
import GCode from "./Components/GCode";


const App = () => {

  return (
     <Router>
      <RecoilRoot>
          <Header/>
          <Routes>
            <Route path="/" element={<ManuellMode/>} />
            <Route path="/oula" element={<Game/>} />
            <Route path="/settings" element={<SettingsPage/>} />
            <Route path="/gcode-editor" element={<GCode/>} />         
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