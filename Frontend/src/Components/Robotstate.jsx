import React, { useState, useEffect } from 'react';
import { useRecoilState } from "recoil";
import { settingAtom } from "../utils/atoms";

const RobotStateDisplay = () => {
  const [settings, setSettings] = useRecoilState(settingAtom);
  const [robotState, setRobotState] = useState(() => {
    const savedState = localStorage.getItem('robotState');
    return savedState ? JSON.parse(savedState) : {};
  });
  const [ws, setWs] = useState(null);
  const [isVisible, setIsVisible] = useState(false);

  useEffect(() => {
    function connect() {
      const websocket = new WebSocket('ws://deltarobot.local:3010');

      websocket.onopen = () => {
        console.log('WebSocket connected');
      };

      websocket.onmessage = (event) => {
        const data = JSON.parse(event.data);
        setRobotState(data);
        localStorage.setItem('robotState', JSON.stringify(data));
      };

      websocket.onerror = (error) => {
        console.error('WebSocket error:', error);
      };

      websocket.onclose = (event) => {
        console.log('WebSocket disconnected', event.reason);
        setTimeout(connect, 5000);
      };

      setWs(websocket);
    }

    connect();

    return () => {
      if (ws) {
        ws.close();
      }
    };
  }, []);

  const toggleVisibility = () => {
    setIsVisible(!isVisible);
  };

  return (
    <div style={{ backgroundColor: settings.color }} className="p-2 text-white rounded-xl mt-2 mx-5 border-4 border-black">
      <button onClick={toggleVisibility} className="bg-blue-600 p-2 rounded-md mb-2">
        {isVisible ? 'Hide Robot State' : 'Show Robot State'}
      </button>
      {isVisible && (
        <div className="flex flex-col md:flex-row">
          <div className="p-2"style={{ width: '80%' }}>
          {Object.keys(robotState).length === 0 ? (
              <p className="mt-2">Waiting for data...</p>
            ) : (
              <div className="ml-5 mb-2">
              {robotState.currentCoordinates && (
                <div className="text-md mb-2">
                <p>Current Coordinates:</p>
                 {robotState.currentCoordinates.map((coord) => coord.toFixed(2)).join(", ")}
                </div>
              )}
              {robotState.currentAngles && (
                <div className="text-md mb-2">
                <p>Current Angles:</p>
                {robotState.currentAngles.map((angle) => angle.toFixed(2)).join(", ")}
                </div>
              )}
              {Object.entries(robotState).filter(([key]) => key !== 'currentCoordinates' && key !== 'currentAngles').map(([key, value]) => (
                <p key={key} className="text-md">
                  {key.charAt(0).toUpperCase() + key.slice(1).toLowerCase()}: {JSON.stringify(value)}
                </p>
              ))}
            </div>
            )}
          </div>
        </div>
      )}
    </div>
  );
}


export default RobotStateDisplay;
