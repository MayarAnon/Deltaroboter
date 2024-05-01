#!/bin/bash

# Festlegen des Basisverzeichnisses
BASE_DIR=$(realpath ../)

# Überprüfen, ob das Log-Verzeichnis vorhanden ist, andernfalls erstellen
LOG_DIR="$BASE_DIR/log"
if [ ! -d "$LOG_DIR" ]; then
    mkdir "$LOG_DIR" || { echo "Fehler: Konnte das Verzeichnis $LOG_DIR nicht erstellen"; exit 1; }
fi

# Funktion zum Hinzufügen von Zeitstempeln zu Logausgaben
with_timestamp() {
    while IFS= read -r line; do
        echo "$(date +%Y-%m-%d\ %H:%M:%S) $line"
    done
}

# Starte alle Services und Server gleichzeitig mit Zeitstempel in den Logs

# Starte den Node.js-Server
cd "$BASE_DIR/Backend/WebServer/"
nohup node server.js 2>&1 | with_timestamp >> "$LOG_DIR/server.log" &

# Starte den State Publisher
cd "$BASE_DIR/Backend/StatePublisher/"
nohup python statepublisher.py 2>&1 | with_timestamp >> "$LOG_DIR/statepublisher.log" &

# Starte das Homing-Skript
cd "$BASE_DIR/Backend/Homing/"
nohup python homing.py 2>&1 | with_timestamp >> "$LOG_DIR/homing.log" &

# Starte das Gripper Control Skript
cd "$BASE_DIR/Backend/GripperControl/"
nohup python gripperControl.py 2>&1 | with_timestamp >> "$LOG_DIR/gripperControl.log" &

# Starte das Motion Planning Programm
cd "$BASE_DIR/Backend/MotionPlaning/"
nohup ./MotionPlaning 2>&1 | with_timestamp >> "$LOG_DIR/motionPlaning.log" &

# Starte den Motor Controller
cd "$BASE_DIR/Backend/MotorControl/"
sudo ./MotorController 

echo "Alle Dienste wurden gestartet."
