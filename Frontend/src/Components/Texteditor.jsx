import React, { useState, useEffect } from "react";
import hljs from "highlight.js";
import "highlight.js/styles/monokai.css";

// Subfunction for highlighting the G-code
const highlightCode = (code) => {
  return { __html: hljs.highlightAuto(code).value };
};

// Component for displaying the highlighted code when code review expands
const HighlightedCode = ({ text }) => {
  const [highlightedText, setHighlightedText] = useState({ __html: "" });

  useEffect(() => {
    // Set debounce timer
    const timer = setTimeout(() => {
      // Call highlighting function and save the result in state
      setHighlightedText(highlightCode(text));
    }, 100);

    // Cleanup function to clear the timer on component unmount or re-render
    return () => clearTimeout(timer);
  }, [text]);

  return (
    <pre className="mt-4">
      <code
        className="language-gcode "
        dangerouslySetInnerHTML={highlightedText}
      ></code>
    </pre>
  );
};

export {  HighlightedCode };
