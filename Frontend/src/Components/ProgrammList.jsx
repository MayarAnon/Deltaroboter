import React, { useState,useEffect} from 'react';
import axios from 'axios';
import {HighlightedCode} from './Texteditor'
import { useString } from './StringContext';

const LoadProgrammList = (props) => {
    
    const [programmlist, SetProgrammList] = useState([]);

    
  
  
    useEffect(() => {
      // Hier rufen wir den Express-Endpunkt auf, wenn die Komponente montiert wird
      axios
        .get('http://deltarobot:3010/loadGCodeFiles')
        .then((response) => {
          // Die erhaltenen Daten in den Zustand der Komponente speichern
          console.log(response.data);
          SetProgrammList(response.data);
        })
        .catch((error) => {
          console.error('Fehler beim Abrufen der Daten:', error);
        });
    }, []); // Das leere Array als zweites Argument stellt sicher, dass dieser Effekt nur einmal beim Montieren der Komponente ausgeführt wird
  
    return (
      <>
      {programmlist && programmlist.length > 0 ? (
        programmlist.map((programm) => (
          <LoadProgramm
            color ={props.color}
            name={programm.fileName}
            content = {programm.content}
          />
        ))
      ) : (
        <p>Keine Programme verfügbar.</p>
      )}
    </>
    );
  };


  const LoadProgramm = ({color,name,content}) =>{
    const [expand,SetExpand] = useState(false)
    const [isMenuHidden,setMenuHidden] = useState(false)

    const { sharedString, setSharedString } = useString();

  
    const handleExpandChange =()=>{
      SetExpand(!expand)
    }
    const toggleMenu = () => {
        setMenuHidden(!isMenuHidden);
      };

    const handleLoadProgramm =() =>{
        setSharedString(content);
    }

    const handleDelete = () => {
        const address = `http://deltarobot:3010/deleteGCode?name=${name}`; // Hier name durch den tatsächlichen Dateinamen ersetzen
      
        axios.delete(address)
          .then(response => {
            console.log('Datei erfolgreich gelöscht:', response.data);
            // Hier können Sie Ihren Code hinzufügen, um die Antwort zu verarbeiten oder anzuzeigen, falls erforderlich.
          })
          .catch(error => {
            console.error('Fehler beim Löschen der Datei:', error);
          });
      }
  
    
  
    return(
      <div>
        
          <>
          
          <div style={{ backgroundColor: color }} className="ml-5 mr-5 p-4 mt-5 border-4 border-black rounded-2xl flex items-center justify-between">
                <div className="flex justify-between items-center w-full">
                  <label className=" flex item-center text-xl text-white mb-2 mr-2 font-bold">{name.slice(0, -6)}</label>
                  <div
                    className="rounded p-2 flex-grow mr-5" // Abstand zwischen Eingabefeld und Button
                  />
                  <button className="sm:hidden" id="burgerheader" onClick={toggleMenu}>
                    <img src="Burgermenu.png" className="object-contain object-center w-10 h-10"></img>
                  </button>
                  
                  <button
                    className="hidden sm:block mx-2 px-4 py-2 border-2 border-white text-white text-l font-bold rounded hover:bg-black"
                    onClick={handleLoadProgramm}
                  >
                    Load
                  </button>
                  <button
                    className="hidden sm:block mr-2 px-4 py-2 border-2 border-white text-white text-l font-bold rounded hover:bg-black"
                    onClick={handleExpandChange}
                  >
                    {expand ? 'Schließen' : 'Expandieren'}
                  </button>
                  <button
                    className="hidden sm:block mr-2 px-4 py-2 border-2 bg-red-600 hover:bg-red-700 text-white rounded"
                    onClick={handleDelete}
                  >
                    Delete
                  </button>
                </div>      
          </div>
          {isMenuHidden === true && (
            <div  style={{ backgroundColor: color }} className="sm:hidden mx-5 p-4 border-4 border-black rounded-2xl flex items-center justify-between">
                <div className="flex justify-between items-center w-full">
                 <button
                    className=" px-4 py-2 border-2 border-white text-white text-l font-bold rounded hover:bg-black"
                    onClick={handleLoadProgramm}
                  >
                    Load
                  </button>
                  <button
                    className=" px-4 py-2 border-2 border-white text-white text-l font-bold rounded hover:bg-black"
                    onClick={handleExpandChange}
                  >
                    {expand ? 'Schließen' : 'Expandieren'}
                  </button>
                  <button
                    className=" px-4 py-2 border-2 bg-red-600 hover:bg-red-700 text-white rounded"
                    onClick={handleDelete}
                  >
                    Delete
                  </button>
                </div>
            </div>
          )}
          {expand === true && (
            <div  style={{ backgroundColor: color }} className="p-4 text-white rounded-xl mx-5 border-4 border-black overflow-auto">
                <HighlightedCode text={content}/>
            </div>
          )}
          
          </>
       
      </div>
    );
  }

export default LoadProgrammList