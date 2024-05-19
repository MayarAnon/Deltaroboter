import React, { useEffect } from "react";
import { useSetRecoilState } from "recoil";
import { robotStateAtom, pathPointsAtom } from '../utils/atoms';
import * as THREE from "three";

const WebSocketConnection = () => {
  const setRobotState = useSetRecoilState(robotStateAtom);
  const setPathPoints = useSetRecoilState(pathPointsAtom);

  useEffect(() => {
    let websocket = new WebSocket("ws://192.168.0.43:80");

    websocket.onopen = () => console.log("WebSocket connected");
    websocket.onmessage = event => {
      const data = JSON.parse(event.data);
      setRobotState(data);

      if (data.currentCoordinates.length > 0) {
        setPathPoints(prev => [
          ...prev,
          new THREE.Vector3(...data.currentCoordinates),
        ]);
      }
    };

    websocket.onerror = error => console.error("WebSocket error:", error);
    websocket.onclose = event => {
      console.log("WebSocket disconnected", event.reason);
      setTimeout(() => {
        websocket = new WebSocket("ws://192.168.0.43:80");
      }, 5000);
    };

    return () => websocket.close();
  }, [setRobotState, setPathPoints]);

  return null;
};

export default WebSocketConnection;
