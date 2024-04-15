import React,{ useState,useEffect} from 'react';

const Slider = ({ label, min, max, onChange,externalValue,color}) => {
    const [value, setValue] = useState((min + max) / 2 );
    const percentage = ((value - min) / (max - min)) * 100;
  
    const handleChange = (e) => {
      const newValue = e.target.value;
      setValue(newValue);
      onChange(newValue);
    };
  
    useEffect(() =>{
      setValue(externalValue)
    },[externalValue])
  
    // Adjust the padding and the height of the slider container if needed
    return (
      <div style={{ backgroundColor: color }} className="flex flex-col items-center justify-center p-4 border-4 border-black rounded-2xl h-24 w-full">
        <label className="text-white mb-2">{label}: {value} ({percentage.toFixed(0)}%)</label>
        <input
          type="range"
          min={min}
          max={max}
          value={value}
          
          className="slider slider-thumb w-full h-2 bg-blue-400 rounded-full cursor-pointer"
          onChange={handleChange}
        />
      </div>
    );
  };

export default Slider