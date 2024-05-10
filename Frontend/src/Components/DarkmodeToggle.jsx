import React, { useState,useEffect } from 'react';
import '../styles/BB8Toggle.css'; // Stellen Sie sicher, dass der CSS-Code in einer separaten Datei namens 'BB8Toggle.css' gespeichert ist.
import { useRecoilState } from "recoil";
import { settingAtom } from "../utils/atoms";

function BB8Toggle({ onClick }) {
  const [settings, setSettings] = useRecoilState(settingAtom);
  const [isChecked, setIsChecked] = useState(settings.darkMode);

  useEffect(() => {
    // Aktualisieren Sie `isChecked`, wenn sich `settings.darkMode` ändert.
    setIsChecked(settings.darkMode);
  }, [settings.darkMode]);

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
