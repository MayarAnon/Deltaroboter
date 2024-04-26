const express = require('express');
const WebSocket = require('ws');
const http = require('http');
const fs = require('fs');
const path = require('path');
const cors = require('cors');
const MqttClient = require("./mqttClient");
const app = express();
const port = 3010;
// HTTP-Server auf dem Express-App basiert, für WebSockets erforderlich
const server = http.createServer(app);
const wss = new WebSocket.Server({ server });

// MQTT-Client instanziieren
const serviceName = "Webserver"; 
let mqttClient;
async function setupMqttClient() {
  const mqtt = new MqttClient(serviceName);
  mqttClient = await mqtt;
  await mqttClient.subscribe('robot/state');
}
setupMqttClient().then(() => {
  console.log("MQTT-Client verbunden und bereit.");
}).catch((error) => {
  console.error("Fehler beim Verbinden mit MQTT:", error);
});

app.use(cors());
app.use(express.json());

const gcodeFolder = path.join(__dirname, '../../Frontend/gcode');

// Endpunkt zum Laden der GCode-Dateien im Ordner
app.get('/loadGCodeFiles', (req, res) => {
  const data = [];

  fs.readdirSync(gcodeFolder).forEach(file => {
    const filePath = path.join(gcodeFolder, file);

    try {
      // Prüfen, ob die Datei eine GCode-Datei ist
      if (path.extname(file).toLowerCase() === '.gcode') {
        // Lese den Inhalt der Datei
        const fileContent = fs.readFileSync(filePath, 'utf-8');

        // Füge den Inhalt der Datei in das Array hinzu
        data.push({ fileName: file, content: fileContent });
      }
    } catch (err) {
      console.error(`Fehler beim Lesen der Datei ${file}: ${err}`);
    }
  });

  res.json(data);
});
// Endpunkt zum Löschen einer Datei mit einem bestimmten Namen
app.delete('/deleteGCode', (req, res) => {
  // Erfassen Sie den Namen der zu löschenden Datei aus der Anfrage
  const fileNameToDelete = req.query.name;
  // Überprüfen Sie, ob ein Dateiname in der Anfrage vorhanden ist
  if (!fileNameToDelete) {
    return res.status(400).json({ error: 'Dateiname nicht angegeben' });
  }

  // Erstellen Sie den vollständigen Dateipfad zur zu löschenden Datei
  const filePathToDelete = path.join(gcodeFolder, `${fileNameToDelete}`);

  // Überprüfen Sie, ob die Datei existiert
  if (fs.existsSync(filePathToDelete)) {
    // Löschen Sie die Datei
    fs.unlinkSync(filePathToDelete);
    console.log(`Datei "${fileNameToDelete}" erfolgreich gelöscht.`);
    res.json({ message: `Datei "${fileNameToDelete}" erfolgreich gelöscht.` });
  } else {
    res.status(404).json({ error: `Datei "${fileNameToDelete}" nicht gefunden` });
  }
});
// Endpunkt zum Speichern eines Programms
app.post('/gcode', (req, res) => {
  console.log('Anfrage zum Speichern eines Programms erhalten');
  const programData = req.body; // Die Daten werden im JSON-Format erwartet

  if (!programData || !programData.name || !programData.content) {
    return res.status(400).json({ error: 'Ungültige Daten' });
  }

  const programName = programData.name;
  const programFilePath = path.join(__dirname, 'gcode', `${programName}.gcode`);

  // Daten in eine JSON-Datei schreiben
  fs.writeFile(programFilePath,programData.content, (err) => {
    if (err) {
      console.error('Fehler beim Schreiben der Datei:', err);
      return res.status(500).json({ error: 'Fehler beim Speichern des Programms' });
    }

    console.log(`Programm "${programName}" erfolgreich gespeichert.`);
    res.json({ message: `Programm "${programName}" erfolgreich gespeichert.` });
  });
});
// Endpunkt zum übermitteln von den Einstellungen
app.post('/updateSettings', async (req, res) => {
//   // Beispiel für die Verwendung der Funktion
// const settings = {
//   gripperMode: "parallelGripper",
//   motorSpeed: 75
// };

  const settings = req.body;
  if (!settings) {
    return res.status(400).json({ error: 'Keine Einstellungen gesendet' });
  }

  try {
    if (!mqttClient || !mqttClient.connected) {
      throw new Error("MQTT-Client ist nicht verbunden");
    }

    // Durchlaufen aller Einstellungen und Publizieren auf den entsprechenden Topics
    if (settings.gripperMode) {
      await mqttClient.publish('gripper/mode', settings.gripperMode);
    }
    if (settings.motorSpeed !== undefined) { // Einschließlich 0 als gültiger Wert
      await mqttClient.publish('motors/speed', settings.motorSpeed.toString());
    }

    res.status(200).json({ message: 'Einstellungen erfolgreich aktualisiert und publiziert' });
  } catch (error) {
    console.error('Fehler beim Publizieren der Einstellungen:', error);
    res.status(500).json({ error: 'Fehler beim Publizieren der Einstellungen' });
  }
});
// Endpunkt zum stoppen aller motoren
app.post('/motors/stop', async (req, res) => {
  const stopSignal = req.body.stop;
  if (typeof stopSignal !== 'boolean') {
    return res.status(400).json({ error: 'Ungültige Daten, erwartet true' });
  }

  try {
    await mqttClient.publish('motors/stop', JSON.stringify(stopSignal));
    res.status(200).json({ message: 'Motorstopp signalisiert.' });
  } catch (error) {
    console.error('Fehler beim Publizieren des Motorstopps:', error);
    res.status(500).json({ error: 'Fehler beim Publizieren des Motorstopps' });
  }
});
// Endpunkt für die Übermittelung von den koordinaten fürs manualmode
app.post('/manual/control/coordinates', async (req, res) => {
  const coordinates = req.body.coordinates;
  if (!Array.isArray(coordinates) || coordinates.length !== 4) {
    return res.status(400).json({ error: 'Ungültige Koordinaten, erwartet ein Array von 4 Elementen' });
  }

  try {
    await mqttClient.publish('manual/control/coordinates', JSON.stringify(coordinates));
    res.status(200).json({ message: 'Koordinaten erfolgreich aktualisiert.' });
  } catch (error) {
    console.error('Fehler beim Publizieren der Koordinaten:', error);
    res.status(500).json({ error: 'Fehler beim Publizieren der Koordinaten' });
  }
});
// Endpunkt für die Übermittelung von der Greifersteuerung fürs manualmode
app.post('/manual/control/gripper', async (req, res) => {
  const gripperValue = req.body.gripper;
  if (typeof gripperValue !== 'number') {
    return res.status(400).json({ error: 'Ungültige Greiferstärke, erwartet eine Zahl' });
  }

  try {
    await mqttClient.publish('manual/control/gripper', gripperValue.toString());
    res.status(200).json({ message: 'Greifersteuerung erfolgreich aktualisiert.' });
  } catch (error) {
    console.error('Fehler beim Publizieren der Greiferstärke:', error);
    res.status(500).json({ error: 'Fehler beim Publizieren der Greiferstärke' });
  }
});
// Endpunkt für die übermittelung des Namen des auszuführenden Programm
app.post('/pickandplace/program', async (req, res) => {
  const program = req.body.program;
  if (typeof program !== 'string') {
    return res.status(400).json({ error: 'Ungültiger Programmwert, erwartet eine Zeichenkette' });
  }

  try {
    await mqttClient.publish('pickandplace/program', program);
    res.status(200).json({ message: 'Programm erfolgreich übermittelt.' });
  } catch (error) {
    console.error('Fehler beim Publizieren des Programms:', error);
    res.status(500).json({ error: 'Fehler beim Publizieren des Programms' });
  }
});

//websocket-mqtt-service
wss.on('connection', function connection(ws) {
  console.log('WebSocket client verbunden.');

  mqttClient.on('message', (topic, message) => {
    if (topic === 'robot/state') {
      ws.send(message.toString()); // Senden der Nachricht an den verbundenen WebSocket-Client
    }
  });

  ws.on('close', () => {
    console.log('WebSocket client getrennt.');
  });

  ws.on('error', (error) => {
    console.error('WebSocket-Fehler:', error);
  });
});


// Statische Dateien bedienen
app.use(express.static(path.join(__dirname, 'build')));

// Anhören sowohl auf dem HTTP- als auch auf dem WebSocket-Server
server.listen(port, () => {
  console.log(`Server und WebSocket laufen auf Port ${port}`);
});

// Handle clean exit
function handleExit(options, err) {
  if (options.cleanup) {
    if (mqttClient) {
      mqttClient.end();
    }
  }
  if (err) console.log(err.stack);
  if (options.exit) process.exit();
}

process.on('exit', handleExit.bind(null, {cleanup: true}));
process.on('SIGINT', handleExit.bind(null, {exit: true}));
process.on('SIGUSR1', handleExit.bind(null, {exit: true}));
process.on('SIGUSR2', handleExit.bind(null, {exit: true}));
process.on('uncaughtException', handleExit.bind(null, {exit: true}));