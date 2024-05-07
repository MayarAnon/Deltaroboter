import React, { useEffect, useRef, useState } from "react";
import * as THREE from "three";
import { OrbitControls } from "three/examples/jsm/controls/OrbitControls";
import { GLTFLoader } from "three/examples/jsm/loaders/GLTFLoader";

// Konstanten für die Basisgeometrie und Positionsdaten.
const baseRadius = 100;
const effectorRadius = 50;
const upperArmLength = 150;
const lowerArmLength = 400;
//die position der Motoren auf einer scheibe (0°,120°,240°)
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
// Hilfsfunktion zum Erstellen von Achsenhelfern im 3D-Raum.
function createAxesHelper(length, thickness) {
  const axes = new THREE.Object3D();

  // Material für die Achsen
  const materials = {
    x: new THREE.MeshBasicMaterial({ color: 0xff0000 }), // Rot für die X-Achse
    y: new THREE.MeshBasicMaterial({ color: 0x00ff00 }), // Grün für die Y-Achse
    z: new THREE.MeshBasicMaterial({ color: 0x0000ff }), // Blau für die Z-Achse
  };

  // X-Achse (Rot)
  const xAxisGeometry = new THREE.CylinderGeometry(
    thickness,
    thickness,
    length,
    32
  );
  const xAxis = new THREE.Mesh(xAxisGeometry, materials.x);
  xAxis.rotation.z = -Math.PI / 2;
  xAxis.position.x = length / 2;

  // Y-Achse (Grün)
  const yAxisGeometry = new THREE.CylinderGeometry(
    thickness,
    thickness,
    length,
    32
  );
  const yAxis = new THREE.Mesh(yAxisGeometry, materials.y);
  yAxis.position.y = length / 2;

  // Z-Achse (Blau)
  const zAxisGeometry = new THREE.CylinderGeometry(
    thickness,
    thickness,
    length,
    32
  );
  const zAxis = new THREE.Mesh(zAxisGeometry, materials.z);
  zAxis.rotation.x = Math.PI / 2;
  zAxis.position.z = length / 2;

  axes.add(xAxis, yAxis, zAxis);
  return axes;
}


// Berechnungsfunktion für Gelenkpositionen
function calculateJointPositions(motorAngles) {
  const pi = Math.PI;
  const sin120 = Math.sqrt(3) / 2.0;
  const cos120 = -0.5;
  const sin240 = -sin120;
  const cos240 = cos120;
  // Funktion zur Berechnung der Gelenkpositionen für einen gegebenen Winkel
  function calculatePosition(theta, cosAngle, sinAngle, motorPos) {
    const t = (pi / 180) * theta; // Umwandlung von Grad in Radian
    const x = motorPos.x + upperArmLength * Math.cos(t) * cosAngle;
    const y = motorPos.y + upperArmLength * Math.cos(t) * sinAngle;
    const z = upperArmLength * Math.sin(t);
    return { x, y, z };
  }

  // Berechnung der Gelenkpositionen für jeden der drei Arme
  const joint1 = calculatePosition(-motorAngles[0], 1, 0, basePositions[0]);
  const joint2 = calculatePosition(
    -motorAngles[1],
    cos120,
    sin120,
    basePositions[1]
  );
  const joint3 = calculatePosition(
    -motorAngles[2],
    cos240,
    sin240,
    basePositions[2]
  );

  return [joint1, joint2, joint3];
}
// Hauptkomponente für das Digital Twin Modell.
const DigitalTwin = () => {
  const mountRef = useRef(null);// Referenz für das DOM-Element.
  const [objects, setObjects] = useState({}); // Zustand für verwaltete 3D-Objekte.
  const [robotState, setRobotState] = useState({ 
    currentCoordinates: [0, 0, -280],
    currentAngles: [-32, -32, -32],
  });// Initialer Zustand des Roboters, wird vom ws geupdatet.
  const loader = new GLTFLoader();// Loader für 3D-Modelle.

   // Haupt-Effect-Hook zur Initialisierung und Aktualisierung der 3D-Szene.
  useEffect(() => {
    // scene, kamera, renderer erstellen
    const scene = new THREE.Scene();
    const camera = new THREE.PerspectiveCamera(
      75,
      mountRef.current.clientWidth / mountRef.current.clientHeight,
      0.1,
      2000
    );

    const renderer = new THREE.WebGLRenderer();
    renderer.setSize(
      mountRef.current.clientWidth,
      mountRef.current.clientHeight
    );
    renderer.setClearColor(0xffffff);
    mountRef.current.appendChild(renderer.domElement);

    camera.position.set(0, 700, 0);

    // Licht hinzufügen
    const light = new THREE.AmbientLight(0x404040); // weiches Licht
    scene.add(light);
    const directionalLight = new THREE.DirectionalLight(0xffffff, 1);
    directionalLight.position.set(1, 2, 3);
    scene.add(directionalLight);

    // Koordinatengitter hinzufügen
    const gridHelperXY = new THREE.GridHelper(5000, 60);
    scene.add(gridHelperXY);
    gridHelperXY.rotation.x = Math.PI / 2;
    gridHelperXY.position.set(0, 0, -500);
    //achsen für den endeffektor
    const axesHelper = createAxesHelper(150, 3); // 150 ist die Länge, 2 ist die Dicke
    scene.add(axesHelper);
    axesHelper.position.x = robotState.currentCoordinates[0];
    axesHelper.position.y = robotState.currentCoordinates[1];
    axesHelper.position.z = robotState.currentCoordinates[2];
    axesHelper.rotation.z = Math.PI / 50;
    
  
    // // einfache Basis
    // const baseGeometry = new THREE.CylinderGeometry(
    //   baseRadius,
    //   baseRadius,
    //   10,
    //   32
    // );
    // const baseMaterial = new THREE.MeshBasicMaterial({
    //   color: 0x000000,
    //   opacity: 0.5,
    //   transparent: true,
    // });
    // const base = new THREE.Mesh(baseGeometry, baseMaterial);
    // base.rotation.x = Math.PI / 2;
    // scene.add(base);

    // // Einfacher Endeffektor
    // const effectorGeometry = new THREE.CylinderGeometry(
    //   effectorRadius,
    //   effectorRadius,
    //   10,
    //   32
    // );
    // const effectorMaterial = new THREE.MeshBasicMaterial({
    //   color: 0x000000,
    //   opacity: 0.5,
    //   transparent: true,
    // });
    // const effector = new THREE.Mesh(effectorGeometry, effectorMaterial);
    // effector.position.x = robotState.currentCoordinates[0];
    // effector.position.y = robotState.currentCoordinates[1];
    // effector.position.z = robotState.currentCoordinates[2];
    // effector.rotation.x = Math.PI / 2;
    // scene.add(effector);
    // model base.glb laden und initailisieren
    let base;
    loader.load(
      "/model/base.glb",
      function (gltf) {
        base = gltf.scene;
        base.scale.set(10, 10, 10);
        base.position.set(0, 0, 90);
        base.rotation.z = Math.PI / 6;
        scene.add(base);
        setObjects((prev) => ({ ...prev, base }));
      },
      undefined,
      function (error) {
        console.error("Error loading the base model:", error);
      }
    );
    let effector;
    // model endeffector.glb laden und initailisieren
    loader.load(
      "/model/endeffector.glb",
      function (gltf) {
        effector = gltf.scene;
        effector.scale.set(10, 10, 10);
        effector.position.set(
          robotState.currentCoordinates[0],
          robotState.currentCoordinates[1],
          robotState.currentCoordinates[2]
        );
        effector.rotation.z = Math.PI / 6;
        scene.add(effector);
        setObjects((prev) => ({ ...prev, effector })); // Speichern des Endeffektors in den Zustand
      },
      undefined,
      function (error) {
        console.error("Error loading the end effector model:", error);
      }
    );
    // visualisierung für die motorposition
    // basePositions.forEach((position, index) => {
    //   const motorMaterial = new THREE.MeshBasicMaterial({ color: 0xff0000 });
    //   const motorGeometry = new THREE.SphereGeometry(15, 32, 32);
    //   const motors = new THREE.Mesh(motorGeometry, motorMaterial);
    //   motors.position.set(position.x, position.y, 0);
    //   scene.add(motors);
    // });

    // model motor1.glb laden und initailisieren
    loader.load(
      "/model/motor1.glb",
      function (gltf) {
        scene.add(gltf.scene);
        gltf.scene.scale.set(10, 10, 10);
        gltf.scene.position.set(0, 0, 90);
        gltf.scene.rotation.z = Math.PI / 6;
      },
      undefined,
      function (error) {
        console.error(error);
      }
    );
    // model motor2.glb laden und initailisieren
    loader.load(
      "/model/motor2.glb",
      function (gltf) {
        scene.add(gltf.scene);
        gltf.scene.scale.set(10, 10, 10);
        gltf.scene.position.set(0, 0, 90);
        gltf.scene.rotation.z = Math.PI / 6;
      },
      undefined,
      function (error) {
        console.error(error);
      }
    );
    // model motor3.glb laden und initailisieren
    loader.load(
      "/model/motor3.glb",
      function (gltf) {
        scene.add(gltf.scene);
        gltf.scene.scale.set(10, 10, 10);
        gltf.scene.position.set(0, 0, 90);
        gltf.scene.rotation.z = Math.PI / 6;
      },
      undefined,
      function (error) {
        console.error(error);
      }
    );
    // Arbeitsraum Zylinder hinzufügen
    const workspaceGeometry = new THREE.CylinderGeometry(200, 200, 200, 32); // Radius, Radius, Höhe, Segmentzahl
    const workspaceMaterial = new THREE.MeshBasicMaterial({
      color: 0x888888,
      transparent: true,
      opacity: 0.3,
    });
    const workspaceCylinder = new THREE.Mesh(
      workspaceGeometry,
      workspaceMaterial
    );
    workspaceCylinder.position.z = -380; // Mitte zwischen -280 und -480
    workspaceCylinder.rotation.x = Math.PI / 2; // Drehung um die X-Achse, um den Zylinder senkrecht zu stellen
    scene.add(workspaceCylinder);
    // Steuerung
    const controls = new OrbitControls(camera, renderer.domElement);
    controls.enableDamping = true; // Optional, für weicheres "Dämpfung" Effekt
    controls.dampingFactor = 0.05;
    controls.screenSpacePanning = false;
    controls.maxPolarAngle = Math.PI / 2;

    controls.target.set(0, 0, 0);

    controls.update();
    //scene drehen
    scene.rotation.x = Math.PI;
    scene.rotation.z = Math.PI / 2;
    // Speichern von Objekten zur späteren Aktualisierung
    setObjects({
      scene,
      camera,
      renderer,
      base,
      effector,
      controls,
      axesHelper,
    });

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
    // Lösche alle vorherigen Arme und Gelenke
    const toRemove = [];
    objects.scene.traverse((child) => {
      if (child.userData.type === "joint" || child.userData.type === "arm") {
        toRemove.push(child);
      }
    });
    toRemove.forEach((child) => {
      objects.scene.remove(child);
    });

    // Update und Neuerstellung der Gelenke und Arme
    const jointMaterial = new THREE.MeshBasicMaterial({ color: 0xff0aaa });
    const armMaterial = new THREE.MeshBasicMaterial({
      color: 0x000000,
      opacity: 0.4,
      transparent: true,
    });
    const lowerArmMaterial = new THREE.MeshBasicMaterial({
      color: 0x000000,
      opacity: 0.4,
      transparent: true,
    });
    const jointPositions = calculateJointPositions(robotState.currentAngles);
    jointPositions.forEach((position, index) => {
      // Erstellen neuer Gelenke
      const jointGeometry = new THREE.SphereGeometry(10, 32, 32);
      const joint = new THREE.Mesh(jointGeometry, jointMaterial);
      joint.position.set(position.x, position.y, position.z);
      joint.userData.type = "joint"; 
      objects.scene.add(joint);

      // Oberarme
      const motorPosition = new THREE.Vector3(
        basePositions[index].x,
        basePositions[index].y,
        0
      );
      const jointPosition = new THREE.Vector3(
        position.x,
        position.y,
        position.z
      );
      const armGeometry = new THREE.CylinderGeometry(5, 5, upperArmLength, 32);
      const arm = new THREE.Mesh(armGeometry, armMaterial);
      arm.position.copy(motorPosition).lerp(jointPosition, 0.5);
      arm.lookAt(jointPosition);
      arm.rotateX(Math.PI / 2);
      arm.userData.type = "arm"; 
      objects.scene.add(arm);
      // let arm;
      // loader.load(`/model/upperarm.glb`, (gltf) => {
      //   arm = gltf.scene;
      //   arm.scale.set(10, 10, 10);

      //   arm.position.copy(motorPosition).lerp(jointPosition, 0.5);
      //   arm.lookAt(jointPosition);

      //   arm.rotateX(Math.PI / 2);
      //   if (index === 0) arm.rotateY(Math.PI);
      //   if (index === 1) arm.rotateY((Math.PI * 7) / 4);
      //   if (index === 2) arm.rotateY((Math.PI * 7) / 4);
      //   arm.userData.type = "arm"; 
      //   objects.scene.add(arm);
      // });

      // Unterarme
      const endEffectorPosition = new THREE.Vector3(
        robotState.currentCoordinates[0] +
          effectorRadius * Math.cos((index * 2 * Math.PI) / 3),
        robotState.currentCoordinates[1] +
          effectorRadius * Math.sin((index * 2 * Math.PI) / 3),
        robotState.currentCoordinates[2]
      );
      // //lowerArm model, bug: die orientierung und ausrichtung ist aufwändig
      // let lowerArm;
      // loader.load(`/model/lowerarm.glb`, (gltf) => {
      //   lowerArm = gltf.scene;
      //   lowerArm.scale.set(10, 10, 10);

      //   lowerArm.position.copy(jointPosition).lerp(endEffectorPosition, 0.5);

      //   lowerArm.lookAt(endEffectorPosition);
      //   lowerArm.rotateX(Math.PI / 2);
      //   if(index===1)lowerArm.rotateY(Math.PI*4/3);
      //   if(index===2)lowerArm.rotateY(Math.PI*2/3);
      //   lowerArm.userData.type = "arm"; 
      //   objects.scene.add(lowerArm);
      // });

      // //einfacher lowerArm, funktioniert, armlänge ist fix
      // const lowerArm = new THREE.Mesh(lowerArmGeometry, lowerArmMaterial);
      // lowerArm.position.copy(jointPosition).lerp(endEffectorPosition, 0.5);
      // lowerArm.lookAt(endEffectorPosition);
      // lowerArm.rotateX(Math.PI / 2);
      // lowerArm.userData.type = "arm"; 
      // objects.scene.add(lowerArm);

      //einfacher lowerarm aber mit einer dynamischen länge, um bugs zu vertuschen
      const midPoint = new THREE.Vector3()
        .addVectors(jointPosition, endEffectorPosition)
        .multiplyScalar(0.5);
      const height = jointPosition.distanceTo(endEffectorPosition);
      const lowerArmGeometry = new THREE.CylinderGeometry(5, 5, height, 32);
      const lowerArm = new THREE.Mesh(lowerArmGeometry, lowerArmMaterial);
      lowerArm.position.copy(jointPosition).lerp(endEffectorPosition, 0.5);
      lowerArm.lookAt(endEffectorPosition);
      lowerArm.position.copy(midPoint);
      lowerArm.rotateX(Math.PI / 2);
      lowerArm.userData.type = "arm"; 
      objects.scene.add(lowerArm);
    });

    setObjects(objects);
  }, [robotState, objects]);

  //robotstate über ws aktualisieren
  useEffect(() => {
    function connect() {
      const websocket = new WebSocket("ws://deltarobot:3010");
      websocket.onopen = () => console.log("WebSocket connected");
      websocket.onmessage = (event) => {
        const data = JSON.parse(event.data);
        setRobotState((prevState) => ({
          ...prevState,
          currentCoordinates: [
            data.currentCoordinates[0],
            data.currentCoordinates[1],
            data.currentCoordinates[2],
          ],
          currentAngles: [
            data.currentAngles[0],
            data.currentAngles[1],
            data.currentAngles[2],
          ],
        }));
      };

      websocket.onerror = (error) => console.error("WebSocket error:", error);
      websocket.onclose = (event) => {
        console.log("WebSocket disconnected", event.reason);
        setTimeout(connect, 5000);
      };
    }
    connect();
  }, []);

  //update Effector position and calc motor angles
  useEffect(() => {
    const updateScene = () => {
      if (!objects.scene) return;
      objects.axesHelper.position.x = robotState.currentCoordinates[0];
      objects.axesHelper.position.y = robotState.currentCoordinates[1];
      objects.axesHelper.position.z = robotState.currentCoordinates[2];
      objects.effector.position.x = robotState.currentCoordinates[0];
      objects.effector.position.y = robotState.currentCoordinates[1];
      objects.effector.position.z = robotState.currentCoordinates[2];
      objects.renderer.render(objects.scene, objects.camera);
    };
    if (!objects.scene || !objects.effector || !objects.base) return;
    updateScene();
  }, [robotState.currentCoordinates, objects]);

  //szene
  return (
    <div
      style={{
        display: "flex",
        justifyContent: "center",
        alignItems: "center",
        height: "100vh",
      }}
    >
      <div
        id="statusOverlay"
        style={{
          position: "absolute",
          top: "27vh",
          color: "white",
          backgroundColor: "rgba(0, 0, 0, 0.5)",
          padding: "5px",
          zIndex: 1000, // Stellen Sie sicher, dass das Overlay über dem Canvas angezeigt wird
        }}
      >
        Coordinates: {robotState.currentCoordinates.join(", ")}
        <br />
        Angles: {robotState.currentAngles.join(", ")}
      </div>
      <div
        ref={mountRef}
        style={{ width: "90%", height: "90vh", border: "2px solid black" }}
      ></div>
    </div>
  );
};



export default DigitalTwin;
