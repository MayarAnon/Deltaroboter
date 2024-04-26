import React, { useState, useEffect } from 'react';

const RobotStateDisplay = () => {
  const [robotState, setRobotState] = useState({});

  useEffect(() => {
    const ws = new WebSocket('ws://deltarobot.local:3010');

    ws.onopen = () => {
      console.log('WebSocket connected');
    };

    ws.onmessage = (event) => {
      const data = JSON.parse(event.data);
      setRobotState(data);
    };

    ws.onerror = (error) => {
      console.error('WebSocket error:', error);
    };

    ws.onclose = () => {
      console.log('WebSocket disconnected');
    };

    return () => {
      ws.close();
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
