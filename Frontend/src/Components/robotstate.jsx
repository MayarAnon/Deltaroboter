import React, { useState, useEffect } from 'react';

const RobotStateDisplay = () => {
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
    <div className="p-4 max-w-md mx-auto bg-white rounded-xl shadow-md overflow-hidden md:max-w-2xl">
      <div className="md:flex">
        <div className="p-8">
          <div className="uppercase tracking-wide text-sm text-indigo-500 font-semibold">Robot State</div>
          {Object.keys(robotState).length === 0 ? (
            <p className="mt-2 text-gray-500">Waiting for data...</p>
          ) : (
            <div className="mt-2 text-gray-900">
              {Object.entries(robotState).map(([key, value]) => (
                <p key={key} className="text-sm font-medium">
                  {key}: {JSON.stringify(value)}
                </p>
              ))}
            </div>
          )}
        </div>
      </div>
    </div>
  );
};

export default RobotStateDisplay;
