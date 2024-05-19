import React, { useEffect, useRef, useState } from "react";
import * as THREE from "three";
import { OrbitControls } from "three/examples/jsm/controls/OrbitControls";
import { GLTFLoader } from "three/examples/jsm/loaders/GLTFLoader";
import dat from "dat.gui";
import delta_calcInverse from "../utils/IK";
import { useRecoilValue } from "recoil";
import { settingAtom } from "../utils/atoms";
import RobotStateDisplay from "./Robotstate";
// Constants define the geometric properties of the base and the effector as well as the length of the arms
const baseRadius = 100;
const effectorRadius = 50;
const upperArmLength = 150;
const lowerArmLength = 400;
// Calculates the positions of the joints based on the current motor angles and the system's kinematics
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
// Creates visual guidelines (axes) for the X, Y, and Z coordinates for better orientation in 3D space.
function createAxesHelper(length, thickness) {
  const axes = new THREE.Object3D();

  // Materials for the axes
  const materials = {
    x: new THREE.MeshBasicMaterial({ color: 0xff0000 }), // Rot für die X-Achse
    y: new THREE.MeshBasicMaterial({ color: 0x00ff00 }), // Grün für die Y-Achse
    z: new THREE.MeshBasicMaterial({ color: 0x0000ff }), // Blau für die Z-Achse
  };

  // X-Axe (Red)
  const xAxisGeometry = new THREE.CylinderGeometry(
    thickness,
    thickness,
    length,
    32
  );
  const xAxis = new THREE.Mesh(xAxisGeometry, materials.x);
  xAxis.rotation.z = -Math.PI / 2;
  xAxis.position.x = length / 2;

  // Y-Axe (Green)
  const yAxisGeometry = new THREE.CylinderGeometry(
    thickness,
    thickness,
    length,
    32
  );
  const yAxis = new THREE.Mesh(yAxisGeometry, materials.y);
  yAxis.position.y = length / 2;

  // Z-Axe (Blue)
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

// Calculates the positions of the joints based on the current motor angles and the system's kinematics
function calculateJointPositions(motorAngles) {
  const pi = Math.PI;
  const sin120 = Math.sqrt(3) / 2.0;
  const cos120 = -0.5;
  const sin240 = -sin120;
  const cos240 = cos120;
  // Function to calculate joint positions for a given angle
  function calculatePosition(theta, cosAngle, sinAngle, motorPos) {
    const t = (pi / 180) * theta; // Conversion from degrees to radians
    const x = motorPos.x + upperArmLength * Math.cos(t) * cosAngle;
    const y = motorPos.y + upperArmLength * Math.cos(t) * sinAngle;
    const z = upperArmLength * Math.sin(t);
    return { x, y, z };
  }

  // Calculation of joint positions for each of the three arms
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

// Main component for the Digital Twin model.
const DigitalTwin = () => {
  const settings = useRecoilValue(settingAtom);
  const mountRef = useRef(null); // Reference for the DOM element.
  const websocketRef = useRef(null);

  const savedCameraPosition = localStorage.getItem("cameraPosition");
  const savedSceneObjects = localStorage.getItem("sceneObjects");
  // Set initial scene configuration from localStorage or use default values
  const initialSceneObjects = savedSceneObjects
    ? JSON.parse(savedSceneObjects)
    : {
        gridSize: 5000,
        gridDivisions: 60,
        gridVisible: true,
        axesVisible: false,
        endEffectorVisible: false,
        baseVisible: false,
        motorsVisible: false,
      };
  const initialCameraPosition = savedCameraPosition
    ? parseInt(savedCameraPosition)
    : 1;
  const [cameraPosition, setCameraPosition] = useState(initialCameraPosition); // 1 for the first POV, 2 for the second POV
  const cameraPositionRef = useRef(cameraPosition);
  const cameraRef = useRef();
  // State for managed 3D objects in the scene
  const [objects, setObjects] = useState({});
  //parameters for scene configuration with the Controls-gui
  const [sceneObjects, setSceneObjects] = useState(initialSceneObjects);

  // Initial state of the robot, updated by the WS (websocket).
  const [robotState, setRobotState] = useState({
    currentCoordinates: [0, 0, -280],
    currentAngles: [-31.429121, -31.429121, -31.429121],
  });
  const loader = new GLTFLoader(); // Loader for 3D-Models.
  const [pathPoints, setPathPoints] = useState([]); //for path line
  const [motorsLoaded, setMotorsLoaded] = useState(false);
  // Update the local storage whenever sceneObjects or cameraPosition changes
  useEffect(() => {
    localStorage.setItem("sceneObjects", JSON.stringify(sceneObjects));
    localStorage.setItem("cameraPosition", cameraPosition.toString());
  }, [sceneObjects, cameraPosition]);

  // Updates the Ref value each time the camera position changes
  useEffect(() => {
    cameraPositionRef.current = cameraPosition;
  }, [cameraPosition]);
  //Toggle camera position when the user clicks 'Switch view' in the Controls GUI
  const toggleCameraPosition = () => {
    if (cameraRef.current) {
      if (cameraPositionRef.current === 1) {
        setCameraPosition(2);
        cameraRef.current.position.set(0, 0, -1);
      } else {
        setCameraPosition(1);
        cameraRef.current.position.set(0, 700, 0);
      }
    }
  };

  useEffect(() => {
    const loadInitialRobotState = () => {
      const savedState = localStorage.getItem('robotState');
      if (savedState) {
        setRobotState(JSON.parse(savedState));
      }
    };

    loadInitialRobotState();
    window.addEventListener('robotStateUpdated', loadInitialRobotState);

    return () => {
      window.removeEventListener('robotStateUpdated', loadInitialRobotState);
    };
  }, []);

  //Reset angles and coordinates when the user clicks 'resetScene' in the Controls GUI
  const resetScene = () => {
    // Reset the coordinates and angles
    setRobotState({
      currentCoordinates: [0, 0, -280], // Original coordinates
      currentAngles: [-31.429121, -31.429121, -31.429121], // Original angles
    });

    setPathPoints([]);
  };
  // Adopt coordinates from the Controls GUI and calculate motor angles using inverse kinematics – used only for offline control via the Controls
  const handleCoordinateChange = (index, value) => {
    setRobotState((prevState) => {
      // Update new coordinates based on the changed value
      const newCoordinates = [...prevState.currentCoordinates];
      newCoordinates[index] = value;

      // Calculation of new angles based on the new coordinates
      const [newX, newY, newZ] = newCoordinates;
      const result = delta_calcInverse(newX, newY, newZ);

      if (result.status === 0) {
        // Check if the calculation was successful
        const newAngles = [result.theta1, result.theta2, result.theta3];
        return {
          ...prevState,
          currentCoordinates: newCoordinates,
          currentAngles: newAngles,
        };
      } else {
        // Error handling in case the calculation fails
        console.error("Fehler bei der Berechnung der Motorwinkel");
        return prevState; // No change if an error occurs
      }
    });
  };

  // Main Effect hook: Creates the 3D scene, adds camera and light, and initializes the rendering loop
  useEffect(() => {
    // scene, kamera, renderer
    const scene = new THREE.Scene();
    const camera = new THREE.PerspectiveCamera(
      75,
      mountRef.current.clientWidth / mountRef.current.clientHeight,
      0.1,
      2000
    );
    cameraRef.current = camera;
    const renderer = new THREE.WebGLRenderer();
    renderer.setSize(
      mountRef.current.clientWidth,
      mountRef.current.clientHeight
    );

    renderer.setClearColor(0xffffff);
    mountRef.current.appendChild(renderer.domElement);

    if (cameraPosition === 1) {
      camera.position.set(0, 700, 0);
    } else {
      camera.position.set(0, 0, -1);
    }
    const handleResize = () => {
      // Update camera aspect ratio and renderer size
      camera.aspect =
        mountRef.current.clientWidth / mountRef.current.clientHeight;
      camera.updateProjectionMatrix();
      renderer.setSize(
        mountRef.current.clientWidth,
        mountRef.current.clientHeight
      );
    };

    // Add event listener for window resize
    window.addEventListener("resize", handleResize);
    //********************************************************************add light************************************************************************** */
    const light = new THREE.AmbientLight(0x404040); // weiches Licht
    scene.add(light);
    const directionalLight = new THREE.DirectionalLight(0xffffff, 1);
    directionalLight.position.set(1, 2, 3);
    scene.add(directionalLight);
    //*****************************************************************initialize path line***************************************************************************** */
    // Pfadlinie initialisieren
    const material = new THREE.LineBasicMaterial({
      color: 0xff0000,
      linewidth: 1,
    });
    const geometry = new THREE.BufferGeometry();
    const line = new THREE.Line(geometry, material);
    scene.add(line);

    //***************************************************************add coordinate grid******************************************************************************* */
    // Koordinatengitter hinzufügen
    let gridHelperXY = new THREE.GridHelper(
      sceneObjects.gridSize,
      sceneObjects.gridDivisions
    );
    scene.add(gridHelperXY);
    gridHelperXY.rotation.x = Math.PI / 2;
    gridHelperXY.position.set(0, 0, -500);
    gridHelperXY.visible = sceneObjects.gridVisible;
    //*********************************************************add Axes for the endeffector************************************************************************************* */
    const axesHelper = createAxesHelper(150, 3);
    scene.add(axesHelper);
    axesHelper.position.x = robotState.currentCoordinates[0];
    axesHelper.position.y = robotState.currentCoordinates[1];
    axesHelper.position.z = robotState.currentCoordinates[2];
    axesHelper.visible = sceneObjects.axesVisible;

    //*******************************************************add base models and Proxy Model within the scene*************************************************************************************** */
    // Proxy Model
    const baseGeometry = new THREE.CylinderGeometry(
      baseRadius,
      baseRadius,
      10,
      32
    );
    const baseMaterial = new THREE.MeshBasicMaterial({
      color: 0x000000,
      opacity: 0.5,
      transparent: true,
    });
    const simpleBase = new THREE.Mesh(baseGeometry, baseMaterial);
    simpleBase.rotation.x = Math.PI / 2;
    scene.add(simpleBase);

    // base model
    let base;
    const loadBase = () => {
      if (!objects.base) {
        loader.load(
          "/model/base.glb",
          function (gltf) {
            base = gltf.scene;
            base.scale.set(10, 10, 10);
            base.position.set(0, 0, 90);
            base.rotation.z = Math.PI / 6;
            scene.add(base);
            base.visible = sceneObjects.baseVisible;
            setObjects((prev) => ({ ...prev, base })); // Storing the base in the state
          },
          undefined,
          function (error) {
            console.error("Error loading the base model:", error);
          }
        );
      } else {
        base.visible = true;
      }
    };

    const removeBase = () => {
      if (base) {
        scene.remove(base);
        if (base.material) base.material.dispose();
        if (base.geometry) base.geometry.dispose();
        base = null;
      }
    };

    if (sceneObjects.baseVisible && !objects.base) {
      loadBase();
    } else if (objects.base) {
      objects.base.visible = sceneObjects.baseVisible;
    }
    //***************************************************************add endeffector models(with gripper) and Proxy Model within the scene******************************************************************************* */
    // Proxy Model
    const effectorGeometry = new THREE.CylinderGeometry(
      effectorRadius,
      effectorRadius,
      10,
      32
    );
    const effectorMaterial = new THREE.MeshBasicMaterial({
      color: 0x000000,
      opacity: 0.5,
      transparent: true,
    });
    const simpleEffector = new THREE.Mesh(effectorGeometry, effectorMaterial);
    simpleEffector.position.x = robotState.currentCoordinates[0];
    simpleEffector.position.y = robotState.currentCoordinates[1];
    simpleEffector.position.z = robotState.currentCoordinates[2];
    simpleEffector.rotation.x = Math.PI / 2;
    scene.add(simpleEffector);
    //endeffector models(with gripper)
    let effector;
    const loadEffector = () => {
      if (!objects.effector) {
        const modelPath = `/model/${settings.gripper}.glb`;
        loader.load(
          modelPath,
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
            effector.visible = sceneObjects.endEffectorVisible;
            setObjects((prev) => ({ ...prev, effector }));
          },
          undefined,
          function (error) {
            console.error("Error loading the end effector model:", error);
          }
        );
      } else {
        effector.visible = true;
      }
    };

    const removeEffector = () => {
      if (effector) {
        scene.remove(effector);
        if (effector.material) effector.material.dispose();
        if (effector.geometry) effector.geometry.dispose();
        effector = null;
      }
    };

    if (sceneObjects.endEffectorVisible && !objects.effector) {
      loadEffector();
    } else if (objects.effector) {
      objects.effector.visible = sceneObjects.endEffectorVisible;
    }
    //********************************************************* add motor models and Proxy Model within the scene************************************************************************************* */
    //Proxy Model
    const simpleMotorsArray = [];

    basePositions.forEach((position, index) => {
      const motorMaterial = new THREE.MeshBasicMaterial({ color: 0xff0000 });
      const motorGeometry = new THREE.SphereGeometry(15, 32, 32);
      const simpleMotors = new THREE.Mesh(motorGeometry, motorMaterial);
      simpleMotors.position.set(position.x, position.y, 0);
      scene.add(simpleMotors);

      // Add the created mesh object to the array
      simpleMotorsArray.push(simpleMotors);
    });
    // motor models
    const modelNames = ["motor1", "motor2", "motor3"];
    const motors = {};
    const loadMotors = () => {
      if (motorsLoaded) return;

      modelNames.forEach((modelName) => {
        loader.load(
          `/model/${modelName}.glb`,
          function (gltf) {
            const motor = gltf.scene;
            scene.add(motor);
            motor.scale.set(10, 10, 10);
            motor.position.set(0, 0, 90);
            motor.rotation.z = Math.PI / 6;
            motor.visible = sceneObjects.motorsVisible;
            motors[modelName] = motor;
          },
          undefined,
          function (error) {
            console.error(error);
          }
        );
      });
      setMotorsLoaded(true); //  Set the state that the motors are loaded
    };

    const removeMotor = (motorName) => {
      const motor = motors[motorName];
      if (motor) {
        // Remove the motor object from the scene
        scene.remove(motor);

        // Dispose of the mesh material
        if (motor.material) {
          motor.material.dispose();
        }

        // Dispose of the mesh geometry
        if (motor.geometry) {
          motor.geometry.dispose();
        }

        // Remove the object from the motors directory
        delete motors[motorName];
      }
    };

    if (sceneObjects.motorsVisible && !motorsLoaded) {
      loadMotors();
    } else if (objects.motors) {
      Object.values(objects.motors).forEach(
        (motor) => (motor.visible = sceneObjects.motorsVisible)
      );
    }
    //*******************************************************Add workspace cylinder************************************************************************************** */
    const workspaceGeometry = new THREE.CylinderGeometry(200, 200, 200, 32); // Radius, radius, height, number of segments
    const workspaceMaterial = new THREE.MeshBasicMaterial({
      color: 0x888888,
      transparent: true,
      opacity: 0.3,
      depthWrite: false, // Verhindert das Schreiben in den Tiefenpuffer
    });
    const workspaceCylinder = new THREE.Mesh(
      workspaceGeometry,
      workspaceMaterial
    );
    workspaceCylinder.position.z = -380; // Center between -280 and -480
    workspaceCylinder.rotation.x = Math.PI / 2; // Rotation around the X-axis to set the cylinder upright
    workspaceCylinder.renderOrder = 999;
    scene.add(workspaceCylinder);

    //************************************************************** Dat.gui Component ******************************************************************************** */

    const gui = new dat.GUI({ autoPlace: false });
    gui.close();
    // Append the GUI to a specific HTML container element
    document.getElementById("gui-container").appendChild(gui.domElement);
    // Add a button to the GUI for toggling the camera view, linked to the toggleCameraPosition function
    gui
      .add({ toggleCamera: () => toggleCameraPosition() }, "toggleCamera")
      .name("Switch View");
    // Add a button to the GUI for resetting the entire scene
    gui.add({ resetScene }, "resetScene").name("Reset Scene");
    // Create a folder in the GUI for managing effector coordinates
    const coordinatesFolder = gui.addFolder("Effector Coordinates");

    // Add sliders to adjust the X, Y, and Z coordinates of the robot's effector, with specified ranges
    // Each slider calls handleCoordinateChange on value change to update the state
    coordinatesFolder
      .add(robotState.currentCoordinates, "0", -200, 200)
      .name("X")
      .onChange((val) => handleCoordinateChange(0, val));
    coordinatesFolder
      .add(robotState.currentCoordinates, "1", -200, 200)
      .name("Y")
      .onChange((val) => handleCoordinateChange(1, val));
    coordinatesFolder
      .add(robotState.currentCoordinates, "2", 280, 480)
      .name("Z")
      .onChange((val) => handleCoordinateChange(2, -val));
    coordinatesFolder.close();
    // Create another folder for general scene configuration options
    const gridFolder = gui.addFolder("Scene Configuration");
    // Add slider to adjust the size of the grid in the scene, re-creating the grid helper on change
    gridFolder
      .add(sceneObjects, "gridSize", 100, 10000)
      .name("Grid Size")
      .onChange((value) => {
        scene.remove(gridHelperXY);
        gridHelperXY = new THREE.GridHelper(value, sceneObjects.gridDivisions);
        gridHelperXY.rotation.x = Math.PI / 2;
        gridHelperXY.position.set(0, 0, -500);
        gridHelperXY.visible = sceneObjects.gridVisible;
        scene.add(gridHelperXY);
      });
    // Add slider to adjust the number of divisions in the grid, re-creating the grid helper on change
    gridFolder
      .add(sceneObjects, "gridDivisions", 10, 100)
      .name("Grid Divisions")
      .onChange((value) => {
        scene.remove(gridHelperXY);
        gridHelperXY = new THREE.GridHelper(sceneObjects.gridSize, value);
        gridHelperXY.rotation.x = Math.PI / 2;
        gridHelperXY.position.set(0, 0, -500);
        gridHelperXY.visible = sceneObjects.gridVisible;
        scene.add(gridHelperXY);
      });
    // Toggle visibility of the grid in the scene
    gridFolder
      .add(sceneObjects, "gridVisible")
      .name("Show Grid")
      .onChange((value) => {
        gridHelperXY.visible = value;
        setSceneObjects((prevState) => ({ ...prevState, gridVisible: value }));
      });
    // Toggle visibility of the axes helper in the scene
    gridFolder
      .add(sceneObjects, "axesVisible")
      .name("Show Axes")
      .onChange((value) => {
        axesHelper.visible = value;
        setSceneObjects((prevState) => ({ ...prevState, axesVisible: value }));
      });
    // Toggle visibility of the end effector in the scene, managing additional components on change
    gridFolder
      .add(sceneObjects, "endEffectorVisible")
      .name("Show Endeffector")
      .onChange((value) => {
        if (value) {
          loadEffector();
        } else {
          removeEffector();
        }
        simpleEffector.visible = !value;
        setSceneObjects((prevState) => ({
          ...prevState,
          endEffectorVisible: value,
        }));
      });
    // Toggle visibility of the base, managing additional components on change
    gridFolder
      .add(sceneObjects, "baseVisible")
      .name("Show Base")
      .onChange((value) => {
        if (value) {
          loadBase();
        } else {
          removeBase();
        }
        simpleBase.visible = !value;
        setSceneObjects((prevState) => ({ ...prevState, baseVisible: value }));
      });
    // Manage visibility and loading of motor objects in the scene, handling resource management on change
    gridFolder
      .add(sceneObjects, "motorsVisible")
      .name("Show Motors")
      .onChange((value) => {
        if (value) {
          if (!motorsLoaded) {
            loadMotors(); // Load the motors if not already loaded
          } else {
            // Set visibility for all already loaded motors
            Object.keys(motors).forEach((key) => {
              if (motors[key]) {
                motors[key].visible = value;
              }
            });
          }
        } else {
          // Remove motors and free up resources
          Object.keys(motors).forEach((key) => {
            removeMotor(key);
          });
          setMotorsLoaded(false); // Reset the state indicating that motors are not loaded
        }
        simpleMotorsArray[0].visible = !value;
        simpleMotorsArray[1].visible = !value;
        simpleMotorsArray[2].visible = !value;
        setSceneObjects((prevState) => ({
          ...prevState,
          motorsVisible: value,
        }));
      });

    gridFolder.close();
    /****************************************************************** Controls *****************************************************************************/
    const controls = new OrbitControls(camera, renderer.domElement);
    controls.enableDamping = true; //Optional, for a softer 'damping' effect
    controls.dampingFactor = 0.05;
    controls.screenSpacePanning = true;
    // controls.maxPolarAngle = Math.PI / 2;
    controls.maxDistance = 1500;
    controls.enablePan = true;
    controls.panSpeed = 0.5;
    controls.target.set(0, 0, 0);

    controls.update();
    //****************************************************************Scene settings****************************************************************************** */
    scene.rotation.x = Math.PI;
    scene.rotation.z = Math.PI / 2;
    // Saving objects for later updating
    setObjects({
      scene,
      camera,
      renderer,
      base,
      simpleBase,
      effector,
      simpleEffector,
      controls,
      axesHelper,
      line,
    });
    //*****************************************************************Render-loop and Cleanup***************************************************************************** */
    //
    const animate = () => {
      requestAnimationFrame(animate);

      controls.update();
      renderer.render(scene, camera);
    };
    animate();
    return () => {
      window.removeEventListener("resize", handleResize);
      renderer.dispose();
      controls.dispose();
      gui.destroy();
    };
  }, []);

  // Update of joints and arms
  useEffect(() => {
    if (!objects.scene) return;
    // Clear all previous arms and joints
    const toRemove = [];
    objects.scene.traverse((child) => {
      if (child.userData.type === "joint" || child.userData.type === "arm") {
        toRemove.push(child);
      }
    });
    toRemove.forEach((child) => {
      objects.scene.remove(child);
    });

    // Update and recreate joints and arms
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
      // Creating new joints
      const jointGeometry = new THREE.SphereGeometry(10, 32, 32);
      const joint = new THREE.Mesh(jointGeometry, jointMaterial);
      joint.position.set(position.x, position.y, position.z);
      joint.userData.type = "joint";
      objects.scene.add(joint);

      // Upper arms
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

      // Lower arms
      const endEffectorPosition = new THREE.Vector3(
        robotState.currentCoordinates[0] +
          effectorRadius * Math.cos((index * 2 * Math.PI) / 3),
        robotState.currentCoordinates[1] +
          effectorRadius * Math.sin((index * 2 * Math.PI) / 3),
        robotState.currentCoordinates[2]
      );
      // Lower arm model, bug: the orientation and alignment are cumbersome
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

      // Simple lower arm, works fine but arm length is fixed
      // const lowerArmGeometry = new THREE.CylinderGeometry(5, 5, lowerArmLength, 32);
      // const lowerArm = new THREE.Mesh(lowerArmGeometry, lowerArmMaterial);
      // lowerArm.position.copy(jointPosition).lerp(endEffectorPosition, 0.5);
      // lowerArm.lookAt(endEffectorPosition);
      // lowerArm.rotateX(Math.PI / 2);
      // lowerArm.userData.type = "arm";
      // objects.scene.add(lowerArm);

      // Simple lower arm with dynamic length to cover bugs
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

    setObjects(objects); // Update scene objects
  }, [robotState, objects]);

  // Initializes and manages a WebSocket connection to receive real-time robot data
  // useEffect(() => {
  //   // Function to establish a new WebSocket connection
  //   const connectWebSocket = () => {
  //     // Close any existing connections to avoid multiple connections
  //     if (websocketRef.current) {
  //       websocketRef.current.close();
  //     }

  //     // Create a new WebSocket connection
  //     websocketRef.current = new WebSocket("ws://192.168.0.43:80");

  //     // Event: WebSocket is opened
  //     websocketRef.current.onopen = () => {
  //       console.log("WebSocket connected");
  //     };

  //     // Event: Message received
  //     websocketRef.current.onmessage = (event) => {
  //       const data = JSON.parse(event.data);
  //       setRobotState((prevState) => ({
  //         ...prevState,
  //         currentCoordinates: [
  //           data.currentCoordinates[0],
  //           data.currentCoordinates[1],
  //           data.currentCoordinates[2],
  //         ],
  //         currentAngles: [
  //           data.currentAngles[0],
  //           data.currentAngles[1],
  //           data.currentAngles[2],
  //         ],
  //       }));
  //     };

  //     // Event: Error
  //     websocketRef.current.onerror = (error) => {
  //       console.error("WebSocket error:", error);
  //     };

  //     // Event: Connection closed
  //     websocketRef.current.onclose = (event) => {
  //       console.log("WebSocket disconnected", event.reason);
  //       // Auto-reconnect after 5 seconds if not closed normally
  //       if (![1000, 1005].includes(event.code)) {
  //         // 1000: normal closure, 1005: no status rcvd
  //         setTimeout(connectWebSocket, 5000);
  //       }
  //     };
  //   };

  //   // Connect on initial component load
  //   connectWebSocket();

  //   // Cleanup function
  //   return () => {
  //     if (websocketRef.current) {
  //       websocketRef.current.close();
  //     }
  //   };
  // }, []);

  // Updates the 3D scene based on changes in end effector coordinates
  useEffect(() => {
    console.log("Updating positions to: ", objects);
    if (!objects.scene || !objects.axesHelper) {
      return;
    } else {
      objects.axesHelper.position.x = robotState.currentCoordinates[0];
      objects.axesHelper.position.y = robotState.currentCoordinates[1];
      objects.axesHelper.position.z = robotState.currentCoordinates[2];
      setObjects(objects);
      objects.renderer.render(objects.scene, objects.camera);
    }
    if (!objects.scene || !objects.simpleEffector) {
      return;
    } else {
      objects.simpleEffector.position.set(
        robotState.currentCoordinates[0],
        robotState.currentCoordinates[1],
        robotState.currentCoordinates[2]
      );
      setObjects(objects);
      objects.renderer.render(objects.scene, objects.camera);
    }
    if (!objects.scene || !objects.effector) {
      return;
    } else {
      objects.effector.position.set(
        robotState.currentCoordinates[0],
        robotState.currentCoordinates[1],
        robotState.currentCoordinates[2]
      );
      setObjects(objects);
      objects.renderer.render(objects.scene, objects.camera);
    }
  }, [robotState.currentCoordinates, objects]);

  //Updates the path points when coordinates change
  useEffect(() => {
    if (robotState.currentCoordinates.length > 0) {
      setPathPoints((prev) => [
        ...prev,
        new THREE.Vector3(...robotState.currentCoordinates),
      ]);
    }
  }, [robotState.currentCoordinates]);
  // Cleans up and updates the geometric representation of the robot path when coordinates change
  useEffect(() => {
    if (objects.line && pathPoints.length > 0) {
      const geometry = new THREE.BufferGeometry().setFromPoints(pathPoints);
      objects.line.geometry.dispose(); // Clean up old geometry
      objects.line.geometry = geometry; // Assign new geometry
      objects.line.geometry.verticesNeedUpdate = true; // Set flag to force update
    }
  }, [pathPoints, objects.line]);

  //useEffect to load colormode from localstorage
  useEffect(() => {
    let savedSettings = localStorage.getItem("settings");
    if (savedSettings) {
      savedSettings = JSON.parse(savedSettings);
      if (savedSettings.darkMode) {
        console.log(savedSettings.darkMode);
        document.body.classList.add("dark-mode");
      }
    }
  }, []);
  // Main component for the digital twin model, including visual representation and controls
  return (
    <>
      <div
        style={{ display: "flex", justifyContent: "center", height: "80vh" }}
      >
        <div
          ref={mountRef}
          style={{ width: "90%", height: "80vh", border: "2px solid black" }}
        />
        <div
          id="gui-overlay-container"
          style={{
            position: "absolute",
            top: "25",
            right: "5vw",
            width: "300px",
            height: "20%",
            display: "flex",
            flexDirection: "column",
            alignItems: "flex-end",
          }}
        >
          <div id="gui-container"></div>
          <div className="my-2"></div> {/* Divider */}
          <div
            id="statusOverlay"
            style={{
              color: "white",
              backgroundColor: "rgba(0, 0, 0, 0.5)",
              padding: "3px",
            }}
          >
            Coordinates:{" "}
            {robotState.currentCoordinates
              .map((coord) => coord.toFixed(2))
              .join(", ")}
            <br />
            Angles:{" "}
            {robotState.currentAngles
              .map((angle) => angle.toFixed(2))
              .join(", ")}
          </div>
        </div>
      </div>
      <RobotStateDisplay></RobotStateDisplay>
    </>
  );
};

export default DigitalTwin;
