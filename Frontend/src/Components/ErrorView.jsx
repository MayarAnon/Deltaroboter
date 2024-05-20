import React from 'react';
import { useRecoilValue, useResetRecoilState } from 'recoil';
import { errorStateAtom } from '../utils/atoms';
import axios from 'axios';
const ErrorView = () => {
  const server = process.env.REACT_APP_API_URL;
  const error = useRecoilValue(errorStateAtom);

  const postHomingSignal = async () => {
    try {
      const response = await axios.post(`${server}/homing`, {
        active: true, // Hier senden wir immer `true` an den Server
      });
      console.log("Homing signalisiert: " + response.data.message);
    } catch (error) {
      console.error("Fehler beim Senden des Homing-Signals:", error);
      alert("Fehler beim Senden des Homing-Signals.");
    }
    
  };

 

  if (error.errorCode === 0) return null;

  return (
    <div className="error-overlay">
      <div className="error-content">
        <h2>Fehler: {error.message}</h2>
        <h2>Fehlercode: {error.errorCode}</h2>
        <button onClick={postHomingSignal}>Calibrate Deltarobot</button>
      </div>
    </div>
  );
};

export default ErrorView;
