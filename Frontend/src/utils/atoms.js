import { atom } from 'recoil';
export const settingAtom = atom({
  key: 'settingAtom', 
  default: JSON.parse(localStorage.getItem('settings')) || {
    speed: 50,
    manualMode: "buttons",
    gripper: "vacuumGripper",
    color: "#1e293b",
    workSpaceRadius: 200,
    workSpaceHeight: 200
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
  default: -280,  // Adjust the default value as needed
});

export const phiValueAtom = atom({
  key: 'phiValueAtom',
  default: 0,  // Adjust the default value as needed
});

export const actuatorAtom = atom({
  key: 'actuatorAtom',
  default: {
    mode: '',
    value: 0
  }
});

export const gCodeStringAtom = atom({
  key: 'gCodeStringAtom',
  default: { name: '', content: '' },  // Adjust the default value as needed, assuming it's a string
});

export const gCodeModeAtom = atom({
  key: 'gCodeModeAtom',
  default:0,  // Adjust the default value as needed, assuming it's a string
});
