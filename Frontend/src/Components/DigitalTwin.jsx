import React, { useEffect, useRef, useState } from 'react';
import * as THREE from 'three';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls';
const baseRadius = 100;
const effectorRadius = 50;
const upperArmLength = 150;
const lowerArmLength = 400;
const basePositions = [
  { x: baseRadius * Math.cos(0), y: baseRadius * Math.sin(0) },
  {
    x: baseRadius * Math.cos((2 * Math.PI) / 3),
    y: baseRadius * Math.sin((2 * Math.PI) / 3),
  },
  {
    x: baseRadius * Math.cos((4 * Math.PI) / 3),
    y: baseRadius * Math.sin((4 * Math.PI) / 3),
  },
];

const baseAngles = [0, 120, 240];
const DigitalTwin = () => {
  const mountRef = useRef(null);
  const [objects, setObjects] = useState({});
  const [robotState, setRobotState] = useState({
    homing: true,
    currentCoordinates: [0, 0, -280], // M1, M2, M3, Drehachse für den Greifer
    currentAngles: [31, 31, 31], // Motorwinkel in Grad
  });
  
  
  // Initialisierung der Basis, Endeffektor und Arme
  useEffect(() => {
    const scene = new THREE.Scene();
    const camera = new THREE.PerspectiveCamera(75, mountRef.current.clientWidth / mountRef.current.clientHeight, 0.1, 2000);
    const renderer = new THREE.WebGLRenderer();
    renderer.setSize(mountRef.current.clientWidth, mountRef.current.clientHeight);
    mountRef.current.appendChild(renderer.domElement);
    camera.position.set(0,0,1000);

    // Licht hinzufügen
    const light = new THREE.AmbientLight(0x404040); // weiches Licht
    scene.add(light);
    const directionalLight = new THREE.DirectionalLight(0xffffff, 1);
      directionalLight.position.set(1, 2, 3);
      scene.add(directionalLight);

     // Basis
     const baseGeometry = new THREE.CylinderGeometry(
      baseRadius,
      baseRadius,
      10,
      32
    );
    const baseMaterial = new THREE.MeshBasicMaterial({ color: 0xaaaaaa });
    const base = new THREE.Mesh(baseGeometry, baseMaterial);
    base.rotation.x = Math.PI / 2;
    scene.add(base);

    // Endeffektor
    const effectorGeometry = new THREE.CylinderGeometry(
      effectorRadius,
      effectorRadius,
      10,
      32
    );
    const effectorMaterial = new THREE.MeshBasicMaterial({ color: 0xff0000 });
    const effector = new THREE.Mesh(effectorGeometry, effectorMaterial);
    effector.position.x = robotState.currentCoordinates[0];
    effector.position.y = robotState.currentCoordinates[1];
    effector.position.z = robotState.currentCoordinates[2];
    effector.rotation.x = Math.PI / 2;
    scene.add(effector);
    //motoren
    basePositions.forEach((position, index) => {
      const motorMaterial = new THREE.MeshBasicMaterial({ color: 0xff0000 });
      const motorGeometry = new THREE.SphereGeometry(15, 32, 32);
      const motor = new THREE.Mesh(motorGeometry, motorMaterial);
      motor.position.set(position.x, position.y, 0);
      scene.add(motor);
    });
   
    // Steuerung
    const controls = new OrbitControls(camera, renderer.domElement);
      controls.enableDamping = true; // Optional, für weicheres "Dämpfung" Effekt
      controls.dampingFactor = 0.05;
      controls.screenSpacePanning = false;
      controls.maxPolarAngle = Math.PI / 2;
    // Speichern von Objekten zur späteren Aktualisierung
    setObjects({ scene, camera, renderer, base, effector, controls });

    // Render-Schleife
    const animate = () => {
      requestAnimationFrame(animate);
      controls.update();
      renderer.render(scene, camera);
    };
    animate();

    // Cleanup
    return () => {
      renderer.dispose();
      controls.dispose();
    };
  }, []);

  // Update der Gelenke und Arme
  useEffect(() => {
    if (!objects.scene) return;
  
    // Berechnet die Positionen der Gelenke und aktualisiert die Arme
    const updatePositions = () => {
      const jointMaterial = new THREE.MeshBasicMaterial({ color: 0xff0aaa });
      const armMaterial = new THREE.MeshBasicMaterial({ color: 0x00ff00 });
      const lowerArmMaterial = new THREE.MeshBasicMaterial({ color: 0x0000ff });
  
      const jointPositions = calculateJointPositions(robotState.currentAngles);
      const endEffectorPositions = [
        { x: effectorRadius * Math.cos(0), y: effectorRadius * Math.sin(0), z: robotState.currentCoordinates[2] },
        {
          x: effectorRadius * Math.cos((2 * Math.PI) / 3),
          y: effectorRadius * Math.sin((2 * Math.PI) / 3),
          z: robotState.currentCoordinates[2]
        },
        {
          x: effectorRadius * Math.cos((4 * Math.PI) / 3),
          y: effectorRadius * Math.sin((4 * Math.PI) / 3),
          z: robotState.currentCoordinates[2]
        },
      ];
  
      objects.scene.children.forEach(child => {
        if (child.type === 'Mesh' && (child.material.color.getHex() === jointMaterial.color.getHex() || child.material.color.getHex() === armMaterial.color.getHex() || child.material.color.getHex() === lowerArmMaterial.color.getHex())) {
          objects.scene.remove(child);
        }
      });
  
      jointPositions.forEach((position, index) => {
        // Gelenke neu erstellen
        const jointGeometry = new THREE.SphereGeometry(10, 32, 32);
        const joint = new THREE.Mesh(jointGeometry, jointMaterial);
        joint.position.set(position.x, position.y, position.z);
        objects.scene.add(joint);
  
        // Oberarme neu erstellen
        const motorPosition = new THREE.Vector3(basePositions[index].x, basePositions[index].y, 0);
        const jointPosition = new THREE.Vector3(position.x, position.y, position.z);
        const armGeometry = new THREE.CylinderGeometry(5, 5, upperArmLength, 32);
        const arm = new THREE.Mesh(armGeometry, armMaterial);
        arm.position.copy(motorPosition).lerp(jointPosition, 0.5);
        arm.lookAt(jointPosition);
        arm.rotateX(Math.PI / 2);
        objects.scene.add(arm);
  
        // Unterarme neu erstellen
        const endEffectorPosition = new THREE.Vector3(
          endEffectorPositions[index].x,
          endEffectorPositions[index].y,
          robotState.currentCoordinates[2]
        );
        const lowerArmGeometry = new THREE.CylinderGeometry(5, 5, lowerArmLength, 32);
        const lowerArm = new THREE.Mesh(lowerArmGeometry, lowerArmMaterial);
        lowerArm.position.copy(jointPosition).lerp(endEffectorPosition, 0.5);
        lowerArm.lookAt(endEffectorPosition);
        lowerArm.rotateX(Math.PI / 2);
        objects.scene.add(lowerArm);
      });
    };
  
    updatePositions();
    objects.renderer.render(objects.scene, objects.camera);
  
  }, [robotState, objects]);
  
  useEffect(() => {
    function connect() {
      const websocket = new WebSocket('ws://deltarobot.local:3010');
      websocket.onopen = () => console.log('WebSocket connected');
      websocket.onmessage = event => {
        const data = JSON.parse(event.data);
        setRobotState(prevState => ({
          ...prevState,
          ...data
        }));
      };
      websocket.onerror = error => console.error('WebSocket error:', error);
      websocket.onclose = event => {
        console.log('WebSocket disconnected', event.reason);
        setTimeout(connect, 5000);
      };
    }
    connect();
  }, []);
  


  useEffect(() => {
    const updateScene = () => {
      if (!objects.scene) return;
      objects.effector.position.x = robotState.currentCoordinates[0];
      objects.effector.position.x = robotState.currentCoordinates[1];
      objects.effector.position.z = robotState.currentCoordinates[2]; // Update Endeffector position
      objects.renderer.render(objects.scene, objects.camera);
    };

    updateScene();
  }, [robotState.currentCoordinates, objects]);
  return (
    <div>
      <h1>Digital Twin of Delta Robot</h1>
      <div ref={mountRef} style={{ width: "50%", height: "50vh" }}></div>
    </div>
  );
};

// Berechnungsfunktion für Gelenkpositionen
function calculateJointPositions(motorAngles) {
  const pi = Math.PI;
  const sin120 = Math.sqrt(3) / 2.0;
  const cos120 = -0.5;
  const sin240 = -sin120;
  const cos240 = cos120;
  // Funktion zur Berechnung der Gelenkpositionen für einen gegebenen Winkel
  function calculatePosition(theta, cosAngle, sinAngle,motorPos) {
      const t = (pi / 180) * theta;  // Umwandlung von Grad in Radian
      const x = motorPos.x+upperArmLength * Math.cos(t) * cosAngle;
      const y =motorPos.y+upperArmLength * Math.cos(t) * sinAngle;
      const z = upperArmLength * Math.sin(t);
      return {x, y, z};
  }

  // Berechnung der Gelenkpositionen für jeden der drei Arme
  const joint1 = calculatePosition(motorAngles[0],1,0,basePositions[0]);
  const joint2 = calculatePosition(motorAngles[1],cos120, sin120,basePositions[1]);
  const joint3 = calculatePosition(motorAngles[2],cos240, sin240,basePositions[2]);

  return [ joint1, joint2, joint3 ];
}

export default DigitalTwin;
