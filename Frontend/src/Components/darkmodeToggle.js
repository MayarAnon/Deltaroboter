import React, { useState } from 'react';
import './BB8Toggle.css'; // Stellen Sie sicher, dass der CSS-Code in einer separaten Datei namens 'BB8Toggle.css' gespeichert ist.

function BB8Toggle({ onClick }) {
  const [isChecked, setIsChecked] = useState(false);

  const handleToggle = () => {
    setIsChecked(!isChecked);
    if (onClick) {
        onClick();
      }
  };

  return (
    <label className="bb8-toggle">
      <input 
        className="bb8-toggle__checkbox" 
        type="checkbox" 
        checked={isChecked} 
        onChange={handleToggle}
        style={{ display: 'none' }} // Die Checkbox wird im React-Stil versteckt.
      />
      <div className="bb8-toggle__container">
        <div className="bb8-toggle__scenery">
          <div className="bb8-toggle__star"></div>
          <div className="bb8-toggle__star"></div>
          <div className="bb8-toggle__star"></div>
          <div className="bb8-toggle__star"></div>
          <div className="bb8-toggle__star"></div>
          <div className="bb8-toggle__star"></div>
          <div className="bb8-toggle__star"></div>
          <div className="tatto-1"></div>
          <div className="tatto-2"></div>
          <div className="gomrassen"></div>
          <div className="hermes"></div>
          <div className="chenini"></div>
          <div className="bb8-toggle__cloud"></div>
          <div className="bb8-toggle__cloud"></div>
          <div className="bb8-toggle__cloud"></div>
        </div>
        <div className={`bb8 ${isChecked ? 'bb8--active' : ''}`}>
          <div className="bb8__head-container">
            <div className="bb8__antenna"></div>
            <div className="bb8__antenna"></div>
            <div className="bb8__head"></div>
          </div>
          <div className="bb8__body"></div>
        </div>
        <div className="artificial__hidden">
          <div className="bb8__shadow"></div>
        </div>
      </div>
    </label>
  );
}

export default BB8Toggle;
