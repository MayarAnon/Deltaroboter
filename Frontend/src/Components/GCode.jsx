import React, { useState} from 'react';
import { useString } from './StringContext';
import axios from 'axios';
import {TextEditor} from './Texteditor';
import LoadProgrammList from './ProgrammList'



const GCode = (props) =>{

    const [GCodemode, setGCodemode] = useState(0);
    const { sharedString, setSharedString } = useString();
    const [name,setName] = useState("")

    const updateModeGCode = (modi) => {
        setGCodemode(modi);
        console.log(modi)
      }

    const [isMenuHidden, setMenuHidden] = React.useState(true);

    const toggleMenu = () => {
    setMenuHidden(!isMenuHidden);
    };

    const saveGCode = () =>{
        const address = "http://192.168.0.87:3010/gcode";
    
      // Erfassen Sie den Programmnamen (name) und die Positionsdaten (items)
      const programData = {
        name: name, // Stellen Sie sicher, dass name in Ihrer Komponente verfügbar ist
        content: sharedString // Stellen Sie sicher, dass items in Ihrer Komponente verfügbar ist
      };
    
      console.log(programData);
    
      axios.post(address, programData)
        .then(response => {
          console.log('Daten erfolgreich gesendet:', response.data);
          // Fügen Sie hier den Code hinzu, um den gespeicherten Datensatz in Ihrer Komponente zu verarbeiten oder anzuzeigen, falls erforderlich.
        })
        .catch(error => {
          console.error('Fehler beim Senden der Daten:', error);
        });
    }

    const handleNameChange = (e) =>{
        setName(e.target.value)
    }

    


    return(
        <>
            <div style={{ backgroundColor: props.color }} className="p-4 text-white rounded-xl font-bold  mt-10 mx-5 flex items-center justify-between border-4 border-black">
          <div className="text-3xl sm:text-l ">G-Code</div>
          <div className='flex space-x-2 '>
            
            <button className="sm:hidden" id="burgerheader" onClick={toggleMenu}>
              <img src="Burgermenu.png" className="object-contain object-center w-10 h-10"></img>
            </button>
            <div className={`hidden sm:flex space-x-2`} id="menu">
            
            <button
              className="px-4 py-2 border-2 border-white rounded hover:bg-black"
              onClick={() => { GCodemode === 4 ? updateModeGCode(0) : updateModeGCode(4) }}
              >
              {GCodemode === 4 ?
               <img src="Closeicon.png" className="object-contain object-center w-10 h-10"/> :
               <img src="Loadprogrammicon.png" className="object-contain object-center w-10 h-10"/>}
            </button>
            <button
              className="px-4 py-2 border-2 border-white rounded hover:bg-black"
              onClick={() => { GCodemode === 2 ? updateModeGCode(0) : updateModeGCode(2) }}
            >
              {GCodemode === 2 ?
               <img src="Closeicon.png" className="object-contain object-center w-10 h-10"/> :
               <img src="GCode.png" className="object-contain object-center w-10 h-10"/>}
            </button>
            <button
              className="px-4 py-2 border-2 border-white rounded hover:bg-black" onClick={() => { GCodemode === 3 ? updateModeGCode(0) : updateModeGCode(3) }}>
              {GCodemode === 3 ?
               <img src="Closeicon.png" className="object-contain object-center w-10 h-10"/> :
               <img src="Saveicon.png" className="object-contain object-center w-10 h-10"/>}
            </button>
            <button
            className="px-4 py-2 border-2 border-white rounded hover:bg-black"
            onClick={() => GCodemode === 1 ? updateModeGCode(0) : updateModeGCode(1)}
                >
            {GCodemode === 1 ?
                <img src="Closeicon.png" className="object-contain object-center w-10 h-10"/> :
                <img src="Deployicon.png" className='$ object-contain object-center w-10 h-10 '/>}
            </button>
            </div>
          </div>
        </div>
        {isMenuHidden === true && (
        <div style={{ backgroundColor: props.color }} className="sm:hidden mx-5 p-4 border-4 border-black rounded-2xl flex flex-row items-center justify-between">
            <div className="flex flex-row justify-between items-center w-full">
                <div className={"md:hidden"} id="menu">
                
                <button
                className="px-2 smm:px-4 py-2 border-2 border-white rounded hover:bg-black"
                onClick={() => { GCodemode === 4 ? updateModeGCode(0) : updateModeGCode(4) }}
                >
                {GCodemode === 4 ?
                <img src="Closeicon.png" className="object-contain object-center w-10 h-10"/> :
                <img src="Loadprogrammicon.png" className="object-contain object-center w-10 h-10"/>}
                </button>
                <button
                className="px-2 smm:px-4 py-2 border-2 border-white rounded hover:bg-black"
                onClick={() => { GCodemode === 2 ? updateModeGCode(0) : updateModeGCode(2) }}
                >
                {GCodemode === 2 ?
                <img src="Closeicon.png" className="object-contain object-center w-10 h-10"/> :
                <img src="GCode.png" className="object-contain object-center w-10 h-10"/>}
                </button>
                <button
                className="px-2 smm:px-4 py-2 border-2 border-white rounded hover:bg-black" onClick={() => { GCodemode === 3 ? updateModeGCode(0) : updateModeGCode(3)}}>
                {GCodemode === 3 ?
                <img src="Closeicon.png" className="object-contain object-center w-10 h-10"/> :
                <img src="Saveicon.png" className="object-contain object-center w-10 h-10"/>}
                </button>
                <button
                    className="px-2 smm:px-4 py-2 border-2 border-white rounded hover:bg-black"
                    onClick={() => GCodemode === 1 ? updateModeGCode(0) : updateModeGCode(1)}
                >
                    {GCodemode === 1 ?
                        <img src="Closeicon.png" className="object-contain object-center w-10 h-10" /> :
                        <img src="Deployicon.png" className='$ object-contain object-center w-10 h-10 ' />}
                </button>
                </div>
            </div>
        </div>)}

        {GCodemode === 3 && (
          <div style={{ backgroundColor: props.color }} className="mx-5 p-4 border-4 border-black rounded-2xl flex items-center justify-between">
            <div className="flex flex-col sm:flex-row justify-between items-center w-full">
              <label className="text-white mb-2 mr-2 font-bold">Speichern Unter:</label>
              <input
                onChange={handleNameChange}
                className="text-black rounded p-2 flex-grow sm:mr-5 mt-4 mb-4 sm:mt-0 sm:mb-0" // Abstand zwischen Eingabefeld und Button
              />
              <button
                className="px-4 py-2 border-2 border-white text-white text-l font-bold rounded hover:bg-black"
                onClick={saveGCode}
              >
                Speichern
              </button>
            </div>
  
          </div>
        )}
        {GCodemode === 2 &&(
            <TextEditor color={props.color} menuState={props.menuState}/>
        )}
        {GCodemode === 1 &&(
            <div style={{ backgroundColor: props.color }} className="mx-5 p-4 border-4 border-black rounded-2xl flex flex-row items-center justify-between">
            <button
              className="flex w-full justify-center items-center px-4 py-2 border-2 border-white text-white bg-red-700 font-bold rounded hover:bg-black"
            >
              Laden
            </button>
          </div>
          
        )}
        {GCodemode === 4 &&(
           <LoadProgrammList color={props.color}/>
        )}
        </>
    )

}


export default GCode