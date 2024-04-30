# Service-Datei in den richtigen Ordner bringen und Systemdienste konfigurieren
sudo cp deltarobot.service /etc/systemd/system/deltarobot.service || { echo "Fehler beim Kopieren der Service-Datei"; exit 1; }
sudo systemctl daemon-reload
sudo systemctl enable deltarobot.service
sudo systemctl start deltarobot.service || { echo "Fehler beim Starten des Dienstes"; exit 1; }



# Rechte im Verzeichnis /home/pi/Deltarobot nur für den Benutzer 'pi' setzen
chmod u+rwx,go= -R /home/pi/Deltarobot || { echo "Fehler beim Setzen der Berechtigungen"; exit 1; }

echo "Alle Vorgänge erfolgreich abgeschlossen."