import { useEffect } from "react";
import { useSetRecoilState } from "recoil";
import { robotStateAtom, pathPointsAtom, errorStateAtom } from '../utils/atoms';
import * as THREE from "three";

const WebSocketConnection = () => {
  const setRobotState = useSetRecoilState(robotStateAtom);
  const setPathPoints = useSetRecoilState(pathPointsAtom);
  const setErrorState = useSetRecoilState(errorStateAtom);
  useEffect(() => {
    let websocket = new WebSocket(process.env.REACT_APP_WEBSOCKET_URL);

    websocket.onopen = () => console.log("WebSocket connected");
    websocket.onmessage = event => {
      const data = JSON.parse(event.data);


      //error topic handling
      if (data.Error !== undefined) {
        let errorMessage = '';
        switch (data.Error) {
          case 0:
            errorMessage = ''; // Kein Fehler
            break;
          case 1:
            errorMessage = 'Emergency stop activated'; // Notaus wurde betätigt
            break;
          case 2:
            errorMessage = 'Collision with end switch'; // Kollision mit dem Endscha
            break;
          case 3:
            errorMessage = 'Please calibrate the robot'; // Kollision mit dem Endscha
            break;
          default:
            errorMessage = 'An error has occurred'; // Ein unbekannter Fehler
            break;
        }
      
        setErrorState({ errorCode: data.Error, message: errorMessage });
      }else{
        setErrorState({ errorCode: 3, message: "Please calibrate the robot" });
      }

      if (data.Error === 0) {
        setRobotState(data);
        if (data.currentCoordinates.length > 0) {
          setPathPoints(prev => [
            ...prev,
            new THREE.Vector3(...data.currentCoordinates),
          ]);
        }
      }
    };

    websocket.onerror = error => console.error("WebSocket error:", error);
    websocket.onclose = event => {
      console.log("WebSocket disconnected", event.reason);
      setTimeout(() => {
        websocket = new WebSocket(process.env.REACT_APP_WEBSOCKET_URL);
      }, 1000);
    };

    return () => websocket.close();
  }, [setRobotState, setPathPoints, setErrorState]);

  return null;
};

export default WebSocketConnection;
