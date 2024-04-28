import React, { useState } from "react";
import ConfirmationModal from "./ConfirmationModal";
import { useRecoilState } from "recoil";
import { settingAtom } from "../utils/atoms";
import { useNavigate } from "react-router-dom";
import BB8Toggle from "./darkmodeToggle"
import axios from "axios";
// Header-Komponente
const Header = () => {
  // State-Hook, um den Anzeigezustand des Menüs zu verwalten
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

  const toggleDarkMode = () => {
    if (darkMode >= 10 && darkMode <= 13) {
      setDarkMode(0);
      document.body.classList.add("dark-mode2");
      console.log("https://www.youtube.com/watch?v=dQw4w9WgXcQ");
    } else if (darkMode < 10 || darkMode > 13) {
      document.body.classList.remove("dark-mode2");

      document.body.classList.toggle("dark-mode");
    }
    setDarkMode(darkMode + 1);
  };

  const toggle = () => {
    setIcon(prevIcon => {
      const newIconValue = prevIcon + 1;
      if (newIconValue === 2) {
        setIsModalOpen(true);
        return 0; 
      }
      return newIconValue;
    });
  };
  const ModalhandleConfirm = () => {
    navigate("/debug-mode");
    setIsModalOpen(false);
  };
  const closeModal = () => {
    setIsModalOpen(false);
  };

  return (
    <>
      <ConfirmationModal
        color={settings.color}
        onConfirm={ModalhandleConfirm}
        isOpen={isModalOpen}
        onClose={closeModal}
      >
      </ConfirmationModal>
      <div
        style={{ backgroundColor: settings.color }}
        className={`p-4 text-white rounded-xl font-bold  mt-10 mx-5 flex items-center justify-between border-4 border-black`}
      >
        <div className="text-3xl sm:text-l flex item-center">
          Deltaroboter
          <button className=" inline-block ml-4 " onClick={toggle}>
            <img
              src="Delta.png"
              className="hidden smm:block object-contain object-center w-10 h-10 "
            />
          </button>
        </div>
        
        <button className="md:hidden" id="burgerheader" onClick={toggleMenu}>
          <img
            src="Burgermenu.png"
            className="object-contain object-center w-10 h-10"
          ></img>
        </button>
        <div className={`hidden md:flex md:space-x-2`} id="menu">
          <button
            onClick={GCode}
            className="px-4 py-2 ml-2 border-2 border-white rounded hover:bg-black"
          >
            <img
              src="GCode.png"
              className=" object-contain object-center w-10 h-10"
            ></img>
          </button>
          <button
            onClick={ManualMode}
            className="px-4 py-2 border-2 border-white rounded hover:bg-black"
          >
            <img
              src="JoystickIcon.png"
              className="object-contain object-center w-10 h-10"
            ></img>
          </button>
          {/*  
          <button onClick={PickPlaceMode} className="px-4 py-2 border-2 border-white rounded hover:bg-black">
          <img src="Addpositionicon.png" className="object-contain object-center w-10 h-10"></img>
            </button>
          */}
          <button
            onClick={SettingsMode}
            className="px-4 py-2 border-2 border-white rounded hover:bg-black"
          >
            <img
              src="Settingsicon.png"
              className=" object-contain object-center w-10 h-10"
            ></img>
          </button>
          <button
            onClick={sendMotorStop}
            className="px-2 py-2 rounded hover:bg-black"
          >
            <img
              src="stopIcon.png"
              className="object-contain object-center w-12 h-12"
            ></img>
          </button>
          {/* <button
            onClick={toggleDarkMode}
            className="px-2 py-2 rounded hover:bg-black"
          >
            <img
              src="Darkmodeicon.png"
              className="object-contain object-center w-10 h-10"
            ></img>
          </button> */}
          <BB8Toggle  onClick={toggleDarkMode}></BB8Toggle>
        </div>
      </div>
      {isMenuHidden === true && (
        <div
          style={{ backgroundColor: settings.color }}
          className="md:hidden mx-5 p-4 border-4 border-black rounded-2xl flex items-center justify-between"
        >
          <div className="flex justify-between items-center w-full">
            <button
              onClick={GCode}
              className="px-2 smm:px-4 py-2 ml-2 border-2 border-white rounded hover:bg-black"
            >
              <img
                src="GCode.png"
                className=" object-contain object-center w-10 h-10"
              ></img>
            </button>

            <button
              onClick={ManualMode}
              className="px-2 smm:px-4 py-2 border-2 border-white rounded hover:bg-black"
            >
              <img
                src="JoystickIcon.png"
                className="object-contain object-center w-10 h-10"
              ></img>
            </button>
            {/*
              <button onClick={PickPlaceMode} className="px-2 smm:px-4 py-2 border-2 border-white rounded hover:bg-black">
                <img src="Addpositionicon.png" className="object-contain object-center w-10 h-10"></img>
              </button>
              */}
            <button
              onClick={SettingsMode}
              className="px-2 smm:px-4 py-2 border-2 border-white rounded hover:bg-black"
            >
              <img
                src="Settingsicon.png"
                className=" object-contain object-center w-10 h-10"
              ></img>
            </button>

            <button
              onClick={sendMotorStop}
              className="px-2 py-2 rounded hover:bg-black"
            >
              <img
                src="stopIcon.png"
                className="object-contain object-center w-10 h-10"
              ></img>
            </button>
            <BB8Toggle  onClick={toggleDarkMode}></BB8Toggle>
            {/* <button
              onClick={toggleDarkMode}
              className="px-2 smm:px-4 py-2 rounded hover:bg-black"
            >
              <img
                src="Darkmodeicon.png"
                className="object-contain object-center w-10 h-10"
              ></img>
            </button> */}
          </div>
        </div>
      )}
    </>
  );
};

export default Header;
