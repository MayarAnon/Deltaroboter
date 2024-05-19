/*
This script defines various atoms for Recoil, a state management library for React applications.
Atoms are used to manage and update the state of the application.

The atoms store various settings and values used in the application.

Atoms:

settingAtom: Stores application settings, which are by default loaded from the browser's localStorage or have a default value.
xValueAtom: Stores the X value.
yValueAtom: Stores the Y value.
zValueAtom: Stores the Z value.
phiValueAtom: Stores the Phi value.
actuatorAtom: Stores the mode and value of the actuator.
gCodeStringAtom: Stores the name and content of the G-code.
gCodeModeAtom: Stores the view mode of the G-code.
*/

import { atom } from "recoil";
export const settingAtom = atom({
  key: "settingAtom",
  default: JSON.parse(localStorage.getItem("settings")) || {
    speed: 50,
    manualMode: "buttons",
    gripper: "vacuumGripper",
    color: "#1e293b",
    darkmode: false,
    motionProfil: "TrapezProfil",
    workSpaceRadius: 200,
    workSpaceHeight: 200,
  },
});
export const xValueAtom = atom({
  key: "xValueAtom",
  default: 0,
});

export const yValueAtom = atom({
  key: "yValueAtom",
  default: 0,
});

export const zValueAtom = atom({
  key: "zValueAtom",
  default: -280,
});

export const phiValueAtom = atom({
  key: "phiValueAtom",
  default: 0,
});

export const actuatorAtom = atom({
  key: "actuatorAtom",
  default: {
    mode: "",
    value: 0,
  },
});

export const gCodeStringAtom = atom({
  key: "gCodeStringAtom",
  default: { name: "", content: "" },
});

export const gCodeModeAtom = atom({
  key: "gCodeModeAtom",
  default: 0,
});

export const robotStateAtom = atom({
  key: "robotStateAtom",
  default:{homing:true,
    currentCoordinates:[0,0,-280,0],
    currentAngles:[-31.429121,-31.429121,-31.429121],
    gripperFeedback:false,
    gripperMode:"parallelGripper",
    motionProfil:"TrapezProfil",
    motorsSpeed:50}
});

export const pathPointsAtom = atom({
  key: 'pathPoints', // unique ID (with respect to other atoms/selectors)
  default: [], // default value (aka initial value)
});