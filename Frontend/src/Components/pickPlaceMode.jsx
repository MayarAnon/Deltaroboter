import React, { useState, useEffect, useRef } from 'react';
import axios from 'axios';
import { DndProvider, useDrag, useDrop } from 'react-dnd';
import { HTML5Backend } from 'react-dnd-html5-backend';
import { TouchBackend } from 'react-dnd-touch-backend';

  
  const LoadProgramm = ({color,name,items,newprogramm}) =>{
    const [expand,SetExpand] = useState(false)
    const [isMenuHidden,setMenuHidden] = useState(false)
  
    const handleExpandChange =()=>{
      SetExpand(!expand)
    }
    console.log(items)
  
    //Weitergabe an Daten Höchste Komponente
    const handleLoadProgramm =()=>{
      newprogramm(items)
      SetExpand(false)
    }

    const toggleMenu = () => {
        setMenuHidden(!isMenuHidden);
      };
  
    const handleDelete = () => {
      const address = `http://192.168.137.2:3010/delete?name=${name}`; // Hier name durch den tatsächlichen Dateinamen ersetzen
    
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
                  <label className=" flex item-center text-xl text-white mb-2 mr-2 font-bold">{name}</label>
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
                    Expand
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
            <div style={{ backgroundColor: color }} className="sm:hidden mx-5 p-4 border-4 border-black rounded-2xl  flex items-center justify-between">
                <div className="flex justify-between items-center w-full">
                 <button
                    className="mx-2 px-4 py-2 border-2 border-white text-white text-l font-bold rounded hover:bg-black"
                    onClick={handleLoadProgramm}
                  >
                    Load
                  </button>
                  <button
                    className="mr-2 px-4 py-2 border-2 border-white text-white text-l font-bold rounded hover:bg-black"
                    onClick={handleExpandChange}
                  >
                    Expand
                  </button>
                  <button
                    className=" mr-2 px-4 py-2 border-2 bg-red-600 hover:bg-red-700 text-white rounded"
                    onClick={handleDelete}
                  >
                    Delete
                  </button>
                </div>
            </div>
          )}
          {expand === true && (
            <>
            {items.map(item => (
              <DisplayProgramm color={color} item={item} />
            ))}
            </>
          )}
          
          </>
       
      </div>
    );
  }
  
  
  
  const LoadProgrammList = ({color, newprogramm }) => {
    // Weitergabe an Daten Höchste Komponente
    const handleNewProgramm = (positions) => {
      newprogramm(positions);
    };
  
    const [programmlist, SetProgrammList] = useState([]);
  
  
    useEffect(() => {
      // Hier rufen wir den Express-Endpunkt auf, wenn die Komponente montiert wird
      axios
        .get('http://192.168.137.2:3010/loadFiles')
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
            color={color}
            name={programm.name}
            items={programm.positions}
            newprogramm={handleNewProgramm}
          />
        ))
      ) : (
        <p>Keine Programme verfügbar.</p>
      )}
    </>
    );
  };

  const DisplayProgramm = ({color,item}) =>{
    
    return(
      <div style={{ backgroundColor: color }} className="flex items-center p-2 mt-2 mx-5 rounded-xl justify-between border-4 border-black">
        <div className="text-white flex-grow">ID:{item.Id} X:{item.xPosition} Y:{item.yPosition} Z:{item.zPosition} phi:{item.zPosition} ton:{item.ton} toff:{item.toff} actuator:{item.actuatorStatus} </div>

      </div> 
    )
  }
  
  
 // Komponente zur Darstellung eines einzelnen Items mit Drag-and-Drop-Funktionalität
const DraggableDisplayItem = ({color, item, index, moveItem, onRemoveItem }) => {
  const ref = useRef(null);
  const [, drop] = useDrop({
    accept: 'item',
    hover(item, monitor) {
      if (!ref.current) {
        return;
      }
      const dragIndex = item.index;
      const hoverIndex = index;
      // Verschiebe das Element nicht auf sich selbst
      if (dragIndex === hoverIndex) {
        return;
      }
      // Bestimme die Bildschirmmitte
      const hoverBoundingRect = ref.current?.getBoundingClientRect();
      const hoverMiddleY = (hoverBoundingRect.bottom - hoverBoundingRect.top) / 2;
      // Bestimme die Mausposition
      const clientOffset = monitor.getClientOffset();
      const hoverClientY = clientOffset.y - hoverBoundingRect.top;
      // Nur wenn die Maus die Mitte überquert, führe den Move aus
      if (dragIndex < hoverIndex && hoverClientY < hoverMiddleY) {
        return;
      }
      if (dragIndex > hoverIndex && hoverClientY > hoverMiddleY) {
        return;
      }
      // Führe den Move aus
      moveItem(dragIndex, hoverIndex);
      item.index = hoverIndex;
    },
  });
  const [{ isDragging }, drag] = useDrag({
    type: 'item',
    item: () => {
      return { id: item.Id, index };
    },
    collect: (monitor) => ({
      isDragging: monitor.isDragging(),
    }),
  });
  drag(drop(ref));

  const opacity = isDragging ? 0.5 : 1;

  return (
    <div  ref={ref} style={{ opacity, backgroundColor: color }} className="flex items-center p-2 mt-2 mx-5 rounded-xl justify-between border-4 border-black">
      <span className="mr-2 cursor-pointer">
        <img alt="Drag icon" src="dragicon.png" className="object-contain object-center w-10 h-10"></img>
      </span>
      <div className="text-white flex-grow">ID:{item.Id} X:{item.xPosition} Y:{item.yPosition} Z:{item.zPosition} phi:{item.phi} ton:{item.ton} toff:{item.toff} actuator:{item.actuatorStatus}</div>
      <button className="px-4 py-2 bg-red-600 hover:bg-red-700 text-white rounded" onClick={onRemoveItem}>Delete</button>
    </div>
  );
};

// Hilfsfunktion, um zu überprüfen, ob es sich um ein Touch-Gerät handelt
// Sie können eine einfachere oder spezifischere Implementierung basierend auf Ihren Bedürfnissen verwenden
const isTouchDevice = () => {
  return 'ontouchstart' in window || navigator.maxTouchPoints > 0 || navigator.msMaxTouchPoints > 0;
};
  
  
  
  
  const PickPlaceMode = ({color,coordinates,items,setItems}) => {
    
    const [fileName,setFileName] = useState('')
    const [state, setState] = useState({
      xPosition: '',
      yPosition: '',
      zPosition: '',
      phi: '',
      actuatorStatus: '',
      ton: '',
      toff: '',
      Id: 0
    });


    const backend = isTouchDevice() ? TouchBackend : HTML5Backend;

    const moveItem = (dragIndex, hoverIndex) => {
      const dragItem = items[dragIndex];
      const newItems = [...items];
      newItems.splice(dragIndex, 1); // Entferne das Element, das gezogen wird
      newItems.splice(hoverIndex, 0, dragItem); // Füge das gezogene Element an der neuen Position ein
      setItems(newItems);
    };

    //Quelle für Dragable Items: https://github.com/colbyfayock/my-final-space-characters/blob/master/src/App.js
    
    const loadFromManual = () =>{
      setState(coordinates)
    }

    const removeItem = (itemId) => {
      setItems(items.filter(item => item.Id !== itemId));
    };

    const handleTakeOver = () => {
      // Erhöhe die ID basierend auf der größten ID in der Liste `items`
      setItems(prevItems => {
        const maxId = prevItems.reduce((max, item) => Math.max(max, item.Id), -1); // Starte mit -1, falls die Liste leer ist
        const newState = { ...state, Id: maxId + 1 }; // Erstelle ein neues state-Objekt mit der erhöhten ID
    
        return [...prevItems, newState]; // Füge das neue state-Objekt der Liste hinzu
      });
    }

    
    

    const [pickplacemode, setPickplacemode] = useState(0);
  
    //Weitergabe an Daten Höchste Komponente
    const handleNewProgramm = (positions)=>{
      setItems(positions)
      console.log(positions)
    }
    


    const updateModePickPlace = (modi) => {
      setPickplacemode(modi);
      console.log(modi)
    }
    


    const [isMenuHidden, setMenuHidden] = React.useState(true);
  
    const toggleMenu = () => {
      setMenuHidden(!isMenuHidden);
    };
    
  
    
  
    const saveProgramm = () => {
      const address = "http://192.168.0.87:3010/programs";
    
      // Erfassen Sie den Programmnamen (name) und die Positionsdaten (items)
      const programData = {
        name: fileName, // Stellen Sie sicher, dass name in Ihrer Komponente verfügbar ist
        positions: items // Stellen Sie sicher, dass items in Ihrer Komponente verfügbar ist
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
    
    
    
  
    return (
      <>
        <div style={{ backgroundColor: color }} className="p-4 text-white rounded-xl font-bold mt-10 mx-5 flex items-center justify-between border-4 border-black">
          <div className="text-3xl sm:text-l ">Programm</div>
          <div className='flex space-x-2 '>
            
            <button className="sm:hidden" id="burgerheader" onClick={toggleMenu}>
              <img src="Burgermenu.png" className="object-contain object-center w-10 h-10"></img>
            </button>
            <div className={`hidden sm:flex space-x-2`} id="menu">
            {items.length === 0 ? null : (
              <button
                className="px-4 py-2 ml-2 border-2 border-white rounded hover:bg-black"
                onClick={() => pickplacemode === 1 ? updateModePickPlace(0) : updateModePickPlace(1)}
              >
                {pickplacemode === 1 ?
                <img src="Closeicon.png" className="object-contain object-center w-10 h-10"/> :
                <img src="Deployicon.png" className='$ object-contain object-center w-10 h-10 '/>}
              </button>
            )}
            <button
              className="px-4 py-2 border-2 border-white rounded hover:bg-black"
              onClick={() => { pickplacemode === 4 ? updateModePickPlace(0) : updateModePickPlace(4) }}
              >
              {pickplacemode === 4 ?
               <img src="Closeicon.png" className="object-contain object-center w-10 h-10"/> :
               <img src="Loadprogrammicon.png" className="object-contain object-center w-10 h-10"/>}
            </button>
            <button
              className="px-4 py-2 border-2 border-white rounded hover:bg-black"
              onClick={() => { pickplacemode === 2 ? updateModePickPlace(0) : updateModePickPlace(2) }}
            >
              {pickplacemode === 2 ?
               <img src="Closeicon.png" className="object-contain object-center w-10 h-10"/> :
               <img src="Addpositionicon.png" className="object-contain object-center w-10 h-10"/>}
            </button>
            <button
              className="px-4 py-2 border-2 border-white rounded hover:bg-black" onClick={() => { pickplacemode === 3 ? updateModePickPlace(0) : updateModePickPlace(3); console.log(items) }}>
              {pickplacemode === 3 ?
               <img src="Closeicon.png" className="object-contain object-center w-10 h-10"/> :
               <img src="Saveicon.png" className="object-contain object-center w-10 h-10"/>}
            </button>
            </div>
          </div>
        </div>
        {isMenuHidden === true && (
        <div style={{ backgroundColor: color }} className="sm:hidden mx-5 p-4 border-4 border-black rounded-2xl  flex flex-row items-center justify-between">
            <div className="flex justify-between items-center w-full">
                <div className={"md:hidden"} id="menu">
                {items.length === 0 ? null : (
                <button
                    className="px-2 smm:px-4 py-2 border-2 border-white rounded hover:bg-black"
                    onClick={() => pickplacemode === 1 ? updateModePickPlace(0) : updateModePickPlace(1)}
                >
                    {pickplacemode === 1 ?
                    <img src="Closeicon.png" className="object-contain object-center w-10 h-10"/> :
                    <img src="Deployicon.png" className='$ object-contain object-center w-10 h-10 '/>}
                </button>
                )}
                <button
                className="px-2 smm:px-4 py-2 border-2 border-white rounded hover:bg-black"
                onClick={() => { pickplacemode === 4 ? updateModePickPlace(0) : updateModePickPlace(4) }}
                >
                {pickplacemode === 4 ?
                <img src="Closeicon.png" className="object-contain object-center w-10 h-10"/> :
                <img src="Loadprogrammicon.png" className="object-contain object-center w-10 h-10"/>}
                </button>
                <button
                className="px-2 smm:px-4 py-2 border-2 border-white rounded hover:bg-black"
                onClick={() => { pickplacemode === 2 ? updateModePickPlace(0) : updateModePickPlace(2) }}
                >
                {pickplacemode === 2 ?
                <img src="Closeicon.png" className="object-contain object-center w-10 h-10"/> :
                <img src="Addpositionicon.png" className="object-contain object-center w-10 h-10"/>}
                </button>
                <button
                className="px-2 smm:px-4 py-2 border-2 border-white rounded hover:bg-black" onClick={() => { pickplacemode === 3 ? updateModePickPlace(0) : updateModePickPlace(3); console.log(items) }}>
                {pickplacemode === 3 ?
                <img src="Closeicon.png" className="object-contain object-center w-10 h-10"/> :
                <img src="Saveicon.png" className="object-contain object-center w-10 h-10"/>}
                </button>
                </div>
            </div>
        </div>)}
        {/* Parameterbereich, der bei Klick sichtbar wird */}
        {pickplacemode === 2 && (
          <div style={{ backgroundColor: color }}  className=" mx-5 p-4 border-4 border-black rounded-2xl">
            <div className="grid grid-cols-2 gap-4">
            <div className="flex flex-col">
                <label className="text-white mb-2">Einschaltverzögerung:</label>
                <input
                  type="number"
                  className="text-black w-full rounded p-2"
                  value = {state.ton}
                  onChange={(e) =>{setState(prevState => ({ ...prevState, ton: e.target.value }))}}
                />
              </div>
              <div className="flex flex-col">
                <label className="text-white mb-2">Ausschaltverzögerung:</label>
                <input
                  type="number"
                  className="text-black w-full rounded p-2"
                  value = {state.toff}
                  onChange={(e) =>{setState(prevState => ({ ...prevState, toff: e.target.value }))}}
                />
              </div>
              <div className="flex flex-col">
                <label className="text-white mb-2">X Position:</label>
                <input
                  type="number"
                  className="text-black w-full rounded p-2"
                  value = {state.xPosition}
                  onChange={(e) =>{setState(prevState => ({ ...prevState, xPosition: e.target.value }))}}
                />
              </div>
              <div className="flex flex-col">
                <label className="text-white mb-2">Y Position:</label>
                <input
                  type="number"
                  className="text-black w-full rounded p-2"
                  value = {state.yPosition}
                  onChange={(e) =>{setState(prevState => ({ ...prevState, yPosition: e.target.value }))}}
                />
              </div>
              <div className="flex flex-col">
                <label className="text-white mb-2">Z Position:</label>
                <input
                  type="number"
                  className="text-black w-full rounded p-2"
                  value = {state.zPosition}
                  onChange={(e) =>{setState(prevState => ({ ...prevState, zPosition: e.target.value }))}}
                />
              </div>
              <div className="flex flex-col">
                <label className="text-white mb-2">Phi:</label>
                <input
                  type="number"
                  className="text-black w-full rounded p-2"
                  value = {state.phi}
                  onChange={(e) =>{setState(prevState => ({ ...prevState, phi: e.target.value }))}}
                />
              </div>
              <div className="flex flex-col col-span-2">
                <label className="text-white mb-2">Actuator Status:</label>
                <input
                  type="text"
                  className="text-black w-full rounded p-2"
                  value = {state.actuatorStatus}
                  onChange={(e) =>{setState(prevState => ({ ...prevState, actuatorStatus: e.target.value }))}}
                />
              </div>
            </div>
            <button
              className="px-4 py-2 mt-4 border-2 border-white rounded bg-black text-white mr-4"
              onClick={loadFromManual}
            >
              Werte aus Manuell Laden
            </button>
            <button
              className="px-4 py-2 mt-4 border-2 border-white rounded bg-black text-white"
              onClick={handleTakeOver}
            >
              Übernehmen
            </button>
          </div>
        )}
       
        {pickplacemode === 1 && (
          <div style={{ backgroundColor: color }} className= " mx-5 p-4 border-4 border-black rounded-2xl  flex flex-row items-center justify-between">
            <div className ="flex flex-row">
                <div>
                    
                    <label className="text-white mb-2 mr-2 font-bold sm:hidden">Wdh:</label>
                    <label className="text-white mb-2 mr-2 font-bold hidden sm:block">Wiederholungen:</label>
                </div>
              <input
                type="number"
                className="text-black rounded p-2"
                style={{ width: '100px' }} // Set a fixed width for the input
                min="1"
              />
            </div>
            <button
              className="px-4 py-2 border-2 border-white text-white bg-red-700 font-boldtext-l font-bold rounded hover:bg-black"
              style={{ marginLeft: 'auto' }} // This will push the button to the right
            >
              Laden
            </button>
          </div>
        )}
  
        {pickplacemode === 3 && (
          <div style={{ backgroundColor: color }} className="mx-5 p-4 border-4 border-black rounded-2xl flex items-center justify-between">
            <div className="flex flex-col sm:flex-row justify-between items-center w-full">
              <label className="text-white mb-2 mr-2 font-bold">Speichern Unter:</label>
              <input
                onChange={(e) =>{setFileName(e.target.value)}}
                className="text-black rounded p-2 flex-grow sm:mr-5 mt-4 mb-4 sm:mt-0 sm:mb-0" // Abstand zwischen Eingabefeld und Button
              />
              <button
                className="px-4 py-2 border-2 border-white text-white text-l font-bold rounded hover:bg-black"
                onClick={saveProgramm}
              >
                Speichern
              </button>
            </div>
  
          </div>
        )}
        
        {pickplacemode === 4 &&(
          <LoadProgrammList color={color} newprogramm={handleNewProgramm}/>
        )}

        {pickplacemode !== 4 &&(
          <>
            <DndProvider backend={backend} options={isTouchDevice() ? { enableMouseEvents: true } : {}}>
              {/* UI-Komponenten und Logik */}
              {items.map((item, index) => (
                <DraggableDisplayItem
                  color={color}
                  key={item.Id}
                  index={index}
                  item={item}
                  moveItem={moveItem}
                  onRemoveItem={() => removeItem(item.Id)}
                />
              ))}
            </DndProvider>
          </>
        )}
      </>
    );
  };

  export default PickPlaceMode