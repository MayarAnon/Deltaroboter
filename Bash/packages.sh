#!/bin/bash

# Definieren des Installationspfads (zwei Verzeichnisebenen über dem aktuellen Verzeichnis)
INSTALLATION_PATH=$(realpath ../../)

# Aktualisieren der Paketlisten und Installation der erforderlichen Abhängigkeiten
sudo apt-get update
sudo apt-get install -y cmake git build-essential libssl-dev

# cJSON installieren
echo "Installation von cJSON..."
cd $INSTALLATION_PATH
git clone https://github.com/DaveGamble/cJSON.git
cd cJSON
mkdir build
cd build
cmake ..
make
sudo make install

# paho.mqtt.c installieren
echo "Installation von paho.mqtt.c..."
cd $INSTALLATION_PATH
git clone https://github.com/eclipse/paho.mqtt.c.git
cd paho.mqtt.c
mkdir build
cd build
cmake -DPAHO_WITH_SSL=ON -DPAHO_ENABLE_TESTING=OFF ..
make
sudo make install

# Aktualisieren des shared library cache
sudo ldconfig

echo "Installation abgeschlossen. cJSON und paho.mqtt.c wurden installiert."
