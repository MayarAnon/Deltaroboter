import React, { useEffect, useRef, useState } from "react";
import * as THREE from "three";
import { OrbitControls } from "three/examples/jsm/controls/OrbitControls";

const DigitalTwin = () => {
  const mountRef = useRef(null);
  const [robotState, setRobotState] = useState({
    homing: true,
    currentCoordinates: [0, 0, -280], // M1, M2, M3, Drehachse für den Greifer
    currentAngles: [-41, -41, -41], // Motorwinkel in Grad
  });
  const baseAngles = [0, 120, 240];
  const [motorAngles, setMotorAngles] = useState({
    theta1: -100,
    theta2: -41,
    theta3: -41,
  });
  const [endEffectorPosition, setEndEffectorPosition] = useState({
    x: 0,
    y: 0,
    z: -280,
  });

  const [jointPositions, setJointPositions] = useState([
    { x: 0, y: 0, z: 0},
    { x: 0, y: 0, z: 0 },
    { x: 0, y: 0, z: 0}
]);
// const [jointPositions, setJointPositions] = useState([
//     { x: 188.677395055693, y: -164.01475724762682, z: -181.59114565142391 },
//     { x: 47.70224884413623, y: 245.406795861916, z: -427.2440775171496 },
//     { x: -236.37964389982918, y: -81.39203861428919, z: -231.16477683142648}
// ]);
  // Parameter
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
    const joint1 = calculatePosition(motorAngles.theta1,1,0,basePositions[0]);
    const joint2 = calculatePosition(motorAngles.theta2,cos120, sin120,basePositions[1]);
    const joint3 = calculatePosition(motorAngles.theta3,cos240, sin240,basePositions[2]);

    return [ joint1, joint2, joint3 ];
}

useEffect(() => {
    const joints = calculateJointPositions(motorAngles);
    setJointPositions(joints);
}, [motorAngles]);
  useEffect(() => {
    let renderer, scene, camera, frameId, controls;

    const initThree = () => {
      // Szene initialisieren
      scene = new THREE.Scene();
      camera = new THREE.PerspectiveCamera(
        75,
        mountRef.current.clientWidth / mountRef.current.clientHeight,
        0.1,
        2000
      );
      renderer = new THREE.WebGLRenderer();
      renderer.setSize(
        mountRef.current.clientWidth,
        mountRef.current.clientHeight
      );
      mountRef.current.appendChild(renderer.domElement);
      camera.position.set(0, 0, 1000);

      // Beleuchtung hinzufügen
      const ambientLight = new THREE.AmbientLight(0x404040, 1);
      scene.add(ambientLight);
      const directionalLight = new THREE.DirectionalLight(0xffffff, 1);
      directionalLight.position.set(1, 2, 3);
      scene.add(directionalLight);
      // OrbitControls hinzufügen
      controls = new OrbitControls(camera, renderer.domElement);
      controls.enableDamping = true; // Optional, für weicheres "Dämpfung" Effekt
      controls.dampingFactor = 0.05;
      controls.screenSpacePanning = false;
      controls.maxPolarAngle = Math.PI / 2;

      createRobot();
    };

    const createRobot = () => {
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
      effector.position.z = robotState.currentCoordinates[2]; // Homing Z position
      effector.rotation.x = Math.PI / 2;
    //   scene.add(effector);

      
      // Arme
      const armMaterial = new THREE.MeshBasicMaterial({ color: 0x00ff00 });

      const motorMaterial = new THREE.MeshBasicMaterial({ color: 0xff0000 });
      const jointMaterial = new THREE.MeshBasicMaterial({ color: 0xff0aaa });
      robotState.currentAngles.forEach((angle, index) => {
        const motorGeometry = new THREE.SphereGeometry(5, 32, 32);
        const jointGeometry = new THREE.SphereGeometry(10, 32, 32);
        const motor = new THREE.Mesh(motorGeometry, motorMaterial);
        const joint = new THREE.Mesh(jointGeometry, jointMaterial);
        const upperArmGeometry = new THREE.CylinderGeometry(
          5,
          5,
          upperArmLength,
          32
        );
        const upperArm = new THREE.Mesh(upperArmGeometry, armMaterial);

        const baseAngle = (baseAngles[index] * Math.PI) / 180;
        const motorAngle = (angle * Math.PI) / 180;
        const motorPositionX = basePositions[index].x;
        const motorPositionY = basePositions[index].y;
        // const armEndX = motorPositionX + upperArmLength * Math.cos(motorAngle);
        // const armEndZ = upperArmLength * Math.sin(motorAngle);

        motor.position.set(motorPositionX, motorPositionY, 0);
        joint.position.set(jointPositions[index].x, jointPositions[index].y, jointPositions[index].z);
        upperArm.position.set(motorPositionX, motorPositionY, 0);
        upperArm.rotation.z = baseAngle;

        scene.add(new THREE.AxesHelper());
        // scene.add(upperArm);
        scene.add(motor);
        scene.add(joint);
      });
    };

    const animate = () => {
      controls.update(); // Nur notwendig, wenn controls.enableDamping oder controls.autoRotate aktiviert sind
      frameId = requestAnimationFrame(animate);
      renderer.render(scene, camera);
    };

    const resize = () => {
      if (renderer) {
        camera.aspect =
          mountRef.current.clientWidth / mountRef.current.clientHeight;
        camera.updateProjectionMatrix();
        renderer.setSize(
          mountRef.current.clientWidth,
          mountRef.current.clientHeight
        );
      }
    };

    window.addEventListener("resize", resize);
    initThree();
    frameId = requestAnimationFrame(animate);

    return () => {
      window.removeEventListener("resize", resize);
      cancelAnimationFrame(frameId);
      renderer.dispose();
      controls.dispose(); // Reinige die Controls beim Demontieren
    };
  }, [jointPositions]);

  return (
    <div>
      <h1>Digital Twin</h1>
      <div ref={mountRef} style={{ width: "100%", height: "100vh" }} />
    </div>
  );
};

export default DigitalTwin;
