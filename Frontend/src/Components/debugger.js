import React, { useState, useEffect } from "react";

import { Unity, useUnityContext } from "react-unity-webgl";
import "./debugger.css";
const Debugger = () => {
  const { unityProvider } = useUnityContext({
    loaderUrl: "Build/FlappyBird.loader.js",
    dataUrl: "Build/webgl.data",
    frameworkUrl: "Build/build.framework.js",
    codeUrl: "Build/build.wasm",
  });
  const [cursors, setCursors] = useState([]);
   // Direkte Überprüfung der Bildschirmbreite im Render-Zweig
   
  useEffect(() => {
    const interval = setInterval(() => {
      setCursors((cursors) =>
        cursors.map((cursor) => ({
          ...cursor,
          x: cursor.x + (Math.random() * 200 - 100), 
          y: cursor.y + (Math.random() * 200 - 100),
        }))
      );
    }, 100); // Update-Rate von 100ms

    return () => clearInterval(interval); // Bereinigung des Intervalls
  }, []);

  const handleClick = (event) => {
    // Fügt bei jedem Klick ein neues Bild hinzu
    for (let index = 0; index < 100; index++) {
      setCursors((prevCursors) => [
        ...prevCursors,
        {
          x: event.clientX,
          y: event.clientY,
          visible: true,
        },
      ]);
    }
  };
  if (window.innerWidth <= 1024) {
    return null; // Zeigt nichts an, wenn die Bildschirmbreite 1024px oder weniger beträgt
  }else{
  return (
    <>
      {cursors.map(
        (cursor, index) =>
          cursor.visible && (
            <img
              key={index}
              src="\image-removebg-preview.png" // Pfad zum Bild
              className="cursor-image"
              style={{
                left: `${cursor.x}px`,
                top: `${cursor.y}px`,
                display: "block",
                width: "17px",
                height: "17px",
                transform: 'rotate(15deg)'
              }}
              alt="Custom Cursor"
            />
          )
      )}
      <div className="flex flex-col items-center">
        <div className="flex flex-col mt-5 items-center justify-center">
          <Unity
            unityProvider={unityProvider}
            style={{ height: "50vw", maxHeight: "500px", width: "70vw" }}
          />
        </div>

        <div onClick={handleClick}>
          <div className="brick one"></div>
          <div className="tooltip-mario-container">
            <div className="box"></div>
            <div className="mush"></div>
          </div>
          <div className="brick two"></div>
        </div>
      </div>
    </>
  );}
};

export default Debugger;
