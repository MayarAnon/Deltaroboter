#!/bin/bash

# Erhöht die Robustheit des Skripts
set -euo pipefail

# System auf den aktuellen Stand bringen
sudo apt-get update -y || { echo "Fehler beim Aktualisieren der Paketlisten"; exit 1; }

# Stelle sicher, dass Skriptdateien im Unix-Format sind
for script in node.sh Mosquitto.sh start.sh packages.sh service.sh; do
    dos2unix "$script" || { echo "Fehler beim Konvertieren von $script"; exit 1; }
    chmod +x "$script" || { echo "Fehler beim Konvertieren von $script"; exit 1; }
done

# Skripte für zusätzliche Dienste ausführen
./node.sh || { echo "Fehler beim Ausführen von node.sh"; exit 1; }
./Mosquitto.sh || { echo "Fehler beim Ausführen von Mosquitto.sh"; exit 1; }
./packages.sh || {echo "Fehler beim Ausführen von packages.sh";exit 1;}

#./service.sh || {echo "Fehler beim Ausführen von service.sh";exit 1;}



