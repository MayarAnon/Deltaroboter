
import React from 'react';

const InfoComponent = (props) => {
    // Hier würdest du Zustände und Handler-Funktionen definieren
  
    return (
      <div style={{ backgroundColor: props.color }}  className="p-4 text-white font-bold rounded-xl mt-10 mx-5 border-4 border-black">
  
        <div className="mb-2">Version: V0.3 (Testing)</div>
        <div className="border-t border-gray-600 my-2"></div> {/* Divider */}
  
        <div className="mb-2">Team:</div>
        <ul className="list-disc ml-5 mb-2">
          <li>Dennis Roth: Infortronik 2021</li>
          <li>Mayar Hanhon: Infortronik 2021</li>
          <li>Silias Wahl: Maschinenbau 2021</li>
          <li>Daniel Fuchs: Maschinenbau 2021</li>
        </ul>
        <div className="border-t border-gray-600 my-2"></div> {/* Divider */}
  
        <div className="mb-2">Studienarbeiten:</div>
        <ul className="list-disc ml-5 mb-2">
          <li>Entwicklung der Steuerung und Leistungselektronik eines Deltaroboters</li>
          <li>Softwarearchitektur und mechanisches Design eines Deltaroboters</li>
  
        </ul>
  
        <div className="border-t border-gray-600 my-2"></div> {/* Divider */}
  
        <div className="mb-2">Verantwortlicher: Herr Steinert, Robin Geistlinger, Achim Hantschel </div>
        <div className="border-t border-gray-600 my-2"></div> {/* Divider */}
  
        <div className="mb-2">Finanziert durch: DHBW Mosbach</div>
      </div>
    );
  };

  
export default InfoComponent;