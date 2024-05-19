# Service-Datei in den richtigen Ordner bringen und Systemdienste konfigurieren
echo "Überprüfe die Quelldatei..."
cat deltarobot.service || { echo "Quelldatei nicht gefunden oder leer"; exit 1; }

echo "Kopiere Service-Datei..."
sudo cp -a deltarobot.service /etc/systemd/system/deltarobot.service || { echo "Fehler beim Kopieren der Service-Datei"; exit 1; }

echo "Überprüfe kopierte Datei..."
cat /etc/systemd/system/deltarobot.service || { echo "Kopierte Datei ist leer oder nicht vorhanden"; exit 1; }

echo "Aktualisiere Systemd..."
sudo systemctl daemon-reload
sudo systemctl enable deltarobot.service
sudo systemctl start deltarobot.service || { echo "Fehler beim Starten des Dienstes"; exit 1; }

echo "Alle Vorgänge erfolgreich abgeschlossen."
