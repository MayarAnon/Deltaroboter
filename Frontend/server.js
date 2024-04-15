const express = require('express');
const fs = require('fs');
const path = require('path');
const cors = require('cors');

const app = express();
const port = 3010;

app.use(cors());
// Ordner, in dem die Dateien gespeichert sind
const dataFolder = path.join(__dirname, 'data');

app.use(express.json());

// Endpunkt zum Laden der Dateien im Ordner
app.get('/loadFiles', (req, res) => {
  // Array, um die Daten aus den Dateien zu speichern
  const data = [];

  // Lese alle Dateien im Ordner
  fs.readdirSync(dataFolder).forEach((file) => {
    const filePath = path.join(dataFolder, file);

    try {
      // Prüfen, ob die Datei eine JSON-Datei ist
      if (path.extname(file) === '.json') {
        // Lese den Inhalt der Datei
        const fileContent = fs.readFileSync(filePath, 'utf-8');

        // Füge die geparsten Daten in das Array hinzu
        const jsonData = JSON.parse(fileContent);
        data.push(jsonData);
      }
    } catch (err) {
      console.error(`Fehler beim Lesen der Datei ${file}: ${err}`);
    }
  });

  res.json(data);
});


const gcodeFolder = path.join(__dirname, 'gcode');
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
// Endpunkt zum Speichern eines Programms
app.post('/programs', (req, res) => {
  console.log('Anfrage zum Speichern eines Programms erhalten');
  const programData = req.body; // Die Daten werden im JSON-Format erwartet

  if (!programData || !programData.name || !programData.positions) {
    return res.status(400).json({ error: 'Ungültige Daten' });
  }

  const programName = programData.name;
  const programFilePath = path.join(__dirname, 'data', `${programName}.json`);

  // Daten in eine JSON-Datei schreiben
  fs.writeFile(programFilePath, JSON.stringify(programData), (err) => {
    if (err) {
      console.error('Fehler beim Schreiben der Datei:', err);
      return res.status(500).json({ error: 'Fehler beim Speichern des Programms' });
    }

    console.log(`Programm "${programName}" erfolgreich gespeichert.`);
    res.json({ message: `Programm "${programName}" erfolgreich gespeichert.` });
  });
});


// Endpunkt zum Löschen einer Datei mit einem bestimmten Namen
app.delete('/delete', (req, res) => {
  // Erfassen Sie den Namen der zu löschenden Datei aus der Anfrage
  const fileNameToDelete = req.query.name;

  // Überprüfen Sie, ob ein Dateiname in der Anfrage vorhanden ist
  if (!fileNameToDelete) {
    return res.status(400).json({ error: 'Dateiname nicht angegeben' });
  }

  // Erstellen Sie den vollständigen Dateipfad zur zu löschenden Datei
  const filePathToDelete = path.join(dataFolder, `${fileNameToDelete}.json`);

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

// Statische Dateien bedienen
app.use(express.static(path.join(__dirname, 'build')));



app.listen(port, '0.0.0.0', () => {
  console.log(`Server läuft und hört auf Port ${port}`);
});
