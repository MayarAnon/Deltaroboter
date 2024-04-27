import { atom } from 'recoil';
export const settingAtom = atom({
  key: 'settingAtom', 
  default: {             
    speed: 50,
    manualMode: "buttons",
    gripper: "1option",
    color: "#1e293b",
    workSpaceRadius:200,
    workSpaceHeight:200
  }
});
export const xValueAtom = atom({
  key: 'xValueAtom',
  default: 0,  // Adjust the default value as needed
});

export const yValueAtom = atom({
  key: 'yValueAtom',
  default: 0,  // Adjust the default value as needed
});

export const zValueAtom = atom({
  key: 'zValueAtom',
  default: 0,  // Adjust the default value as needed
});

export const phiValueAtom = atom({
  key: 'phiValueAtom',
  default: 0,  // Adjust the default value as needed
});

export const actuatorAtom = atom({
  key: 'actuatorAtom',
  default: '',  // Adjust the default value as needed, assuming it's a string
});

export const gCodeStringAtom = atom({
  key: 'gCodeStringAtom',
  default: "Hier Ihren G-Code eingeben...",  // Adjust the default value as needed, assuming it's a string
});