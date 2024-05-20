import  { useEffect } from "react";
import { useSetRecoilState } from "recoil";
import { robotStateAtom, pathPointsAtom,errorStateAtom  } from '../utils/atoms';
import * as THREE from "three";

const WebSocketConnection = () => {
  const setRobotState = useSetRecoilState(robotStateAtom);
  const setPathPoints = useSetRecoilState(pathPointsAtom);
  const setErrorState = useSetRecoilState(errorStateAtom);
  useEffect(() => {
    let websocket = new WebSocket("ws://192.168.0.43:80");

    websocket.onopen = () => console.log("WebSocket connected");
    websocket.onmessage = event => {
      const data = JSON.parse(event.data);
      setRobotState(data);
      if (data.error && data.error !== 0) {
        setErrorState({ errorCode: data.error, message: 'Ein Fehler ist aufgetreten' });
        return;
      }

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
      }, 1000);
    };

    return () => websocket.close();
  }, [setRobotState, setPathPoints, setErrorState]);

  return null;
};

export default WebSocketConnection;
