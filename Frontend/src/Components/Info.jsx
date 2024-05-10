import React from "react";
// InfoComponent: Displays information about the project, team members, theses, supervisors, and funding
const InfoComponent = (props) => {
  return (
    <div
      style={{ backgroundColor: props.color }}
      className="p-4 text-white font-bold rounded-xl mt-10 mx-5 border-4 border-black"
    >
      <div className="mb-2">Version: V0.4 (Debugging)</div>
      <div className="border-t border-gray-600 my-2"></div> {/* Divider */}
      <div className="mb-2">Team:</div>
      <ul className="list-disc ml-5 mb-2">
        <li>Dennis Roth: Electrical Engineering (Infortronik) 2021</li>
        <li>Mayar Hanhon: Electrical Engineering (Infortronik) 2021</li>
        <li>Silias Wahl: Mechanical Engineering 2021</li>
        <li>Daniel Fuchs: Mechanical Engineering 2021</li>
      </ul>
      <div className="border-t border-gray-600 my-2"></div> {/* Divider */}
      <div className="mb-2">Theses:</div>
      <ul className="list-disc ml-5 mb-2">
        <li>
          Entwicklung der Steuerung und Leistungselektronik eines Deltaroboters
        </li>
        <li>Softwarearchitektur und mechanisches Design eines Deltaroboters</li>
      </ul>
      <div className="border-t border-gray-600 my-2"></div> {/* Divider */}
      <div className="mb-2">
        Supervised by: Mr. Steinert, Robin Geistlinger, Achim Hantschel{" "}
      </div>
      <div className="border-t border-gray-600 my-2"></div> {/* Divider */}
      <div className="mb-2">Funded by: DHBW Mosbach</div>
    </div>
  );
};

export default InfoComponent;
