import React, { useState, useEffect } from 'react';

const Slider = ({ label, min, max, onChange, externalValue, color }) => {
    const [value, setValue] = useState(((min + max) / 2).toFixed(1));

    const percentage = ((value - min) / (max - min)) * 100;

    const handleInputChange = (e) => {
        const newValue = parseFloat(e.target.value).toFixed(1);
        setValue(newValue);
    };

    const handleRelease = (e) => {
        const newValue = parseFloat(e.target.value).toFixed(1);
        setValue(newValue);
        onChange(newValue); // Aktualisiert den Wert nur, wenn der Benutzer die Interaktion beendet
    };

    useEffect(() => {
      setValue(parseFloat(externalValue).toFixed(1)); // Stellt sicher, dass auch externe Änderungen auf 0.1 genau sind
    }, [externalValue]);

    return (
      <div style={{ backgroundColor: color }} className="flex flex-col items-center justify-center p-4 border-4 border-black rounded-2xl h-24 w-full">
        <label className="text-white mb-2">{label}: {value} ({percentage.toFixed(0)}%)</label>
        <input
          type="range"
          min={min}
          max={max}
          value={value}
          step="0.1" // Erlaubt Feinjustierung auf 0.1 Schritte
          className="slider slider-thumb w-full h-2 bg-blue-400 rounded-full cursor-pointer"
          onChange={handleInputChange} // Rundet und aktualisiert den lokalen Wert kontinuierlich beim Verschieben
          onMouseUp={handleRelease} // Fügt das MouseUp-Event hinzu
          onTouchEnd={handleRelease} // Fügt das TouchEnd-Event hinzu für Touch-Geräte
        />
      </div>
    );
};

export default Slider;
