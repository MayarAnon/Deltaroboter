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

# Array zum Speichern der Prozess-IDs
pids=()

# Starte alle Services und Server gleichzeitig mit Zeitstempel in den Logs
start_services() {
    # Starte den Node.js-Server
    cd "$BASE_DIR/Backend/WebServer/"
    nohup node server.js 2>&1 | with_timestamp >> "$LOG_DIR/server.log" &
    pids+=($!)

    # Starte den State Publisher
    cd "$BASE_DIR/Backend/StatePublisher/"
    nohup python statepublisher.py 2>&1 | with_timestamp >> "$LOG_DIR/statepublisher.log" &
    pids+=($!)

    # Starte das Homing-Skript
    cd "$BASE_DIR/Backend/Homing/"
    nohup python homing.py 2>&1 | with_timestamp >> "$LOG_DIR/homing.log" &
    pids+=($!)

    # Starte das Gripper Control Skript
    cd "$BASE_DIR/Backend/GripperControl/"
    nohup python gripperControl.py 2>&1 | with_timestamp >> "$LOG_DIR/gripperControl.log" &
    pids+=($!)

    # Starte das Motion Planning Programm
    cd "$BASE_DIR/Backend/MotionPlaning/"
    nohup ./MotionPlaning 2>&1 | with_timestamp >> "$LOG_DIR/motionPlaning.log" &
    pids+=($!)

    # Starte den Motor Controller
    cd "$BASE_DIR/Backend/MotorControl/"
    sudo ./MotorController &
    pids+=($!)
}

start_services

# Trap zum Stoppen der Dienste
cleanup() {
    echo "Beende alle Dienste..."
    for pid in "${pids[@]}"; do
        kill -9 "$pid" >/dev/null 2>&1
    done
    echo "Alle Dienste wurden gestoppt."
    exit 0
}

trap 'cleanup' SIGINT SIGTERM EXIT



# Starte die Dienste in einer Endlosschleife
while true; do
    sleep 1
done
