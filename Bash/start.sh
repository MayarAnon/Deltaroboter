#!/bin/bash

# Festlegen des Basisverzeichnisses
BASE_DIR="/home/pi/Desktop/Deltaroboter/Backend"

# Überprüfen, ob das Log-Verzeichnis vorhanden ist, andernfalls erstellen
LOG_DIR="$BASE_DIR/log"
if [ ! -d "$LOG_DIR" ]; then
    mkdir "$LOG_DIR" || { echo "Fehler: Konnte das Verzeichnis $LOG_DIR nicht erstellen"; exit 1; }
fi

# Wechsle in das Log-Verzeichnis oder breche bei einem Fehler ab
cd "$LOG_DIR" || { echo "Fehler: Konnte nicht in das Verzeichnis $LOG_DIR wechseln"; exit 1; }

# Funktion zum Hinzufügen von Zeitstempeln zu Logausgaben
with_timestamp() {
    while IFS= read -r line; do
        echo "$(date +%Y-%m-%d\ %H:%M:%S) $line"
    done
}

# Starte die Services und Server mit Zeitstempel in den Logs
nohup node "$BASE_DIR/WebServer/server.js" 2>&1 | with_timestamp > server.log &
nohup python "$BASE_DIR/StatePublisher/statepublisher.py" 2>&1 | with_timestamp > statepublisher.log &
nohup python "$BASE_DIR/Homing/homing.py" 2>&1 | with_timestamp > homing.log &
nohup python "$BASE_DIR/GripperControl/gripperControl.py" 2>&1 | with_timestamp > gripperControl.log &
nohup "$BASE_DIR/MotionPlaning/MotionPlaning" 2>&1 | with_timestamp > motionPlaning.log &
cd $BASE_DIR/MotorControl
sudo  ./MotorController

echo "Alle Prozesse wurden gestartet."
