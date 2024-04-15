import React, { useState, useCallback, useRef, useEffect} from 'react';
import InfoComponent from './Info';
import ConfirmationModal from './ConfirmationModal';

const SettingsPage = (props) => {
  

    const [settings,setSettings] = useState({
      interpolationMode: 'pointToPoint',
      speed: props.speed||50,
      manualMode: props.manuellMode || 'buttons', // Standardwert hinzugefügt
      gripper: props.Gripper || '1option',
      koordinateSystem: props.koordinateSystem || 'kartesisch',
      workSpaceRadius: props.workSpaceRadius || 200,
      workSpaceHeight: props.workSpaceHeight || 200,
      color: props.color || '#1e293b'}) // Standardwert hinzugefügt

  
  
    const handleInterpolationChange = useCallback((e) => {
      setSettings(prevSettings => ({...prevSettings,interpolationMode: e.target.value}))
    },[]);
  
    const handleSpeedChange = useCallback((e) => {
      setSettings(prevSettings => ({...prevSettings,speed: e.target.value}))
    },[]);
  
    const handleKoordinateSystem = useCallback((e) => {
      setSettings(prevSettings => ({...prevSettings,koordinateSystem: e.target.value}))
    },[]);
  
    const handleLanguage = (event) =>{
  
    }
  
    const handleGripper = useCallback((e) => {
      setSettings(prevSettings => ({...prevSettings,gripper: e.target.value}))
    },[]);
  
    const handleManualModeChange = useCallback((e) => {
      setSettings(prevSettings => ({...prevSettings,manualMode: e.target.value}))
    },[]);
  
    const handleWorkSpaceRadius = useCallback((e) => {
      setSettings(prevSettings => ({...prevSettings,workSpaceRadius: e.target.value}))
    },[]);
  
    const handleWorkSpaceHeight = useCallback((e) => {
      setSettings(prevSettings => ({...prevSettings,workSpaceHeight: e.target.value}))
    },[]);

    const handleColorChange = useCallback((e) => {
      setSettings(prevSettings => ({...prevSettings, color: e.target.value}))
    },[]);
  
    const showApiGuide = () => {
      // Implementieren Sie die Logik, um die API-Anleitung anzuzeigen
      console.log('API Anleitung anzeigen');
    };
  
  
    //Schicken an Parent kommponente
    useEffect(() =>{
      props.onSettingsChange(settings)
    },[settings])
  
    const [isModalOpen, setIsModalOpen] = useState(false);
  
    const DatabasehandleOpenModal = () => {
      setIsModalOpen(true);
    };
  
    const DatabasehandleCloseModal = () => {
      setIsModalOpen(false);
    };
  
    const DatabasehandleConfirm = () => {
      // Bestätigungslogik hier
      setIsModalOpen(false);
    };

    const calibratehandleOpenModal = () => {
      setIsModalOpen(true);
    };
  
    const calibratehandleCloseModal = () => {
      setIsModalOpen(false);
    };
  
    const calibratehandleConfirm = () => {
      // Bestätigungslogik hier
      setIsModalOpen(false);
    };
    
    
  
    return (
      <>
        <div style={{ backgroundColor: settings.color }} className="p-4 text-white rounded-xl font-bold  mt-10 mx-5 border-4 border-black">
          <ConfirmationModal color={settings.color} isOpen={isModalOpen} onClose={DatabasehandleCloseModal} onConfirm={DatabasehandleConfirm} text={"Datenbank löschen"} />
          <ConfirmationModal color={settings.color} isOpen={isModalOpen} onClose={calibratehandleCloseModal} onConfirm={calibratehandleConfirm} text={"Deltaroboter Kalibrieren"} />
          <div className="mb-4">
            <label>Interpolationsmodus:</label>
            <select value={settings.interpolationMode} onChange={handleInterpolationChange} className="ml-2 p-2 bg-black text-white rounded">
              <option value="pointToPoint">Point to Point</option>
              <option value="linear">Linear interpolieren</option>
              <option value="circular">Kreisförmig interpolieren</option>
            </select>
          </div>
          <div className="border-t border-gray-600 my-2"></div> {/* Divider */}
          <div className="mb-4">
            <label>Geschwindigkeit: {settings.speed}%</label>
            <input type="range" min="1" max="100" value={settings.speed} onChange={handleSpeedChange} className="slider w-full" />
          </div>
  
          <div className="border-t border-gray-600 my-2"></div> {/* Divider */}
          <div className="mb-4">
            <label>Manueller Modus:</label>
            <select value={settings.manualMode} onChange={handleManualModeChange} className="ml-2 p-2 bg-black text-white rounded">
              <option value="buttons" >Knöpfe</option>
              <option value="sliders" >Schieberegler</option>
            </select>
          </div>
          <div className="border-t border-gray-600 my-2"></div> {/* Divider */}
          <div className="mb-4">
            <label>Greifersystem:</label>
            <select value={settings.gripper} onChange={handleGripper} className="ml-2 p-2 bg-black text-white rounded">
              <option value="1option">Vakuumgreifer</option>
              <option value="2option">Compliant Greifer</option>
              <option value="3option">Mechanischer Parallelgreifer</option>
              <option value="4option">4. Greifer</option>
            </select>
          </div>
          <div className="border-t border-gray-600 my-2"></div> {/* Divider */}
          <div className="mb-4">
            <label>Koordinationsystem:</label>
            <select value={props.koordinateSystem} onChange={handleKoordinateSystem} className="ml-2 p-2 bg-black text-white rounded">
              <option value="kartesisch">Kartesische Koordinaten</option>
              <option value="zylinder">Zylinderkoordinaten</option>
  
            </select>
          </div>
          <div className="border-t border-gray-600 my-2"></div> {/* Divider */}
          
          <div className="grid grid-cols-3 gap-3">

            <div>
                <label className="flex flex-col text-white mb-2 mr-2 font-bold md:hidden">Arbeitsraum:</label>

                <label className=" flex-col text-white mb-2 mr-2 font-bold hidden md:block">Arbeitsraum(Zylinder):</label>
            </div>
            <div className="flex flex-row">
                <div className="flex flex-col">
                <label className="text-white mb-2 w-1/3">Radius r(mm):</label>
                <input
                  type="number"
                  className="text-white w-2/3 rounded p-2 bg-black"
                  value={settings.workSpaceRadius}
                  onChange={handleWorkSpaceRadius}
                  
                />
                </div>
              </div>
              <div className="flex flex-row">
                <div className="flex flex-col">
                <label className="text-white mb-2 w-1/3">Höhe z(mm):</label>
                <input
                  type="number"
                  className="text-white w-2/3 rounded p-2 bg-black"
                  value={settings.workSpaceHeight}
                  onChange={handleWorkSpaceHeight}
                />
                </div>
              </div>
            </div>
          <div className="border-t border-gray-600 my-2"></div> {/* Divider */}
          <div className="mb-4">
            <button onClick={DatabasehandleOpenModal} className="px-4 py-2 bg-red-600 hover:bg-red-700 text-white rounded">
              Datenbank löschen
            </button>
          </div>
          <div className="border-t border-gray-600 my-2"></div> {/* Divider */}
          <div className="mb-4">
            <button onClick={calibratehandleOpenModal} className="px-4 py-2 bg-red-600 hover:bg-red-700 text-white rounded">
              Deltaroboter Kalibrieren
            </button>
          </div>
          <div className="border-t border-gray-600 my-2"></div> {/* Divider */}
          <div className="mb-4">
            <button onClick={showApiGuide} className="px-4 py-2 bg-blue-600 hover:bg-blue-700 text-white rounded">
              API Anleitung
            </button>
          </div>
          <div className="border-t border-gray-600 my-2"></div> {/* Divider */}
          <div>
            <button className="px-4 py-2 mt-4 text-white rounded bg-black" >
              Farbe wählen: <input type="color" onChange={handleColorChange} className="ml-5 rounded" />
            </button>
  
          </div>
  
        </div>
        <InfoComponent color={settings.color}/>
      </>
    );
  };

  export default SettingsPage