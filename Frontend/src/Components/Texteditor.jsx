import React, { useState, useEffect } from 'react';
import hljs from 'highlight.js';
import 'highlight.js/styles/monokai.css';
import { useString } from './StringContext';

// Unterfunktion zur Highlighting des G-Codes
const highlightCode = (code) => {
  return { __html: hljs.highlightAuto(code).value };
};

// Komponente zur Anzeige des hervorgehobenen Codes
const HighlightedCode = ({ text }) => {
  const [highlightedText, setHighlightedText] = useState({ __html: '' });

  useEffect(() => {
    // Debounce Timer setzen
    const timer = setTimeout(() => {
      // Highlighting-Funktion aufrufen und das Ergebnis im Zustand speichern
      setHighlightedText(highlightCode(text));
    }, 100); // Verzögerung von 100ms

    // Cleanup-Funktion, um den Timer bei Komponenten-Unmount oder bei erneuten Rendern zu löschen
    return () => clearTimeout(timer);
  }, [text]); // Abhängigkeit von `text`, um Effekt bei Änderungen auszulösen

  return (
    <pre className="mt-4">
      <code
        className="language-gcode "
        dangerouslySetInnerHTML={highlightedText}
      ></code>
    </pre>
  );
};

const TextEditor = (props) => {
  const { sharedString, setSharedString } = useString();

  const handleChange = (e) => {
    setSharedString(e.target.value);
  };

  return (
    <div style={{ backgroundColor: props.color }} className="p-4 text-white rounded-xl mx-5 border-4 border-black">
      <textarea
        value={sharedString}
        onChange={handleChange}
        placeholder="Hier Ihren G-Code eingeben..."
        className="w-full h-32 p-2 text-black rounded-md overflow-auto"
        style={{ minHeight: '150px' }}
      ></textarea>
      <HighlightedCode text={sharedString} />
    </div>
  );
};

// Benannte Exports
export { TextEditor, HighlightedCode };
