#!/bin/bash

# Festlegen des Basisverzeichnisses
BASE_DIR="/home/pi/Desktop/Deltaroboter/Backend/log"

# Funktion zum Hinzufügen von Zeitstempeln zu Logausgaben
with_timestamp() {
    while IFS= read -r line; do
        echo "$(date +%Y-%m-%d\ %H:%M:%S) $line"
    done
}

# Wechsle in das Verzeichnis oder breche bei Fehler ab
cd "$BASE_DIR" || { echo "Fehler: Konnte nicht in das Verzeichnis $BASE_DIR wechseln"; exit 1; }

# Starte die Services und Server mit Zeitstempel in den Logs
nohup node ../WebServer/server.js 2>&1 | with_timestamp > server.log &
nohup python ../StatePublisher/statepublisher.py 2>&1 | with_timestamp > statepublisher.log &
nohup python ../Homing/homing.py 2>&1 | with_timestamp > homing.log &
nohup python ../GripperControl/gripperControl.py 2>&1 | with_timestamp > gripperControl.log &
nohup ../MotionPlaning/MotionPlaning 2>&1 | with_timestamp > motionPlaning.log &
sudo nohup ../MotorControl/MotorController 2>&1 | with_timestamp > motorControl.log &

echo "Alle Prozesse wurden gestartet."