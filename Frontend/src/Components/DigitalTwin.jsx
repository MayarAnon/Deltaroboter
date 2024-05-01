import React, { useEffect, useRef, useState } from 'react';
import * as THREE from 'three';

const DigitalTwin = () => {
    const mountRef = useRef(null);
    const [websocket, setWebsocket] = useState(null);
    const [robotState, setRobotState] = useState(null);

    useEffect(() => {
        function connect() {
            const ws = new WebSocket('ws://deltarobot.local:3010');

            ws.onopen = () => {
                console.log('WebSocket connected');
            };

            ws.onmessage = (event) => {
                const data = JSON.parse(event.data);
                setRobotState(data);
            };

            ws.onerror = (error) => {
                console.error('WebSocket error:', error);
            };

            ws.onclose = (event) => {
                console.log('WebSocket disconnected', event.reason);
                setTimeout(connect, 5000);  // Reconnect every 5 seconds
            };

            setWebsocket(ws);
        }

        connect();
        return () => websocket?.close();
    }, []);

    useEffect(() => {
        let renderer, scene, camera, frameId;

        const initThree = () => {
            // Szene initialisieren
            scene = new THREE.Scene();
            camera = new THREE.PerspectiveCamera(75, mountRef.current.clientWidth / mountRef.current.clientHeight, 0.1, 1000);
            renderer = new THREE.WebGLRenderer();
            renderer.setSize(mountRef.current.clientWidth, mountRef.current.clientHeight);
            mountRef.current.appendChild(renderer.domElement);
            camera.position.set(0, 0, 5);

            // Beleuchtung hinzufügen
            const ambientLight = new THREE.AmbientLight(0x404040);
            scene.add(ambientLight);
            const directionalLight = new THREE.DirectionalLight(0xffffff, 1);
            directionalLight.position.set(1, 2, 3);
            scene.add(directionalLight);
        };

        const animate = () => {
            frameId = requestAnimationFrame(animate);
            renderer.render(scene, camera);
        };

        const resize = () => {
            if (renderer) {
                camera.aspect = mountRef.current.clientWidth / mountRef.current.clientHeight;
                camera.updateProjectionMatrix();
                renderer.setSize(mountRef.current.clientWidth, mountRef.current.clientHeight);
            }
        };

        window.addEventListener('resize', resize);
        initThree();
        frameId = requestAnimationFrame(animate);

        return () => {
            window.removeEventListener('resize', resize);
            cancelAnimationFrame(frameId);
            renderer.dispose();
        };
    }, []);

    // Hier könnte die Logik zur Aktualisierung der Roboterszene basierend auf dem Zustand `robotState` implementiert werden

    return (
        <div>
            <h1>Digital Twin</h1>
            <div ref={mountRef} style={{ width: '800px', height: '600px' }} />
        </div>
    );
};

export default DigitalTwin;
