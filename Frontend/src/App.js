import React, { useState, useEffect} from 'react';
import { CSSTransition } from 'react-transition-group';
import './App.css'
import { StringProvider } from './Components/StringContext';

import SettingsPage from './Components/Settings';
import Header from './Components/Header';
import ManuellMode from './Components/manualMode';
import PickPlaceMode from './Components/pickPlaceMode'
import GCode from './Components/GCode'
import GCodeEditor from './Components/GCodeEditor';


const App = (props) => {
  const [menuState, setMenuState] = useState(1);
  
  const updateMode = (modi) => {
    setMenuState(modi);
    console.log(modi)
  }

  const [Settings, setSettings] = useState({interpolationMode: 'pointToPoint',
  speed: 50,
  manualMode: 'buttons',
  gripper: '1option',
  koordinateSystem: 'kartesisch',
  workSpaceRadius: '200',
  workSpaceHeight: '200',
  color: '#1e293b'});

  const handleSettingsChange = (newSettings) => {
    setSettings(newSettings);
    
    
  };
  const [coordinates,setCoordinates] = useState({
    x: 0,
    y: 0,
    z: 0,
    phi: 0,
    actuator: '',
    ton: 0,
    toff: 0
  })

  const handleCoordinateChange = (newCoordinates) =>{
    setCoordinates(newCoordinates)
  }

  //hier ist das aktuell Geladene Programm gespeichert
  const [items, setItems] = useState([]);

  useEffect(() =>{
    console.log(items)
  },[items])


  
  return (
    <>
      <StringProvider>
      
      <Header
       color ={Settings.color}
       callback={updateMode} 
       />
      <CSSTransition
        in={menuState === 1}
        timeout={250}
        classNames="slide-down"
        unmountOnExit
      >
        <ManuellMode 
          color ={Settings.color}
          modi={Settings.manualMode} 
          gripper={Settings.gripper} 
          speed={Settings.speed} 
          onCoordinateChange={handleCoordinateChange} 
          menuState={menuState}
          koordinateSystem={Settings.koordinateSystem} 
          workSpaceRadius ={Settings.workSpaceRadius}
          workSpaceHeight ={Settings.workSpaceHeight}/>
      </CSSTransition>
      
      {menuState === 2 && 
        <PickPlaceMode 
          color ={Settings.color}
          items={items}
          setItems={setItems} // Übergeben Sie den setState direkt
          coordinates={coordinates}
        />}
      {menuState === 3 && <SettingsPage
        color ={Settings.color}
        onSettingsChange={handleSettingsChange} 
        manuellMode ={Settings.manualMode} 
        Gripper ={Settings.gripper} 
        speed ={Settings.speed}
        koordinateSystem = {Settings.koordinateSystem}
        />}
      {menuState === 4 &&
        // <GCodeEditor />
        <GCode
        color ={Settings.color}
        menuState={menuState}
        />
      
      
      }
    </StringProvider>

    </>

  );
}



export default App
