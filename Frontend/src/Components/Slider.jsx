import React, { useState, useEffect } from "react";
// Slider: A component to display and control a range input
const Slider = ({ label, min, max, onChange, externalValue, color }) => {
  const [value, setValue] = useState(((min + max) / 2).toFixed(1));
  // Calculate the percentage value of the slider
  const percentage = ((value - min) / (max - min)) * 100;
  // Handler for input change
  const handleInputChange = (e) => {
    const newValue = parseFloat(e.target.value).toFixed(1);
    setValue(newValue);
  };
  // Handler for release event
  const handleRelease = (e) => {
    const newValue = parseFloat(e.target.value).toFixed(1);
    setValue(newValue);
    onChange(newValue); // Update the value only when user interaction ends
  };
  // Effect to update the slider value from external changes
  useEffect(() => {
    setValue(parseFloat(externalValue).toFixed(1)); // Ensure external changes are rounded to 0.1 precision
  }, [externalValue]);

  return (
    <div
      style={{ backgroundColor: color }}
      className="flex flex-col items-center justify-center p-4 border-4 border-black rounded-2xl h-24 w-full"
    >
      <label className="text-white mb-2">
        {label}: {value} ({percentage.toFixed(0)}%)
      </label>
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
