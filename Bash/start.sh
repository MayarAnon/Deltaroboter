#!/bin/bash

# Festlegen des Basisverzeichnisses
BASE_DIR="/home/pi/Deltarobot/Backend/WebServer"

# Wechsle in das Verzeichnis oder breche bei Fehler ab
cd "$BASE_DIR" || { echo "Fehler: Konnte nicht in das Verzeichnis $BASE_DIR wechseln"; exit 1; }

# Starte die Services und Server
nohup node server.js > server.log 2>&1 &
nohup python ../StatePublisher/statepublisher.py > statepublisher.log 2>&1 &
nohup python ../Homing/homing.py > homing.log 2>&1 &
nohup python ../GripperControl/gripperControl.py > gripperControl.log 2>&1 &
sudo nohup ../MotorControl/MotorControl > motorControl.log 2>&1 &
nohup ../MotionPlaning/MotionPlaning > motionPlaning.log 2>&1 &

echo "Alle Prozesse wurden gestartet."
