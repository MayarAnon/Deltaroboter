#!/bin/bash

# Erhöht die Robustheit des Skripts
set -euo pipefail

# System auf den aktuellen Stand bringen
sudo apt-get update -y || { echo "Fehler beim Aktualisieren der Paketlisten"; exit 1; }

# Stelle sicher, dass Skriptdateien im Unix-Format sind
for script in node.sh Mosquitto.sh start.sh; do
    dos2unix "$script" || { echo "Fehler beim Konvertieren von $script"; exit 1; }
done

# Service-Datei in den richtigen Ordner bringen und Systemdienste konfigurieren
sudo cp deltarobot.service /etc/systemd/system/deltarobot.service || { echo "Fehler beim Kopieren der Service-Datei"; exit 1; }
sudo systemctl daemon-reload
sudo systemctl enable deltarobot.service
sudo systemctl start deltarobot.service || { echo "Fehler beim Starten des Dienstes"; exit 1; }

# Skripte für zusätzliche Dienste ausführen
bash node.sh || { echo "Fehler beim Ausführen von node.sh"; exit 1; }
bash Mosquitto.sh || { echo "Fehler beim Ausführen von Mosquitto.sh"; exit 1; }

# Rechte im Verzeichnis /home/pi/Deltarobot nur für den Benutzer 'pi' setzen
chmod u+rwx,go= -R /home/pi/Deltarobot || { echo "Fehler beim Setzen der Berechtigungen"; exit 1; }

echo "Alle Vorgänge erfolgreich abgeschlossen."