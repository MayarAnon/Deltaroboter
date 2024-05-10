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
      if (settings.darkMode) {
        document.body.classList.add("dark-mode");
      }
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
      motionProfil: settings.motionProfil,
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
    axios
      .delete("http://deltarobot:3010/deleteLogs")
      .then((response) => {
        alert("Erfolg: " + response.data.message);
      })
      .catch((error) => {
        alert(
          "Fehler: " + (error.response?.data?.error || "Unbekannter Fehler")
        );
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
      // Überprüfen, ob der Dark-Modus aktiv ist und entsprechend aktualisieren
      if (document.body.classList.contains("dark-mode")) {
        setSettings((prevSettings) => ({
          ...prevSettings,
          darkMode: true,
        }));
      } else {
        setSettings((prevSettings) => ({
          ...prevSettings,
          darkMode: false,
        }));
      }
    }
    setDarkMode(darkMode + 1);
  };

  const handleMouseDown = () => {
    // Definition des Request-Body
    const requestBody = {
      action: "enable",
    };

    // Senden des POST-Requests an den Server
    fetch("/magnet/control", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(requestBody),
    })
      .then((response) => response.json())
      .then((data) => {
        console.log("Magnet activated:", data.message);
      })
      .catch((error) => {
        console.error("Error activating magnet:", error);
      });
  };

  const handleMouseUp = () => {
    const requestBody = {
      action: "disable",
    };

    fetch("/magnet/control", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(requestBody),
    })
      .then((response) => response.json())
      .then((data) => {
        console.log("Magnet deactivated:", data.message);
      })
      .catch((error) => {
        console.error("Error deactivating magnet:", error);
      });
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
        <div className="flex flex-col md:flex-row items-center justify-start md:space-x-4">
    <label className="my-4 md:my-0">Grippersystem:</label>
    <select
        value={settings.gripper}
        onChange={handleGripper}
        className="ml-2 p-2 bg-black text-white rounded w-full md:w-auto"
    >
        <option value="vacuumGripper">Vacuum Gripper</option>
        <option value="complientGripper">Compliant Gripper</option>
        <option value="parallelGripper">Parallel Gripper</option>
        <option value="magnetGripper">Magnet Gripper</option>
    </select>
    <button
        className="custom-button w-full md:w-auto mt-4 md:mt-0 md:ml-4"
        onMouseDown={handleMouseDown}
        onMouseUp={handleMouseUp}
        onTouchStart={handleMouseDown}
        onTouchEnd={handleMouseUp}
    >
        Deactivate Magnet
    </button>
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
            <option value="SigmoidProfil">SigmoidProfil</option>
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
          <BB8Toggle onClick={toggleDarkMode} />
        </div>
      </div>
      <RobotStateDisplay></RobotStateDisplay>
      <InfoComponent color={settings.color} />
    </>
  );
};

export default SettingsPage;
