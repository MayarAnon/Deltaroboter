#!/bin/bash
# Config-Datei Netzwerk in den richtigen Ordner bringen und Systemdienste konfigurieren
echo "Überprüfe die Quelldatei..."
cat dhcpcd.conf || { echo "Quelldatei nicht gefunden oder leer"; exit 1; }

echo "Kopiere Config-Datei..."
sudo cp -a dhcpcd.conf /etc/dhcpcd.conf || { echo "Fehler beim Kopieren der Config-Datei"; exit 1; }

echo "Überprüfe kopierte Datei..."
cat /etc/dhcpcd.conf || { echo "Kopierte Datei ist leer oder nicht vorhanden"; exit 1; }

echo "Neustart des Services..."

sudo systemctl restart dhcpcd

echo "Port für Node freigeben"

sudo setcap 'cap_net_bind_service=+ep' `which node`
