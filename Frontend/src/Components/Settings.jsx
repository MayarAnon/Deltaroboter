import React, { useState, useCallback, useEffect } from "react";
import InfoComponent from "./Info";
import ConfirmationModal from "./ConfirmationModal";
import { useRecoilState } from "recoil";
import { settingAtom } from "../utils/atoms";
import axios from "axios";
import RobotStateDisplay from "./Robotstate";
import BB8Toggle from "./DarkmodeToggle";
const SettingsPage = () => {
  const [settings, setSettings] = useRecoilState(settingAtom);

  const [darkMode, setDarkMode] = React.useState(0); // Zustand für Dark Mode
  useEffect(() => {
    // Speichern der Einstellungen im LocalStorage bei Änderung
    localStorage.setItem("settings", JSON.stringify(settings));
  }, [settings]);

  useEffect(() => {
    // Laden der Einstellungen aus dem LocalStorage beim Initialisieren der Komponente
    const savedSettings = localStorage.getItem("settings");
    if (savedSettings) {
      setSettings(JSON.parse(savedSettings));
      console.log(settings);
    }
  }, []);
  // Lokalen Zustand verwenden, um die temporäre Geschwindigkeit zu speichern
  const [tempSpeed, setTempSpeed] = useState(settings.speed);

  const handleSpeedChange = (e) => {
    setTempSpeed(e.target.value); // Aktualisiere nur den lokalen Zustand
  };

  const handleSpeedChangeComplete = (e) => {
    // Aktualisiere den globalen Zustand, wenn die Maus losgelassen wird oder die Touch-Interaktion endet
    setSettings((prevSettings) => ({
      ...prevSettings,
      speed: parseInt(e.target.value, 10),
    }));
  };

  const handleGripper = useCallback(
    (e) => {
      setSettings((prevSettings) => ({
        ...prevSettings,
        gripper: e.target.value,
      }));
    },
    [settings]
  );

  const handleMotionProfil = useCallback(
    (e) => {
      setSettings((prevSettings) => ({
        ...prevSettings,
        motionProfil: e.target.value,
      }));
    },
    [settings]
  );

  
  const handleManualModeChange = useCallback(
    (e) => {
      setSettings((prevSettings) => ({
        ...prevSettings,
        manualMode: e.target.value,
      }));
    },
    [settings]
  );

  const handleColorChange = useCallback(
    (e) => {
      setSettings((prevSettings) => ({
        ...prevSettings,
        color: e.target.value,
      }));
    },
    [settings]
  );

  const showApiGuide = () => {
    // Implementieren Sie die Logik, um die API-Anleitung anzuzeigen
    console.log("API Anleitung anzeigen");
  };

  useEffect(() => {
    const apiUrl = "http://deltarobot:3010/updateSettings";

    const adjustedSettings = {
      gripperMode: settings.gripper,
      motorSpeed: settings.speed,
      motionProfil : settings.motionProfil,
    };

    axios
      .post(apiUrl, adjustedSettings)
      .then((response) => {
        console.log(
          "Einstellungen erfolgreich aktualisiert:",
          adjustedSettings,
          response.data
        );
      })
      .catch((error) => {
        console.error("Fehler beim Aktualisieren der Einstellungen:", error);
      });
  }, [settings]); // Abhängigkeit, sodass dieser Effekt nur ausgelöst wird, wenn sich `settings` ändert

  const postHomingSignal = async () => {
    try {
      const response = await axios.post("http://deltarobot:3010/homing", {
        active: true, // Hier senden wir immer `true` an den Server
      });
      console.log("Homing signalisiert: " + response.data.message);
    } catch (error) {
      console.error("Fehler beim Senden des Homing-Signals:", error);
      alert("Fehler beim Senden des Homing-Signals.");
    }
  };

  const [isModalOpen, setIsModalOpen] = useState(false);
  const [isModalOpenLog, setIsModalOpenLog] = useState(false);

  const calibratehandleOpenModal = () => {
    setIsModalOpen(true);
  };

  const calibratehandleCloseModal = () => {
    setIsModalOpen(false);
  };

  const calibratehandleConfirm = () => {
    // Bestätigungslogik hier
    postHomingSignal();
    setIsModalOpen(false);
  };

  const deleteloghandleOpenModal = () => {
    setIsModalOpenLog(true);
  };

  const deleteloghandleCloseModal = () => {
    setIsModalOpenLog(false);
  };

  const handleDeleteLogs = () => {
    axios.delete('http://deltarobot:3010/deleteLogs')
      .then(response => {
        alert('Erfolg: ' + response.data.message);
      })
      .catch(error => {
        alert('Fehler: ' + (error.response?.data?.error || 'Unbekannter Fehler'));
      });
  };

  const deleteloghandleConfirm = () => {
    handleDeleteLogs();
    setIsModalOpenLog(false);
  };

  //managet darkmode und regenbogen-mode ;)
  const toggleDarkMode = () => {
    if (darkMode >= 10 && darkMode <= 13) {
      setDarkMode(0);
      document.body.classList.add("dark-mode2");
    } else if (darkMode < 10 || darkMode > 13) {
      document.body.classList.remove("dark-mode2");

      document.body.classList.toggle("dark-mode");
    }
    setDarkMode(darkMode + 1);
  };
  return (
    <>
      <div
        style={{ backgroundColor: settings.color }}
        className="p-4 text-white rounded-xl font-bold  mt-10 mx-5 border-4 border-black"
      >
        <ConfirmationModal
          color={settings.color}
          isOpen={isModalOpen}
          onClose={calibratehandleCloseModal}
          onConfirm={calibratehandleConfirm}
          text={"Calibrate Deltarobot"}
        />
        <ConfirmationModal
          color={settings.color}
          isOpen={isModalOpenLog}
          onClose={deleteloghandleCloseModal}
          onConfirm={deleteloghandleConfirm}
          text={"Delete Log-Files"}
        />
        <div className="mb-4">
          <label>Speed: {settings.speed}%</label>
          <input
            type="range"
            min="1"
            max="100"
            value={tempSpeed}
            onChange={handleSpeedChange}
            onMouseUp={handleSpeedChangeComplete}
            onTouchEnd={handleSpeedChangeComplete}
            className="slider w-full"
          />
        </div>
        <div className="border-t border-gray-600 my-2"></div> {/* Divider */}
        <div className="mb-4">
          <label>Manual Mode:</label>
          <select
            value={settings.manualMode}
            onChange={handleManualModeChange}
            className="ml-2 p-2 bg-black text-white rounded"
          >
            <option value="buttons">Buttons</option>
            <option value="sliders">Sliders</option>
          </select>
        </div>
        <div className="border-t border-gray-600 my-2"></div> {/* Divider */}
        <div className="mb-4">
          <label>Grippersystem:</label>
          <select
            value={settings.gripper}
            onChange={handleGripper}
            className="ml-2 p-2 bg-black text-white rounded"
          >
            <option value="vacuumGripper">vacuum Gripper</option>
            <option value="complientGripper">complient Gripper</option>
            <option value="parallelGripper">parellel Gripper</option>
            <option value="magnetGripper">Magnet Gripper</option>
          </select>
        </div>
        <div className="border-t border-gray-600 my-2"></div> {/* Divider */}
        <div className="mb-4">
          <label>Motion Profil:</label>
          <select
            value={settings.motionProfil}
            onChange={handleMotionProfil}
            className="ml-2 p-2 bg-black text-white rounded"
          >
            <option value="RectangleProfil">RectangleProfil</option>
            <option value="TrapezProfil">TrapezProfil</option>
          </select>
        </div>
        <div className="border-t border-gray-600 my-2"></div> {/* Divider */}
        <div className="mb-4">
          <button
            onClick={calibratehandleOpenModal}
            className="px-4 py-2 bg-green-600 hover:bg-red-700 text-white rounded"
          >
            Calibrate Deltarobot
          </button>
        </div>
        <div className="border-t border-gray-600 my-2"></div> {/* Divider */}
        <div className="mb-4">
          <a href="http://deltarobot:3010/downloadApiGuide" target="_blank">
            <button className="px-4 py-2 bg-blue-600 hover:bg-blue-700 text-white rounded">
              Download API Guide
            </button>
          </a>
        </div>
        <div className="border-t border-gray-600 my-2"></div> {/* Divider */}
        <div className="flex justify-start space-x-4 ">
          <div className="mb-2">
            <a href="http://deltarobot:3010/downloadLogs" target="_blank">
              <button className="px-4 py-2 bg-blue-600 hover:bg-blue-700 text-white rounded">
                Download Log-Files
              </button>
            </a>
          </div>
          
          <div className="mb-2">
            <button
              onClick={deleteloghandleOpenModal}
              className="px-4 py-2 bg-red-600 hover:bg-red-700 text-white rounded"
            >
              Delete Log Files
            </button>
          </div>
        </div>
        <div className="border-t border-gray-600 my-2"></div> {/* Divider */}
        <div>
          <label> Choose Color:</label>
          <button className="px-4 py-2 mt-4 text-white rounded">
            <input
              type="color"
              onChange={handleColorChange}
              value={settings.color}
              className="ml-5 rounded"
            />
          </button>
        </div>
        <div className="border-t border-gray-600 my-2"></div> {/* Divider */}
        <div className="flex items-center">
          <label htmlFor="darkModeToggle" className="mr-14">
            Dark mode:
          </label>
          <BB8Toggle  onClick={toggleDarkMode} />
        </div>
      </div>
      <RobotStateDisplay></RobotStateDisplay>
      <InfoComponent color={settings.color} />
    </>
  );
};

export default SettingsPage;
