import React, { createContext, useContext, useState } from 'react';

const StringContext = createContext();

export function useString() {
  return useContext(StringContext);
}

export function StringProvider({ children }) {
  const [sharedString, setSharedString] = useState("Hier Ihren G-Code eingeben...");

  return (
    <StringContext.Provider value={{ sharedString, setSharedString }}>
      {children}
    </StringContext.Provider>
  );
}
