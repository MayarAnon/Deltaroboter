import React from 'react';
import { useRecoilValue, useResetRecoilState } from 'recoil';
import { errorStateAtom } from '../utils/atoms';

const ErrorView = () => {
  const error = useRecoilValue(errorStateAtom);
  const resetError = useResetRecoilState(errorStateAtom);

  const handleReset = () => {
    // Hier könnten Sie Logik zum Neustarten des Roboters implementieren
    resetError();
  };

  if (error.errorCode === 0) return null;

  return (
    <div className="error-container">
      <h2>Fehler: {error.message}</h2>
      <button onClick={handleReset}>Roboter neukalibrieren</button>
    </div>
  );
};

export default ErrorView;
