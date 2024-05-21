import React, { useState, useEffect } from "react";
import { Unity, useUnityContext } from "react-unity-webgl";
import "../styles/debugger.css";

const Debugger = () => {
  const { unityProvider } = useUnityContext({
    loaderUrl: "Build/FlappyBird.loader.js",
    dataUrl: "Build/webgl.data",
    frameworkUrl: "Build/build.framework.js",
    codeUrl: "Build/build.wasm",
  });

  const [cursors, setCursors] = useState([]);
  const [showUnity, setShowUnity] = useState(false);
  const [showImage, setShowImage] = useState(false);

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

  const handleUnityClick = () => {
    setShowUnity(true);
    setShowImage(false);
  };

  const handleImageClick = (event) => {
    setShowImage(true);
    setShowUnity(false);
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
  } else {
    return (
      <>
        {showUnity && (
          <div className="flex flex-col mt-5 items-center justify-center">
            <Unity
              unityProvider={unityProvider}
              style={{ height: "50vw", maxHeight: "500px", width: "70vw" }}
            />
          </div>
        )}

        {showImage && (
          <img
            src="/debugger.png" // Pfad zum Bild
            className="debugger-image"
            alt="Debugger"
            style={{
              position: "absolute",
              left: "50%",
              top: "70%",
              width:"800px",
              transform: "translate(-50%, -50%)",
              zIndex: 10,
            }}
          />
        )}

        {cursors.map(
          (cursor, index) =>
            cursor.visible && (
              <img
                key={index}
                src="/image-removebg-preview.png" // Pfad zum Bild
                className="cursor-image"
                style={{
                  left: `${cursor.x}px`,
                  top: `${cursor.y}px`,
                  display: "block",
                  width: "17px",
                  height: "17px",
                  transform: "rotate(15deg)",
                }}
                alt="Custom Cursor"
              />
            )
        )}

        <div className="flex flex-col items-center">
          <div className="flex flex-row mt-5 items-center justify-center">
            <div onClick={handleUnityClick} style={{ marginRight: "100px" }}>
              <div className="brick one"></div>
              <div className="tooltip-mario-container">
                <div className="box"></div>
                <div className="mush"></div>
              </div>
              <div className="brick two"></div>
            </div>

            <div onClick={handleImageClick} style={{ marginLeft: "100px" }}>
              <div className="brick one"></div>
              <div className="tooltip-mario-container">
                <div className="box"></div>
                <div className="mush"></div>
              </div>
              <div className="brick two"></div>
            </div>
          </div>
        </div>
      </>
    );
  }
};
export default Debugger;
