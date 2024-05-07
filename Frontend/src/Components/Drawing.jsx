import React, { useRef, useEffect } from 'react';
import * as THREE from 'three';
import { Canvas, useFrame, extend, useThree } from '@react-three/fiber';

function CylinderBetweenPoints({ point1, point2, radius, color }) {
  const ref = useRef();
  const { scene } = useThree();

  useEffect(() => {
    const direction = new THREE.Vector3().subVectors(point2, point1);
    const length = direction.length();
    const orientation = new THREE.Matrix4();
    orientation.lookAt(point1, point2, new THREE.Object3D().up);
    orientation.multiply(new THREE.Matrix4().set(1, 0, 0, 0, 0, 0, -1, 0, 0, 1, 0, 0));
    const edgeGeometry = new THREE.CylinderGeometry(radius, radius, length, 8, 1);
    const edge = new THREE.Mesh(edgeGeometry, new THREE.MeshBasicMaterial({ color: color }));
    edge.applyMatrix4(orientation);
    edge.position.x = (point2.x + point1.x) / 2;
    edge.position.y = (point2.y + point1.y) / 2;
    edge.position.z = (point2.z + point1.z) / 2;
    ref.current.add(edge);
  }, [point1, point2, radius, color]);

  return (
    <primitive object={ref.current} />
  );
}

function App() {
  const point1 = new THREE.Vector3(0, 0, 0);
  const point2 = new THREE.Vector3(5, 5, 5);
  
  return (
    <Canvas>
      <ambientLight intensity={0.5} />
      <CylinderBetweenPoints point1={point1} point2={point2} radius={0.1} color="red" />
    </Canvas>
  );
}

export default App;
