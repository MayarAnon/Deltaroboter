// utils/parseGCode.js

/**
 * Parses a single part of a G-Code line.
 * @param {string} part The part of the G-Code line to parse.
 * @returns An object representing the parsed part.
 */
const parsePart = (part) => {
    const letter = part[0].toUpperCase();
    if (['X', 'Y', 'Z'].includes(letter)) {
      const value = parseFloat(part.slice(1));
      return { [letter.toLowerCase()]: value };
    } else if (part.toUpperCase() === 'OPEN') {
      return { gripperOpen: true };
    } else if (part.toUpperCase() === 'CLOSED') {
      return { gripperOpen: false };
    } else if (letter === 'G') {
      const command = part.toUpperCase();
      return { command };
    } else if (letter === 'F') {
      const feedRate = parseFloat(part.slice(1));
      return { feedRate };
    } else {
      throw new Error(`Unknown part: ${part}`);
    }
  };
  
  /**
   * Parses a G-Code string into a sequence of commands.
   * @param {string} gCodeString The G-Code string to parse.
   * @returns An array of objects, each representing a parsed line of G-Code.
   */
  export const parseGCodeToSequence = (gCodeString) => {
    return gCodeString.split('\n')
      .filter(line => line.trim() !== '')
      .map(line => {
        const parts = line.trim().split(/\s+/);
        return parts.reduce((parsed, part) => {
          try {
            const partResult = parsePart(part);
            return { ...parsed, ...partResult };
          } catch (error) {
            console.error(error.message);
            return parsed;
          }
        }, { x: 0, y: 0, z: 0, gripperOpen: false, command: null, feedRate: null });
      });
  };
  /**
 * Konvertiert eine Sequenz von Objekten in G-Code-Zeilen.
 * @param {Array} sequences Die Sequenz von Objekten.
 * @returns {string} Ein String, der den G-Code darstellt.
 */
export const convertSequencesToGCode = (sequences) => {
  return sequences.map(sequence => {
    // Standardbefehl für Bewegung
    let gCodeLine = 'G0';

    // Fügt die X, Y, Z Koordinaten hinzu, falls vorhanden
    if (sequence.x !== undefined) gCodeLine += ` X${sequence.x}`;
    if (sequence.y !== undefined) gCodeLine += ` Y${sequence.y}`;
    if (sequence.z !== undefined) gCodeLine += ` Z${sequence.z}`;
    console.log(`sequence.gripperActive: ${sequence.gripperOpen}`)
    // Fügt den Gripper-Status hinzu
    gCodeLine += sequence.gripperOpen ? ' OPEN' : ' CLOSED';

    return gCodeLine;
  }).join('\n'); // Verbindet jede Zeile mit einem Zeilenumbruch
};
