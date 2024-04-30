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
  settingAtom
} from "../utils/atoms";

const ManuellMode = () => {
  const [settings, setSettings] = useRecoilState(settingAtom);
  const [xValue, setXValue] = useRecoilState(xValueAtom);
  const [yValue, setYValue] = useRecoilState(yValueAtom);
  const [zValue, setZValue] = useRecoilState(zValueAtom);
  const [phiValue, setPhiValue] = useRecoilState(phiValueAtom);
  const [actuator, setActuator] = useRecoilState(actuatorAtom);
  const countIntervalRef = useRef(null);
  const isFirstRender = useRef(true);
  const calculateStep = () => {
    // Berechnet Schritte von 0.1 bis 10 basierend auf settings.speed
    return 0.1 + (settings.speed / 100) * 9.9;
  };

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
          const step = calculateStep() * direction; // Richtung und Schrittgröße einbeziehen
          const newX = (parseFloat(value) + step).toFixed(1); // Rundet auf eine Nachkommastelle
          const newY = yValue;
          return newX * newX + newY * newY <= 40000 ? newX : value;
        };
        break;
      case "y":
        setter = setYValue;
        valueChecker = (value) => {
          const step = calculateStep() * direction;
          const newY = parseFloat((value + step).toFixed(1)); // Rundet auf eine Nachkommastelle
          const newX = xValue;
          return newX * newX + newY * newY <= 40000 ? newY : value;
        };
        break;
      case "z":
        setter = setZValue;
        valueChecker = (value) => {
          const step = calculateStep() * direction;
          const newZ = parseFloat((value + step).toFixed(1)); // Rundet auf eine Nachkommastelle
          return newZ >= -480 && newZ <= -280 ? newZ : value;
        };
        break;
      case "phi":
        setter = setPhiValue;
        valueChecker = (value) => {
          const step = calculateStep() * direction;
          return parseFloat((value + step).toFixed(1)); // Rundet auf eine Nachkommastelle
        };
        break;
      default:
        return;
    }

    countIntervalRef.current = setInterval(() => {
      setter((prevValue) => valueChecker(prevValue));
    }, Math.max(0, 100 - settings.speed * 0.1)); // Die Geschwindigkeit beeinflusst das Intervall und darf nicht negativ sein
  };

  const handleMouseUpOrLeave = () => {
    clearInterval(countIntervalRef.current);
  };

  useEffect(() => {
    if (isFirstRender.current) {
      isFirstRender.current = false; // Setze auf false nach dem ersten Render
    } else {
      sendCoordinates(); // Wird nur aufgerufen, wenn sich einer der Werte ändert, nicht beim ersten Render
    }
  }, [xValue, yValue, zValue, phiValue]); // Abhängigkeiten des useEffect Hooks

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

  const sendGripperSignals = async (gripperValue) => {
    try {
      const response = await axios.post("http://deltarobot:3010/manual/control/gripper", {
        gripper: gripperValue, // Ändern Sie 'signal' zu 'gripper'
      });
      console.log("Greiferstärke gesendet:", gripperValue);
    } catch (error) {
      console.error(
        "Fehler beim Senden der Greiferstärke:",
        error.response ? error.response.data : error.message
      );
    }
  };
  const updateActuator = (newMode, newValue) => {
    setActuator({
      mode: newMode,
      value: Number(newValue)
    });
    sendGripperSignals(Number(newValue))
  };


  const updateCoordinates = (newXValue, newYValue,newZValue,newPhiValue) => {
    setXValue(newXValue);
    setYValue(newYValue);
    setZValue(newZValue);
    setPhiValue(newPhiValue);
    sendCoordinates()
  };
  return (
    <>
      <div className="flex flex-wrap justify-center mx-2 mt-10 sm:mt-15 gap-4">
        {settings.gripper === "parallelGripper" && (
          <div className=" w-3/4 flex items-center justify-center gap-4 hidden sm:block">
            <Slider
              color={settings.color}
              label="parallel gripper"
              min={0}
              max={100}
              externalValue={actuator.value}
              onChange={(value) => updateActuator(settings.gripper, Number(value))}
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
              onChange={(value) => updateCoordinates(value,yValue,zValue,phiValue)}
            />
            <Slider
              color={settings.color}
              label="+Y / -Y"
              min={-settings.workSpaceRadius * 0.7}
              max={settings.workSpaceRadius * 0.7}
              externalValue={yValue}
              onChange={(value) => updateCoordinates(xValue,value,zValue,phiValue)}
            />
            <Slider
              color={settings.color}
              label="+Z / -Z"
              min={-settings.workSpaceHeight / 2 - 380}
              max={settings.workSpaceHeight / 2 - 380}
              externalValue={zValue}
              onChange={(value) => updateCoordinates(xValue,yValue,value,phiValue)}
            />
            <Slider
              color={settings.color}
              label="+Phi / -Phi"
              min={-180}
              max={180}
              externalValue={phiValue}
              onChange={(value) => updateCoordinates(xValue,yValue,zValue,value)}
            />
          </div>
        )}

        {settings.manualMode === "buttons" && (
          <div className="flex flex-col items-center justify-center p-2 sm:mr-0 border-4 border-black rounded-2xl">
            {/* Oben */}
            <button
              style={{ backgroundColor: settings.color }}
              className="select-none text-white w-20 h-20 sm:w-32 sm:h-32 rounded-xl sm:mb-2 text-3xl flex items-center justify-center hover:bg-black border-4 border-blackborder-4 border-black"
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
                className="select-none mr-10 sm:mr-16 style={{ backgroundColor: settings.color }} text-white w-20 h-20 sm:w-32 sm:h-32 rounded-xl sm:mb-2 text-3xl flex items-center justify-center hover:bg-black border-4 border-black"
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
                className="select-none ml-10 sm:ml-16  text-white w-20 h-20 sm:w-32 sm:h-32 rounded-xl sm:mb-2 text-3xl flex items-center justify-center hover:bg-black border-4 border-black"
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
              className=" select-none text-white w-20 h-20 sm:w-32 sm:h-32 rounded-xl sm:mb-2 text-3xl flex items-center justify-center hover:bg-black border-4 border-black"
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
              className=" select-none text-white w-20 h-20 sm:w-32 sm:h-32 rounded-xl mb-16 text-3xl flex items-center justify-center hover:bg-black border-4 border-black"
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
              className=" select-none text-white  w-20 h-20 sm:w-32 sm:h-32 rounded-xl text-3xl flex items-center justify-center hover:bg-black border-4 border-black"
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
        {settings.manualMode === "buttons" && (
          <div className="flex flex-row items-center justify-center border-4 border-black rounded-2xl p-2">
            {/* Phi + Button */}
            <button
              style={{ backgroundColor: settings.color }}
              className=" select-none text-white w-20 h-20 sm:w-32 sm:h-32  rounded-xl text-3xl mr-2 flex items-center justify-center hover:bg-black  border-4 border-black"
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
              className=" select-none text-white w-20 h-20 sm:w-32 sm:h-32  rounded-xl text-3xl ml-2 flex items-center justify-center hover:bg-black border-4 border-black"
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
        {(settings.gripper === "complientGripper" ||
          settings.gripper === "vacuumGripper") && (
            <div>
              <span className="select-none text-black text-lg md:hidden">{settings.gripper}</span>
          <div className="flex flex-row items-center sm:flex-col justify-center border-4 border-black rounded-2xl p-2">
            <span className="hidden md:block select-none text-black text-lg">{settings.gripper}</span>
            <button
              onClick={() => updateActuator(settings.gripper,-1)}
              style={{ backgroundColor: settings.color }}
              className=" select-none  text-white w-20 h-20 sm:w-32 sm:h-32 rounded-xl mb-2 text-xl mr-2 ml-2 smm:mr-20 smm:ml-20 sm:mr-0 sm:ml-0 flex items-center justify-center hover:bg-black border-4 border-black"
            >
              Vacuum
            </button>

            <button
              onClick={() => updateActuator(settings.gripper,0)}
              style={{ backgroundColor: settings.color }}
              className=" select-none  text-white w-20 h-20 sm:w-32 sm:h-32 rounded-xl mb-2 text-xl mr-2 ml-2 smm:mr-20 smm:ml-20 sm:mr-0 sm:ml-0 flex items-center justify-center hover:bg-black border-4 border-black"
            >
              Off
            </button>
            <button
              onClick={() => updateActuator(settings.gripper,1)}
              style={{ backgroundColor: settings.color }}
              className=" select-none  text-white w-20 h-20 sm:w-32 sm:h-32 rounded-xl mb-2 text-xl mr-2 ml-2 smm:mr-20 smm:ml-20 sm:mr-0 sm:ml-0 flex items-center justify-center hover:bg-black border-4 border-black"
            >
              Pressure
            </button>
          </div>
          </div>
        )}
        {settings.gripper === "magnetGripper"  && (
           <div>
           <span className="select-none text-black text-lg md:hidden">{settings.gripper}</span>
          <div className="flex flex-row items-center   sm:flex-col justify-center border-4 border-black rounded-2xl p-2">
            <span className="hidden md:block select-none text-black text-lg">{settings.gripper}</span>
            <button
              onClick={() => updateActuator(settings.gripper,1)}
              style={{ backgroundColor: settings.color }}
              className=" select-none  text-white w-20 h-20 sm:w-32 sm:h-32 rounded-xl mb-2 text-xl mr-2 ml-2 smm:mr-20 smm:ml-20 sm:mr-0 sm:ml-0 flex items-center justify-center hover:bg-black border-4 border-black"
            >
              On
            </button>

            <button
              onClick={() => updateActuator(settings.gripper,0)}
              style={{ backgroundColor: settings.color }}
              className=" select-none  text-white w-20 h-20 sm:w-32 sm:h-32 rounded-xl mb-2 text-xl mr-2 ml-2 smm:mr-20 smm:ml-20 sm:mr-0 sm:ml-0 flex items-center justify-center hover:bg-black border-4 border-black"
            >
              Off
            </button>
          </div>
          </div>
        )}
        {settings.gripper === "parallelGripper" && (
          <div className=" w-full sm:w-2/3 flex flex-col items-center justify-center gap-4 sm:hidden">
            <Slider
              color={settings.color}
              label="parallel gripper"
              min={0}
              max={100}
              externalValue={actuator.value}
              onChange={(value) => updateActuator(settings.gripper,Number(value))}
            />
          </div>
        )}

        <div className="flex flex-col border-4 border-black rounded-2xl sm:p-4 ">
          <div
            style={{ backgroundColor: settings.color }}
            className=" text-white w-64 h-16 rounded-xl mb-2 mt-5 mr-2 ml-2 smm:mr-20 smm:ml-20 sm:mr-0 sm:ml-0 text-xl flex items-center px-4 hover:bg-black border-4 border-black"
          >
            <span>X Position:</span>
            <input
              type="number"
              value={xValue}
              onChange={(e) => updateCoordinates(e.target.value,yValue,zValue,phiValue)}
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
              onChange={(e) => updateCoordinates(xValue,e.target.value,zValue,phiValue)}
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
              onChange={(e) => updateCoordinates(xValue,yValue,e.target.value,phiValue)}
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
              onChange={(e) => updateCoordinates(xValue,yValue,zValue,e.target.value)}
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
              value={actuator.value}
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
