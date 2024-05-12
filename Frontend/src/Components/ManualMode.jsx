import React, { useState, useEffect, useRef } from "react";
import Slider from "./Slider";
import axios from "axios";
import { useRecoilState, useRecoilValue } from "recoil";
import {
  xValueAtom,
  yValueAtom,
  zValueAtom,
  phiValueAtom,
  actuatorAtom,
  settingAtom
} from "../utils/atoms";
// ManuellMode component responsible for manual control mode
const ManuellMode = () => {
  const settings = useRecoilValue(settingAtom);
  const server = process.env.REACT_APP_API_URL;
  const [xValue, setXValue] = useRecoilState(xValueAtom);
  const [yValue, setYValue] = useRecoilState(yValueAtom);
  const [zValue, setZValue] = useRecoilState(zValueAtom);
  const [phiValue, setPhiValue] = useRecoilState(phiValueAtom);
  const [actuator, setActuator] = useRecoilState(actuatorAtom);
  const countIntervalRef = useRef(null);
  const isFirstRender = useRef(true);

  // Function to calculate step based on settings.speed
  const calculateStep = () => {
    // Calculates steps from 0.1 to 10 based on settings.speed
    return 0.1 + (settings.speed / 100) * 9.9;
  };
  // Event handler for mouse down event
  const handleMouseDown = (axis, direction) => {
    // Clear interval if already running
    if (countIntervalRef.current) {
      clearInterval(countIntervalRef.current);
    }

    let setter;
    let valueChecker = (value) => value; // Default checker leaving the value unchanged
    // Handle different axes for manual control
    switch (axis) {
      case "x":
        setter = setXValue;
        valueChecker = (value) => {
          const step = calculateStep() * direction; // Consider direction and step size
          const newX = Math.round((parseFloat(value) + step) * 10) / 10; // Rounds to one decimal place
          const newY = yValue;
          return newX * newX + newY * newY <= 40000 ? newX : value;
        };
        break;
      case "y":
        setter = setYValue;
        valueChecker = (value) => {
          const step = calculateStep() * direction;
          const newY = Math.round((parseFloat(value) + step) * 10) / 10; // Rounds to one decimal place
          const newX = xValue;
          return newX * newX + newY * newY <= 40000 ? newY : value;
        };
        break;
      case "z":
        setter = setZValue;
        valueChecker = (value) => {
          const step = calculateStep() * direction;
          const newZ = Math.round((parseFloat(value) + step) * 10) / 10; // Rounds to one decimal place
          return newZ >= -480 && newZ <= -280 ? newZ : value;
        };
        break;
      case "phi":
        setter = setPhiValue;
        valueChecker = (value) => {
          const step = calculateStep() * direction;
          return Math.round((parseFloat(value) + step) * 10) / 10; // Rounds to one decimal place
        };
        break;
      default:
        return;
    }

    countIntervalRef.current = setInterval(() => {
      setter((prevValue) => valueChecker(prevValue));
    }, Math.max(0, 100 - settings.speed * 0.1)); // Speed affects the interval and should not be negative
  };
  // Event handler for mouse up or leave events
  const handleMouseUpOrLeave = () => {
    clearInterval(countIntervalRef.current);
  };
  // Effect hook to send coordinates when values change
  useEffect(() => {
    if (isFirstRender.current) {
      isFirstRender.current = false; // Set to false after the first render
    } else {
      sendCoordinates(); // Only called when any of the values change, not on the first render
    }
  }, [xValue, yValue, zValue, phiValue]);
  // Function to send coordinates to the server
  const sendCoordinates = async () => {
    const coordinates = [
      Math.round(parseFloat(xValue) * 10) / 10,
      Math.round(parseFloat(yValue) * 10) / 10,
      Math.round(parseFloat(zValue) * 10) / 10,
      Math.round(parseFloat(phiValue) * 10) / 10,
    ];
    try {
      const response = await axios.post(
        `${server}/manual/control/coordinates`,
        {
          coordinates,
        }
      );
      console.log("Koordinaten gesendet:", coordinates);
    } catch (error) {
      console.error(
        "Fehler beim Senden der Koordinaten:",
        error.response ? error.response.data : error.message
      );
    }
  };
  // Function to send gripper signals to the server
  const sendGripperSignals = async (gripperValue) => {
    try {
      const response = await axios.post(`${server}/manual/control/gripper`, {
        gripper: gripperValue,
      });
      console.log("Greiferstärke gesendet:", gripperValue);
    } catch (error) {
      console.error(
        "Fehler beim Senden der Greiferstärke:",
        error.response ? error.response.data : error.message
      );
    }
  };
  // Function to update actuator mode and value
  const updateActuator = (newMode, newValue) => {
    setActuator({
      mode: newMode,
      value: Number(newValue),
    });
    sendGripperSignals(Number(newValue));
  };

  // Function to update coordinates while ensuring limits and constraints
  const updateCoordinates = (newXValue, newYValue, newZValue, newPhiValue) => {
    // Limit phi value to the range of -180 to 180
    const adjustedPhiValue = Math.max(-180, Math.min(180, newPhiValue));

    // Limit z value to the range of -480 to -280
    const adjustedZValue = Math.max(-480, Math.min(-280, newZValue));

    // Calculate distance from (0,0) to ensure x and y stay within a circle with radius 200
    const distance = Math.sqrt(newXValue * newXValue + newYValue * newYValue);
    if (distance > 200) {
      // Scale newXValue and newYValue if necessary to stay within the circle
      const scale = 200 / distance;
      newXValue *= scale;
      newYValue *= scale;
    }

    // Set the corrected values
    setXValue(newXValue);
    setYValue(newYValue);
    setZValue(adjustedZValue);
    setPhiValue(adjustedPhiValue);
  };

  // Function to calculate the maximum value for a given value and radius
  const calculateLimit = (value, radius) => {
    return Math.sqrt(radius * radius - value * value);
  };

  //useEffect to load colormode from localstorage
  useEffect(() => {
    let savedSettings = localStorage.getItem("settings");
    if (savedSettings) {
      savedSettings = JSON.parse(savedSettings);
      if (savedSettings.darkMode) {
        console.log(savedSettings.darkMode);
        document.body.classList.add("dark-mode");
      }
    }
  }, []);

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
              onChange={(value) =>
                updateActuator(settings.gripper, Number(value))
              }
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
              min={-calculateLimit(yValue, 200)}
              max={calculateLimit(yValue, 200)}
              externalValue={xValue}
              onChange={(value) =>
                updateCoordinates(value, yValue, zValue, phiValue)
              }
            />
            <Slider
              color={settings.color}
              label="+Y / -Y"
              min={-calculateLimit(xValue, 200)}
              max={calculateLimit(xValue, 200)}
              externalValue={yValue}
              onChange={(value) =>
                updateCoordinates(xValue, value, zValue, phiValue)
              }
            />
            <Slider
              color={settings.color}
              label="+Z / -Z"
              min={-settings.workSpaceHeight / 2 - 380}
              max={settings.workSpaceHeight / 2 - 380}
              externalValue={zValue}
              onChange={(value) =>
                updateCoordinates(xValue, yValue, value, phiValue)
              }
            />
            <Slider
              color={settings.color}
              label="+Phi / -Phi"
              min={-180}
              max={180}
              externalValue={phiValue}
              onChange={(value) =>
                updateCoordinates(xValue, yValue, zValue, value)
              }
            />
          </div>
        )}

        {settings.manualMode === "buttons" && (
          <div className="flex flex-col items-center justify-center p-2 sm:mr-0 border-4 border-black rounded-2xl">
            {/* Up */}
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
            {/* Left right */}
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
            {/* down */}
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
            <span className="select-none text-black text-lg md:hidden">
              {settings.gripper}
            </span>
            <div className="flex flex-row items-center sm:flex-col justify-center border-4 border-black rounded-2xl p-2">
              <span className="hidden md:block select-none text-black text-lg">
                {settings.gripper}
              </span>
              <button
                onClick={() => updateActuator(settings.gripper, -1)}
                style={{ backgroundColor: settings.color }}
                className=" select-none  text-white w-20 h-20 sm:w-32 sm:h-32 rounded-xl mb-2 text-xl mr-2 ml-2 smm:mr-20 smm:ml-20 sm:mr-0 sm:ml-0 flex items-center justify-center hover:bg-black border-4 border-black"
              >
                Vacuum
              </button>

              <button
                onClick={() => updateActuator(settings.gripper, 0)}
                style={{ backgroundColor: settings.color }}
                className=" select-none  text-white w-20 h-20 sm:w-32 sm:h-32 rounded-xl mb-2 text-xl mr-2 ml-2 smm:mr-20 smm:ml-20 sm:mr-0 sm:ml-0 flex items-center justify-center hover:bg-black border-4 border-black"
              >
                Off
              </button>
              <button
                onClick={() => updateActuator(settings.gripper, 1)}
                style={{ backgroundColor: settings.color }}
                className=" select-none  text-white w-20 h-20 sm:w-32 sm:h-32 rounded-xl mb-2 text-xl mr-2 ml-2 smm:mr-20 smm:ml-20 sm:mr-0 sm:ml-0 flex items-center justify-center hover:bg-black border-4 border-black"
              >
                Pressure
              </button>
            </div>
          </div>
        )}
        {settings.gripper === "magnetGripper" && (
          <div>
            <span className="select-none text-black text-lg md:hidden">
              {settings.gripper}
            </span>
            <div className="flex flex-row items-center   sm:flex-col justify-center border-4 border-black rounded-2xl p-2">
              <span className="hidden md:block select-none text-black text-lg">
                {settings.gripper}
              </span>
              <button
                onClick={() => updateActuator(settings.gripper, 1)}
                style={{ backgroundColor: settings.color }}
                className=" select-none  text-white w-20 h-20 sm:w-32 sm:h-32 rounded-xl mb-2 text-xl mr-2 ml-2 smm:mr-20 smm:ml-20 sm:mr-0 sm:ml-0 flex items-center justify-center hover:bg-black border-4 border-black"
              >
                On
              </button>

              <button
                onClick={() => updateActuator(settings.gripper, 0)}
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
              onChange={(value) =>
                updateActuator(settings.gripper, Number(value))
              }
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
              onChange={(e) =>
                updateCoordinates(e.target.value, yValue, zValue, phiValue)
              }
              style={{ backgroundColor: settings.color }}
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
              onChange={(e) =>
                updateCoordinates(xValue, e.target.value, zValue, phiValue)
              }
              style={{ backgroundColor: settings.color }}
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
              onChange={(e) =>
                updateCoordinates(xValue, yValue, e.target.value, phiValue)
              }
              style={{ backgroundColor: settings.color }}
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
              onChange={(e) =>
                updateCoordinates(xValue, yValue, zValue, e.target.value)
              }
              style={{ backgroundColor: settings.color }}
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
              style={{ backgroundColor: settings.color }}
              className="text-white w-20  rounded ml-2"
            />
          </div>
        </div>
      </div>
    </>
  );
};

export default ManuellMode;
