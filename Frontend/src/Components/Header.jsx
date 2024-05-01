import React, { useState } from "react";
import ConfirmationModal from "./ConfirmationModal";
import { useRecoilState } from "recoil";
import { settingAtom } from "../utils/atoms";
import { useNavigate } from "react-router-dom";
import BB8Toggle from "./DarkmodeToggle";
import axios from "axios";

const Header = () => {
  const [settings, setSettings] = useRecoilState(settingAtom);
  const [isMenuHidden, setMenuHidden] = React.useState(true);
  const [darkMode, setDarkMode] = React.useState(0); // Zustand für Dark Mode
  const [isModalOpen, setIsModalOpen] = useState(false);
  const [icon, setIcon] = useState(0);
  const navigate = useNavigate();

  const ManualMode = () => {
    navigate("/");
  };
  const SettingsMode = () => {
    navigate("/settings");
  };
  const GCode = () => {
    navigate("/gcode-editor");
  };

  // Event-Handler-Funktion, um den Menüzustand zu ändern
  const toggleMenu = () => {
    setMenuHidden(!isMenuHidden);
  };
  //sendet stop signal an den endpoint
  const sendMotorStop = async () => {
    try {
      const response = await axios.post("/motors/stop", {
        stop: true, // Korrekt formatiertes Objekt
      });
      console.log("Motors stoppen", response.data); // Ausgabe der Serverantwort
    } catch (error) {
      console.error(
        "Fehler beim Senden von MotorStop:",
        error.response ? error.response.data : error.message // Verbesserte Fehlerausgabe
      );
    }
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

  //zum öffnen vom Modalfenster 2 mal auf das logo  klicken
  const toggle = () => {
    setIcon((prevIcon) => {
      const newIconValue = prevIcon + 1;
      if (newIconValue === 2) {
        setIsModalOpen(true);
        return 0;
      }
      return newIconValue;
    });
  };
  //leitet den Nutzer zum debug-mode ;)
  const handleConfirm = () => {
    navigate("/debug-mode");
    setIsModalOpen(false);
  };
  const closeModal = () => {
    setIsModalOpen(false);
  };
  //für berechnung der farbe vom header
  function lightenHexColor(hex, percent) {
    // Zuerst die Hex-Farbe in R, G, B umwandeln
    let r = parseInt(hex.slice(1, 3), 16);
    let g = parseInt(hex.slice(3, 5), 16);
    let b = parseInt(hex.slice(5, 7), 16);
  
    // Helligkeit anpassen
    r = parseInt(r * (100 + percent) / 100);
    g = parseInt(g * (100 + percent) / 100);
    b = parseInt(b * (100 + percent) / 100);
  
    // Sicherstellen, dass die Werte im gültigen Bereich bleiben
    r = Math.min(255, r);
    g = Math.min(255, g);
    b = Math.min(255, b);
  
    // Zurück in Hex konvertieren
    r = r.toString(16).padStart(2, '0');
    g = g.toString(16).padStart(2, '0');
    b = b.toString(16).padStart(2, '0');
  
    return `#${r}${g}${b}`;
  }
  return (
    <>
      <ConfirmationModal
        color={settings.color} // Setze die Farbe nach Bedarf
        isOpen={isModalOpen}
        onClose={closeModal}
        onConfirm={handleConfirm}
        text="Please enter the password to continue."
        requirePassword={true}
        correctPassword="1234" // Setze das korrekte Passwort hier
      />
      <div
        style={{
        background: `linear-gradient(45deg, ${settings.color}, ${lightenHexColor(settings.color,180)})`,
        boxShadow: '0 4px 8px rgba(0, 0, 0, 0.1)',
        borderRadius: '10px'}}
        className={`p-4 text-white rounded-xl font-bold  mt-10 mx-5 flex items-center justify-between border-4 border-black`}
      >
        <div className="text-3xl sm:text-l flex item-center">
          Deltaroboter
          <button className=" inline-block ml-4 " onClick={toggle}>
            <img
              src="Delta.png"
              className="hidden smm:block object-contain object-center w-10 h-10 "
              alt="DeltaPic"
            />
          </button>
        </div>

        <button className="md:hidden" id="burgerheader" onClick={toggleMenu}>
          <img
            src="Burgermenu.png"
            className="object-contain object-center w-10 h-10"
            alt="Menu"
          ></img>
        </button>
        <div className={`hidden md:flex md:space-x-2`} id="menu">
        <button
            onClick={ManualMode}
            className="px-4 py-2 border-2 border-white rounded hover:bg-black"
          >
            <img
              src="JoystickIcon.png"
              className="object-contain object-center w-10 h-10"
              alt="manualMode"
            ></img>
          </button>
          <button
            onClick={GCode}
            className="px-4 py-2 ml-2 border-2 border-white rounded hover:bg-black"
          >
            <img
              src="GCode.png"
              className=" object-contain object-center w-10 h-10"
              alt="GCodeMode"
            ></img>
          </button>
          
          <button
            onClick={SettingsMode}
            className="px-4 py-2 border-2 border-white rounded hover:bg-black"
          >
            <img
              src="Settingsicon.png"
              className=" object-contain object-center w-10 h-10"
              alt="Settings"
            ></img>
          </button>
          <button
            onClick={sendMotorStop}
            className="px-2 py-2 rounded hover:bg-black"
          >
            <img
              src="stopIcon.png"
              className="object-contain object-center w-12 h-12"
              alt="Stop"
            ></img>
          </button>
          <BB8Toggle onClick={toggleDarkMode}></BB8Toggle>
        </div>
      </div>
      {isMenuHidden === true && (
        <div
        style={{
          background: `linear-gradient(45deg, ${settings.color}, ${lightenHexColor(settings.color,180)})`,
          boxShadow: '0 4px 8px rgba(0, 0, 0, 0.1)',
          borderRadius: '10px',
          position: 'relative',
          overflow: 'hidden'}}
          className="md:hidden mx-5 p-4 border-4 border-black rounded-2xl flex items-center justify-between"
        >
          <div className="flex justify-between items-center w-full">
          <button
              onClick={ManualMode}
              className="px-2 smm:px-4 py-2 border-2 border-white rounded hover:bg-black"
            >
              <img
                src="JoystickIcon.png"
                className="object-contain object-center w-10 h-10"
                alt="manualMode"
              ></img>
            </button>
            <button
              onClick={GCode}
              className="px-2 smm:px-4 py-2 ml-2 border-2 border-white rounded hover:bg-black"
            >
              <img
                src="GCode.png"
                className=" object-contain object-center w-10 h-10"
                alt="GCodeMode"
              ></img>
            </button>

            <button
              onClick={SettingsMode}
              className="px-2 smm:px-4 py-2 border-2 border-white rounded hover:bg-black"
            >
              <img
                src="Settingsicon.png"
                className=" object-contain object-center w-10 h-10"
                alt="Settings"
              ></img>
            </button>

            <button
              onClick={sendMotorStop}
              className="px-2 py-2 rounded hover:bg-black"
            >
              <img
                src="stopIcon.png"
                className="object-contain object-center w-10 h-10"
                alt="Stop"
              ></img>
            </button>
            <BB8Toggle onClick={toggleDarkMode}></BB8Toggle>
            
          </div>
        </div>
      )}
    </>
  );
};

export default Header;
