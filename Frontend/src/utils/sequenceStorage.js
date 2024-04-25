const sequenceKey = 'sequences';

export const saveSequence = (sequence) => {
  try {
    const currentSequences = JSON.parse(localStorage.getItem(sequenceKey)) || [];
    currentSequences.push(sequence);
    localStorage.setItem(sequenceKey, JSON.stringify(currentSequences));
    console.log("Sequence saved successfully.");
  } catch (error) {
    console.error("Error saving sequence:", error.message, error.stack);
  }
};

export const getSequences = () => {
  try {
    return JSON.parse(localStorage.getItem(sequenceKey)) || [];
  } catch (error) {
    console.error("Error getting sequences:", error.message, error.stack);
    return [];
  }
};

export const saveAllSequences = (sequences) => {
  try {
    localStorage.setItem(sequenceKey, JSON.stringify(sequences));
    console.log("All sequences saved successfully.");
  } catch (error) {
    console.error("Error saving all sequences:", error.message, error.stack);
  }
};

export const updateSequence = (id, updatedSequence) => {
  try {
    const currentSequences = JSON.parse(localStorage.getItem(sequenceKey)) || [];
    const sequenceIndex = currentSequences.findIndex(sequence => sequence.id === id);
    
    if (sequenceIndex !== -1) {
      currentSequences[sequenceIndex] = updatedSequence;
      localStorage.setItem(sequenceKey, JSON.stringify(currentSequences));
      console.log(`Sequence updated successfully.${JSON.stringify(currentSequences[sequenceIndex])}`);
    } else {
      console.log("Sequence not found.");
    }
  } catch (error) {
    console.error("Error updating sequence:", error.message, error.stack);
  }
};
