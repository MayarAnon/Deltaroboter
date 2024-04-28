import React, { useState} from 'react';
import axios from 'axios';
import {TextEditor} from './Texteditor';
import LoadProgrammList from './ProgrammList'
import { useRecoilState } from "recoil";
import { settingAtom,gCodeStringAtom,gCodeModeAtom } from "../utils/atoms";
import GCodeEditor from "./GCodeEditor";


const GCode = (p) =>{
    const [settings, setSettings] = useRecoilState(settingAtom);
    const [GCodemode, setGCodemode] = useRecoilState(gCodeModeAtom);
    const [sharedString, setSharedString ] =  useRecoilState(gCodeStringAtom);

    const updateModeGCode = (modi) => {
        setGCodemode(modi);
        console.log(modi)
      }
    const [isMenuHidden, setMenuHidden] = React.useState(true);
    const toggleMenu = () => {
    setMenuHidden(!isMenuHidden);
    };

    const saveGCode = () =>{
        const address = "http://deltarobot:3010/gcode";
    
      // Erfassen Sie den Programmnamen (name) und die Positionsdaten (items)
    
      axios.post(address, sharedString)
        .then(response => {
          console.log('Daten erfolgreich gesendet:', response.data);
          // Fügen Sie hier den Code hinzu, um den gespeicherten Datensatz in Ihrer Komponente zu verarbeiten oder anzuzeigen, falls erforderlich.
        })
        .catch(error => {
          console.error('Fehler beim Senden der Daten:', error);
        });
    }

    const handleNameChange = (e) =>{
      console.log(e.target.value)
      setSharedString((prevSettings) => ({
        ...prevSettings,
        name: e.target.value,
      }));
    }
 

    return(
        <>
            <div style={{ backgroundColor: settings.color }} className="p-4 text-white rounded-xl font-bold  mt-10 mx-5 flex items-center justify-between border-4 border-black">
          <div className="text-3xl sm:text-l ">G-Code</div>
          <div className='flex space-x-2 '>
            
            <button className="sm:hidden" id="burgerheader" onClick={toggleMenu}>
              <img src="Burgermenu.png" className="object-contain object-center w-10 h-10" alt="Menu"></img>
            </button>
            <div className={`hidden sm:flex space-x-2`} id="menu">
            
            <button
              className="px-4 py-2 border-2 border-white rounded hover:bg-black"
              onClick={() => {  updateModeGCode(0) }}
              >
            
               <img src="loadprogrammicon.png" className="object-contain object-center w-10 h-10" alt="loadCode"/>
            </button>
            <button
              className="px-4 py-2 border-2 border-white rounded hover:bg-black"
              onClick={() => {  updateModeGCode(1) }}
            >
              
               <img src="GCode.png" className="object-contain object-center w-10 h-10" alt="Editor"/>
            </button>
            <button
              className="px-4 py-2  rounded hover:bg-black" onClick={() => { GCodemode === 2 ? updateModeGCode(1) : updateModeGCode(2) }}>
              
              {GCodemode === 2 ?
               <img src="Closeicon.png" className="object-contain object-center w-10 h-10" alt="CloseSave"/> :
               <img src="Saveicon.png" className="object-contain object-center w-10 h-10" alt="Save" />}
            </button>
            </div>
          </div>
        </div>
        {isMenuHidden === true && (
        <div style={{ backgroundColor: settings.color }} className="sm:hidden mx-5 p-4 border-4 border-black rounded-2xl flex flex-row items-center justify-between">
            <div className="flex flex-row justify-between items-center w-full">
                <div className={"md:hidden"} id="menu">
                
                <button
                className="px-2 smm:px-4 py-2 border-2 border-white rounded hover:bg-black"
                onClick={() => {  updateModeGCode(0) }}
                >
                <img src="loadprogrammicon.png" className="object-contain object-center w-10 h-10" alt="loadCode"/>
                </button>
                <button
                className="px-2 smm:px-4 py-2 border-2 border-white rounded hover:bg-black"
                onClick={() => {  updateModeGCode(1) }}
                >
                <img src="GCode.png" className="object-contain object-center w-10 h-10" alt="Editor"/>
                </button>
                <button
              className="px-4 py-2  rounded hover:bg-black" onClick={() => { GCodemode === 2 ? updateModeGCode(1) : updateModeGCode(2) }}>
              
              {GCodemode === 2 ?
               <img src="Closeicon.png" className="object-contain object-center w-10 h-10" alt="CloseSave" /> :
               <img src="Saveicon.png" className="object-contain object-center w-10 h-10" alt="Save"/>}
            </button>
                
                </div>
            </div>
        </div>)}

        {GCodemode === 2 && (
          <>
          <div style={{ backgroundColor: settings.color }} className="mx-5 p-2 border-4 border-black rounded-2xl flex items-center justify-between">
            <div className="flex flex-col sm:flex-row justify-between items-center w-full">
              <label className="text-white mb-2 mr-2 font-bold">Speichern Unter:</label>
              <input
                value={sharedString.name}
                onChange={handleNameChange}
                className="text-black rounded p-1 flex-grow sm:mr-5 mt-4 mb-4 sm:mt-0 sm:mb-0" // Abstand zwischen Eingabefeld und Button
              />
              <button
                className="px-4 py-1 border-2 border-white text-white text-l font-bold rounded hover:bg-black"
                onClick={saveGCode}
              >
                Speichern
              </button>
            </div>
  
          </div>
          
          </>
        )}
        {(GCodemode === 1 || GCodemode === 2) &&(
            <GCodeEditor />
        )}
        
        {GCodemode === 0 &&(
           <LoadProgrammList/>
        )}
        </>
    )

}


export default GCode