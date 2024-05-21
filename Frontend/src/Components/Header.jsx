import React, { useState } from "react";
import ConfirmationModal from "./ConfirmationModal";
import { useRecoilValue } from "recoil";
import { settingAtom } from "../utils/atoms";
import { useNavigate } from "react-router-dom";

import axios from "axios";

const Header = () => {
  const server = process.env.REACT_APP_API_URL;
  const settings = useRecoilValue(settingAtom);
  const [isMenuHidden, setMenuHidden] = React.useState(true);
  const [isModalOpen, setIsModalOpen] = useState(false);
  const [icon, setIcon] = useState(0);
  const navigate = useNavigate();
  // Functions to navigate to different modes
  const ManualMode = () => {
    navigate("/");
  };
  const SettingsMode = () => {
    navigate("/settings");
  };
  const GCode = () => {
    navigate("/gcode-editor");
  };
  const DigitalTwinMode = () => {
    navigate("/digital-twin");
  };

  // Function to toggle menu visibility
  const toggleMenu = () => {
    setMenuHidden(!isMenuHidden);
  };
  // Function to send motor stop signal to the endpoint
  const sendMotorStop = async () => {
    try {
      const response = await axios.post(`${server}/motors/stop`, {
        stop: true, // Correctly formatted object
      });
      console.log("Motors stoppen", response.data); // Output server response
    } catch (error) {
      console.error(
        "Fehler beim Senden von MotorStop:",
        error.response ? error.response.data : error.message
      );
    }
  };

  // Function to toggle modal window by clicking the logo twice
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
  // Function to handle confirmation
  const handleConfirm = () => {
    navigate("/debug-mode");
    setIsModalOpen(false);
  };
  // Function to close the modal
  const closeModal = () => {
    setIsModalOpen(false);
  };
  // Function for calculating header color
  function lightenHexColor(hex, percent) {
    // Convert hex color to RGB
    let r = parseInt(hex.slice(1, 3), 16);
    let g = parseInt(hex.slice(3, 5), 16);
    let b = parseInt(hex.slice(5, 7), 16);

    // Adjust brightness
    r = parseInt((r * (100 + percent)) / 100);
    g = parseInt((g * (100 + percent)) / 100);
    b = parseInt((b * (100 + percent)) / 100);

    // Ensure values stay within valid range
    r = Math.min(255, r);
    g = Math.min(255, g);
    b = Math.min(255, b);

    // Convert back to hex
    r = r.toString(16).padStart(2, "0");
    g = g.toString(16).padStart(2, "0");
    b = b.toString(16).padStart(2, "0");

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
          background: `linear-gradient(45deg, ${settings.color
            }, ${lightenHexColor(settings.color, 180)})`,
          boxShadow: "0 4px 8px rgba(0, 0, 0, 0.1)",
          borderRadius: "10px",
        }}
        className={`p-4 text-white rounded-xl font-bold  mt-10 mx-5 flex items-center justify-between border-4 border-black`}
      >
        <div>
          <div className="flex flex-col items-center sm:flex-row">
            <span className="text-3xl sm:text-l">Deltaroboter</span>

            <button className=" inline-block ml-4 " onClick={toggle}>
              <img
                src="DHBW_Icon.png"
                className="hidden smm:block object-contain object-center w-10 h-10 "
                alt="DeltaPic"
              />
            </button>
          </div>

          <div className="flex items-center mt-1 sm:mt-0 border-1 border-white rounded-lg  "style={{ backgroundColor: 'rgba(255, 255, 255, 0.2)', width: '11rem' }}>
          <div className={`ml-2 w-3 h-3 mr-2 rounded-full ${settings.powerstage ? 'bg-green-600' : 'bg-red-600'}`}  />
            <span className={`text-sm ${settings.powerstage ? 'text-green-600' : 'text-red-600'}`}>
              {settings.powerstage ? 'Powerstage enabled' : 'Powerstage disabled'}
            </span>
            
          </div>
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
            onClick={DigitalTwinMode}
            className=" px-4 py-2 ml-2 border-2 border-white rounded hover:bg-black"
          >
            <img
              src="Delta.png"
              className="object-contain object-center w-10 h-10"
              alt="DeltaPic"
            />
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
        </div>
      </div>
      {isMenuHidden === true && (
        <div
          style={{
            background: `linear-gradient(45deg, ${settings.color
              }, ${lightenHexColor(settings.color, 180)})`,
            boxShadow: "0 4px 8px rgba(0, 0, 0, 0.1)",
            borderRadius: "10px",
            position: "relative",
            overflow: "hidden",
          }}
          className="md:hidden mx-5 p-4 border-4 border-black rounded-2xl flex items-center justify-between"
        >
          <div className="flex justify-between items-center w-full">
            <button
              onClick={ManualMode}
              className="px-1 py-1 border-2 border-white rounded hover:bg-black"
            >
              <img
                src="JoystickIcon.png"
                className="object-contain object-center w-11 h-11"
                alt="manualMode"
              ></img>
            </button>
            <button
              onClick={GCode}
              className="px-1 py-1 ml-2 border-2 border-white rounded hover:bg-black"
            >
              <img
                src="GCode.png"
                className=" object-contain object-center w-11 h-11"
                alt="GCodeMode"
              ></img>
            </button>
            <button
              onClick={DigitalTwinMode}
              className="px-1 py-1 ml-2 border-2 border-white rounded hover:bg-black"
            >
              <img
                src="Delta.png"
                className="object-contain object-center w-11 h-11"
                alt="DeltaPic"
              />
            </button>
            <button
              onClick={SettingsMode}
              className="px-1 py-1 ml-2 border-2 border-white rounded hover:bg-black"
            >
              <img
                src="Settingsicon.png"
                className=" object-contain object-center w-11 h-11"
                alt="Settings"
              ></img>
            </button>

            <button
              onClick={sendMotorStop}
              className="px-1 py-1 rounded hover:bg-black"
            >
              <img
                src="stopIcon.png"
                className="object-contain object-center w-14 h-14"
                alt="Stop"
              ></img>
            </button>
          </div>
        </div>
      )}
    </>
  );
};

export default Header;
