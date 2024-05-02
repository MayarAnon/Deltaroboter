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

  useEffect(() => {
    function connect() {
      const websocket = new WebSocket('ws://deltarobot.local:3010');

      websocket.onopen = () => {
        console.log('WebSocket connected');
      };

      websocket.onmessage = (event) => {
        const data = JSON.parse(event.data);
        setRobotState(data);
        localStorage.setItem('robotState', JSON.stringify(data)); // Speichern des neuen Zustands im localStorage
       
      };

      websocket.onerror = (error) => {
        console.error('WebSocket error:', error);
      };

      websocket.onclose = (event) => {
        console.log('WebSocket disconnected', event.reason);
        // Attempt to reconnect every 5 seconds
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

  return (
    <div style={{ backgroundColor: settings.color }} className="p-4 text-white font-bold rounded-xl mt-10 mx-5 border-4 border-black">
      <div className="flex flex-col md:flex-row">
        <div className="p-8 w-full">
          <div className="text-xl font-semibold tracking-wide">Robot State</div>
          <div className="border-t border-gray-600 my-2 w-full"></div> {/* Divider */}
          {Object.keys(robotState).length === 0 ? (
            <p className="mt-2">Waiting for data...</p>
          ) : (
            <div className="ml-5 mb-2">
            {Object.entries(robotState).map(([key, value]) => (
              <p key={key} className="text-md">
                {key.charAt(0).toUpperCase() + key.slice(1).toLowerCase()}: {JSON.stringify(value)}
              </p>
            ))}
          </div>
          )}
        </div>
      </div>
    </div>
  );
  
}  

export default RobotStateDisplay;
