import React from "react";

import { Unity, useUnityContext } from "react-unity-webgl";
const Debugger = () => {
  const { unityProvider } = useUnityContext({
    loaderUrl: "Build/FlappyBird.loader.js",
    dataUrl: "Build/webgl.data",
    frameworkUrl: "Build/build.framework.js",
    codeUrl: "Build/build.wasm",
  });
  
  return (
    <div className="flex flex-col mt-5 items-center justify-center">
      <Unity
        unityProvider={unityProvider}
        style={{ height: "50vw", maxHeight: "500px", width:"70vw" }}
      />
    </div>
  );
};

export default Debugger;
