import React from "react";

import { Unity, useUnityContext } from "react-unity-webgl";
import { useRecoilState } from "recoil";
import { settingAtom } from "../utils/atoms";
const Game = () => {
  const [settings, setSettings] = useRecoilState(settingAtom);
  const { unityProvider } = useUnityContext({
    loaderUrl: "Build/FlappyBird.loader.js",
    dataUrl: "Build/webgl.data",
    frameworkUrl: "Build/build.framework.js",
    codeUrl: "Build/build.wasm",
  });
  const refreshPage = () => {
    window.location.reload();
  };
  return (
    <div className="flex flex-col mt-5 items-center justify-center">
      <Unity
        unityProvider={unityProvider}
        style={{ height: "50vw", maxHeight: "500px", width:"70vw" }}
      />
    </div>
  );
};

export default Game;
