import React, { useState, useEffect, useRef } from "react";
import Slider from "./Slider";
import axios from "axios";
import { useRecoilState } from "recoil";
import {
  xValueAtom,
  yValueAtom,
  zValueAtom,
  phiValueAtom,
  actuatorAtom,
  settingAtom,
} from "../utils/atoms";
const ManuellMode = () => {
  const [settings, setSettings] = useRecoilState(settingAtom);
  const [xValue, setXValue] = useRecoilState(xValueAtom);
  const [yValue, setYValue] = useRecoilState(yValueAtom);
  const [zValue, setZValue] = useRecoilState(zValueAtom);
  const [phiValue, setPhiValue] = useRecoilState(phiValueAtom);
  const [actuator, setActuator] = useRecoilState(actuatorAtom);

  const countIntervalRef = useRef(null);

  const handleMouseDown = (axis, direction) => {
    if (countIntervalRef.current) {
      clearInterval(countIntervalRef.current);
    }

    let setter;
    let valueChecker = (value) => value; // Standard-Checker, der den Wert unverändert lässt

    switch (axis) {
      case "x":
        setter = setXValue;
        valueChecker = (value) => {
          const newY = yValue;
          const newX = value + direction;
          return newX * newX + newY * newY <= 40000 ? newX : value; // Begrenzung des Kreisradius
        };
        break;
      case "y":
        setter = setYValue;
        valueChecker = (value) => {
          const newX = xValue;
          const newY = value + direction;
          return newX * newX + newY * newY <= 40000 ? newY : value; // Begrenzung des Kreisradius
        };
        break;
      case "z":
        setter = setZValue;
        valueChecker = (value) => {
          const newZ = value + direction;
          return newZ >= -480 && newZ <= -280 ? newZ : value; // Begrenzung der Z-Werte
        };
        break;
      case "phi":
        setter = setPhiValue;
        valueChecker = (value) => value + direction; // Für phi könnte eine andere Art von Begrenzung notwendig sein
        break;
      default:
        return;
    }

    countIntervalRef.current = setInterval(() => {
      setter((prevValue) => valueChecker(prevValue));
    }, Math.max(0, 100 - settings.speed)); // Die Geschwindigkeit beeinflusst das Intervall und darf nicht negativ sein
  };

  const handleMouseUpOrLeave = () => {
    clearInterval(countIntervalRef.current);
    sendCoordinates(); // Aufruf der sendCoordinates Funktion, wenn die Maus losgelassen wird
  };

  const sendCoordinates = async () => {
    const coordinates = [
      parseFloat(xValue),
      parseFloat(yValue),
      parseFloat(zValue),
      parseFloat(phiValue),
    ];
    try {
      const response = await axios.post("/manual/control/coordinates", {
        coordinates,
      });
      console.log("Koordinaten gesendet:", coordinates);
    } catch (error) {
      console.error(
        "Fehler beim Senden der Koordinaten:",
        error.response ? error.response.data : error.message
      );
    }
  };

  return (
    <>
      <div className="flex flex-wrap justify-center mx-5 mt-10 sm:mt-32 gap-4">
        {settings.gripper === "3option" && (
          <div className=" w-2/3  flex-col items-center justify-center gap-4 hidden sm:block">
            <Slider
              color={settings.color}
              label="parallel gripper"
              min={0}
              max={100}
              externalValue={parseInt(actuator, 10)}
              onChange={(value) => setActuator(value.toString())}
            />
          </div>
        )}

        {settings.manualMode === "sliders" && (
          // Adjust the width of this container if necessary, or use w-full to take the full width
          <div className=" w-full sm:w-2/3 flex flex-col items-center justify-center gap-4 ">
            {" "}
            {/* Adjust this class to control the width */}
            <Slider
              color={settings.color}
              label="+X / -X"
              min={-settings.workSpaceRadius * 0.7}
              max={settings.workSpaceRadius * 0.7}
              externalValue={xValue}
              onChange={(value) => setXValue(value)}
            />
            <Slider
              color={settings.color}
              label="+Y / -Y"
              min={-settings.workSpaceRadius * 0.7}
              max={settings.workSpaceRadius * 0.7}
              externalValue={yValue}
              onChange={(value) => setYValue(value)}
            />
            <Slider
              color={settings.color}
              label="+Z / -Z"
              min={-settings.workSpaceHeight / 2 - 380}
              max={settings.workSpaceHeight / 2 - 380}
              externalValue={zValue}
              onChange={(value) => setZValue(value)}
            />
            <Slider
              color={settings.color}
              label="+Phi / -Phi"
              min={-180}
              max={180}
              externalValue={phiValue}
              onChange={(value) => setPhiValue(value)}
            />
          </div>
        )}

        {settings.manualMode === "buttons" && (
          <div className="flex flex-col items-center justify-center p-2 sm:mr-0 border-4 border-black rounded-2xl">
            {/* Oben */}
            <button
              style={{ backgroundColor: settings.color }}
              className=" text-white w-20 h-20 sm:w-32 sm:h-32 rounded-xl sm:mb-2 text-3xl flex items-center justify-center hover:bg-black border-4 border-blackborder-4 border-black"
              onMouseDown={() => handleMouseDown("y", 1)}
              onMouseUp={handleMouseUpOrLeave}
              onMouseLeave={handleMouseUpOrLeave}
              onTouchStart={() => handleMouseDown("y", 1)}
              onTouchEnd={handleMouseUpOrLeave}
            >
              +Y
            </button>
            {/* Links und Rechts */}
            <div className="flex">
              <button
                style={{ backgroundColor: settings.color }}
                className=" mr-10 sm:mr-16 style={{ backgroundColor: settings.color }} text-white w-20 h-20 sm:w-32 sm:h-32 rounded-xl sm:mb-2 text-3xl flex items-center justify-center hover:bg-black border-4 border-black"
                onMouseDown={() => handleMouseDown("x", -1)}
                onMouseUp={handleMouseUpOrLeave}
                onMouseLeave={handleMouseUpOrLeave}
                onTouchStart={() => handleMouseDown("x", -1)}
                onTouchEnd={handleMouseUpOrLeave}
              >
                -X
              </button>
              <button
                style={{ backgroundColor: settings.color }}
                className="ml-10 sm:ml-16  text-white w-20 h-20 sm:w-32 sm:h-32 rounded-xl sm:mb-2 text-3xl flex items-center justify-center hover:bg-black border-4 border-black"
                onMouseDown={() => handleMouseDown("x", 1)}
                onMouseUp={handleMouseUpOrLeave}
                onMouseLeave={handleMouseUpOrLeave}
                onTouchStart={() => handleMouseDown("x", 1)}
                onTouchEnd={handleMouseUpOrLeave}
              >
                +X
              </button>
            </div>
            {/* Unten */}
            <button
              style={{ backgroundColor: settings.color }}
              className=" text-white w-20 h-20 sm:w-32 sm:h-32 rounded-xl sm:mb-2 text-3xl flex items-center justify-center hover:bg-black border-4 border-black"
              onMouseDown={() => handleMouseDown("y", -1)}
              onMouseUp={handleMouseUpOrLeave}
              onMouseLeave={handleMouseUpOrLeave}
              onTouchStart={() => handleMouseDown("y", -1)}
              onTouchEnd={handleMouseUpOrLeave}
            >
              -Y
            </button>
          </div>
        )}
        {settings.manualMode === "buttons" && (
          <div className=" flex flex-col justify-center border-4 border-black rounded-2xl p-2">
            {/* +z Button */}
            <button
              style={{ backgroundColor: settings.color }}
              className=" text-white w-20 h-20 sm:w-32 sm:h-32 rounded-xl mb-16 text-3xl flex items-center justify-center hover:bg-black border-4 border-black"
              onMouseDown={() => handleMouseDown("z", 1)}
              onMouseUp={handleMouseUpOrLeave}
              onMouseLeave={handleMouseUpOrLeave}
              onTouchStart={() => handleMouseDown("z", 1)}
              onTouchEnd={handleMouseUpOrLeave}
            >
              +Z
            </button>
            {/* -z Button */}
            <button
              style={{ backgroundColor: settings.color }}
              className="  text-white  w-20 h-20 sm:w-32 sm:h-32 rounded-xl text-3xl flex items-center justify-center hover:bg-black border-4 border-black"
              onMouseDown={() => handleMouseDown("z", -1)}
              onMouseUp={handleMouseUpOrLeave}
              onMouseLeave={handleMouseUpOrLeave}
              onTouchStart={() => handleMouseDown("z", -1)}
              onTouchEnd={handleMouseUpOrLeave}
            >
              -Z
            </button>
          </div>
        )}

        {/* Neue Tasten für +phi und -phi */}
        {settings.manualMode === "buttons" && (
          <div className="flex flex-row items-center justify-center border-4 border-black rounded-2xl p-2">
            {/* Phi + Button */}
            <button
              style={{ backgroundColor: settings.color }}
              className=" text-white w-20 h-20 sm:w-32 sm:h-32  rounded-xl text-3xl mr-2 flex items-center justify-center hover:bg-black  border-4 border-black"
              onMouseDown={() => handleMouseDown("phi", 1)}
              onMouseUp={handleMouseUpOrLeave}
              onMouseLeave={handleMouseUpOrLeave}
              onTouchStart={() => handleMouseDown("phi", 1)}
              onTouchEnd={handleMouseUpOrLeave}
            >
              Phi +
            </button>
            {/* Phi - Button */}
            <button
              style={{ backgroundColor: settings.color }}
              className=" text-white w-20 h-20 sm:w-32 sm:h-32  rounded-xl text-3xl ml-2 flex items-center justify-center hover:bg-black border-4 border-black"
              onMouseDown={() => handleMouseDown("phi", -1)}
              onMouseUp={handleMouseUpOrLeave}
              onMouseLeave={handleMouseUpOrLeave}
              onTouchStart={() => handleMouseDown("phi", -1)}
              onTouchEnd={handleMouseUpOrLeave}
            >
              Phi -
            </button>
          </div>
        )}
        {(settings.gripper === "1option" || settings.gripper === "2option") && (
          <div className="flex flex-row items-center   sm:flex-col justify-center border-4 border-black rounded-2xl p-2">
            <button
              onClick={() => setActuator("On")}
              style={{ backgroundColor: settings.color }}
              className="   text-white w-20 h-20 sm:w-32 sm:h-32 rounded-xl mr-2 sm:mr-0 sm:mb-16 text-3xl flex items-center justify-center hover:bg-black border-4 border-black"
            >
              On
            </button>

            <button
              onClick={() => setActuator("Off")}
              style={{ backgroundColor: settings.color }}
              className="  text-white  w-20 h-20 sm:w-32 sm:h-32 rounded-xl ml-2 sm:ml-0 text-3xl flex items-center justify-center hover:bg-black border-4 border-black"
            >
              Off
            </button>
          </div>
        )}
        {settings.gripper === "3option" && (
          <div className=" w-full sm:w-2/3 flex flex-col items-center justify-center gap-4 sm:hidden">
            <Slider
              color={settings.color}
              label="parallel gripper"
              min={0}
              max={100}
              externalValue={parseInt(actuator, 10)}
              onChange={(value) => setActuator(value.toString())}
            />
          </div>
        )}

        <div className="flex flex-col border-4 border-black rounded-2xl sm:p-4 ">
          {/* Container für die Anzeige der Werte */}
          <div
            style={{ backgroundColor: settings.color }}
            className=" text-white w-64 h-16 rounded-xl mb-2 mt-5 mr-2 ml-2 smm:mr-20 smm:ml-20 sm:mr-0 sm:ml-0 text-xl flex items-center px-4 hover:bg-black border-4 border-black"
          >
            
              <span>X Position:</span>
            <input
              type="number"
              value={xValue}
              onChange={(e) => setXValue(e.target.value)}
              style={{ backgroundColor: settings.color }} // Markiert das Feld als schreibgeschützt
              className="text-white w-20  rounded ml-2"
            />
          </div>
          <div
            style={{ backgroundColor: settings.color }}
            className=" text-white w-64 h-16 rounded-xl mb-2 text-xl mr-2 ml-2 smm:mr-20 smm:ml-20 sm:mr-0 sm:ml-0 flex items-center px-4 hover:bg-black border-4 border-black"
          >
            
              <span>Y Position:</span>
            <input
              type="number"
              value={yValue}
              onChange={(e) => setYValue(e.target.value)}
              style={{ backgroundColor: settings.color }} // Markiert das Feld als schreibgeschützt
              className="text-white w-20  rounded ml-2"
            />
          </div>
          <div
            style={{ backgroundColor: settings.color }}
            className=" text-white w-64 h-16 rounded-xl mb-2 text-xl mr-2 ml-2 smm:mr-20 smm:ml-20 sm:mr-0 sm:ml-0 flex items-center px-4 hover:bg-black border-4 border-black"
          >
            
              <span>Z Position:</span>
           
            <input
              type="number"
              value={zValue}
              onChange={(e) => setZValue(e.target.value)}
              style={{ backgroundColor: settings.color }} // Markiert das Feld als schreibgeschützt
              className="text-white w-20  rounded ml-2"
            />
          </div>
          <div
            style={{ backgroundColor: settings.color }}
            className=" text-white w-64 h-16 rounded-xl mb-2 text-xl mr-2 ml-2 smm:mr-20 smm:ml-20 sm:mr-0 sm:ml-0 flex items-center px-4 hover:bg-black border-4 border-black"
          >
            
              <span>Phi:</span>
            <input
              type="number"
              value={phiValue}
              onChange={(e) => setPhiValue(e.target.value)}
              style={{ backgroundColor: settings.color }} // Markiert das Feld als schreibgeschützt
              className="text-white w-20  rounded ml-2"
            />
          </div>
          <div
            style={{ backgroundColor: settings.color }}
            className=" text-white w-64 h-16 rounded-xl mb-5 text-xl mr-2 ml-2 smm:mr-20 smm:ml-20 sm:mr-0 sm:ml-0 flex items-center px-4 hover:bg-black border-4 border-black"
          >
            Actuator
            <input
              value={actuator}
              readOnly
              style={{ backgroundColor: settings.color }} // Markiert das Feld als schreibgeschützt
              className="text-white w-20  rounded ml-2"
            />
          </div>
        </div>
      </div>
    </>
  );
};

export default ManuellMode;
