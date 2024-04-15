import React,{ useState,useEffect,useRef} from 'react';
import Slider from './Slider'

const ManuellMode = (props) => {



    const [xValue,setXValue] = useState(0)
    const [yValue,setYValue] = useState(0)
    const [zValue,setZValue] = useState(0)
    const [phiValue,setPhiValue] = useState(0)
    const [actuator,setActuator] = useState('')
  
    useEffect(() =>{
      props.onCoordinateChange({xPosition: xValue,yPosition: yValue,zPosition: zValue,phi:phiValue,actuator:actuator,ton:0,toff:0,Id:0})
    },[props.menuState])
  
    const countIntervalRef = useRef(null);
  
    const handleMouseDown = (axis, direction) => {
      // Stellen Sie sicher, dass keine Intervalle überschneiden
      if (countIntervalRef.current) {
        clearInterval(countIntervalRef.current);
      }
      
      let setter;
      switch (axis) {
        case 'x':
          setter = setXValue;
          break;
        case 'y':
          setter = setYValue;
          break;
        case 'z':
          setter = setZValue;
          break;
        case 'phi':
          setter = setPhiValue;
          break;
        default:
          // Handhaben Sie ungültige Achsen
          return;
      }
  
      countIntervalRef.current = setInterval(() => {
        // Diese Funktion wird abhängig von der Geschwindigkeit aufgerufen und ändert den Wert der Variable
        setter((prevValue) => prevValue + direction);
      }, Math.max(0, 100 - props.speed)); // Die Geschwindigkeit beeinflusst das Intervall und darf nicht negativ sein
    };
  
      const handleMouseUpOrLeave = () => {
        // Intervall stoppen, wenn der Knopf losgelassen wird oder der Cursor den Knopf verlässt
        clearInterval(countIntervalRef.current);
      };
  
      
    return (
      <>
        <div className="flex flex-wrap justify-center mx-5 mt-10 sm:mt-32 gap-4">

          {props.gripper === '3option' &&(
            <div className=" w-2/3  flex-col items-center justify-center gap-4 hidden sm:block"> 
              <Slider color={props.color} label="parallel gripper" min={0} max={100} externalValue={parseInt(actuator, 10)} onChange={(value) =>setActuator(value.toString())} />
            </div>
          )}
  
          {props.modi === "sliders" && props.koordinateSystem === 'kartesisch' && (
            // Adjust the width of this container if necessary, or use w-full to take the full width
            <div className=" w-full sm:w-2/3 flex flex-col items-center justify-center gap-4 "> {/* Adjust this class to control the width */}
              <Slider color={props.color} label="+X / -X" min={-props.workSpaceRadius*0.70} max={props.workSpaceRadius*0.70} externalValue={xValue} onChange={(value) =>setXValue(value)} />
              <Slider color={props.color} label="+Y / -Y" min={-props.workSpaceRadius*0.70} max={props.workSpaceRadius*0.70} externalValue={yValue} onChange={(value) => setYValue(value)} />
              <Slider color={props.color} label="+Z / -Z" min={-props.workSpaceHeight/2} max={props.workSpaceHeight/2} externalValue={zValue} onChange={(value) => setZValue(value)} />
              <Slider color={props.color} label="+Phi / -Phi" min={-180} max={180} externalValue={phiValue}  onChange={(value) =>setPhiValue(value) } />
            </div>
          )}
  
          {props.modi === "sliders" && props.koordinateSystem === 'zylinder' && (
            // Adjust the width of this container if necessary, or use w-full to take the full width
            <div className=" w-full sm:w-2/3 flex flex-col items-center justify-center gap-4 "> 
              <Slider color={props.color} label="+phi / -phi" min={-180} max={180} externalValue={xValue} onChange={(value) =>setXValue(value)} />
              <Slider color={props.color} label="+r / -r" min={-props.workSpaceRadius} max={props.workSpaceRadius} externalValue={yValue} onChange={(value) => setYValue(value)} />
              <Slider color={props.color} label="+Z / -Z" min={-props.workSpaceHeight/2} max={props.workSpaceHeight/2} externalValue={zValue} onChange={(value) => setZValue(value)} />
              <Slider color={props.color} label="+theta / - theta" min={-180} max={180} externalValue={phiValue}  onChange={(value) =>setPhiValue(value) } />
            </div>
          )}
  
  
        {props.modi === "buttons" && (
          <div className="flex flex-col items-center justify-center p-2 sm:mr-0 border-4 border-black rounded-2xl">
            {/* Oben */}
            <button style={{ backgroundColor: props.color }} className=" text-white w-20 h-20 sm:w-32 sm:h-32 rounded-xl sm:mb-2 text-3xl flex items-center justify-center hover:bg-black border-4 border-blackborder-4 border-black"
               onMouseDown={() => handleMouseDown('y', 1)} onMouseUp={handleMouseUpOrLeave} onMouseLeave={handleMouseUpOrLeave}
               onTouchStart={() => handleMouseDown('y', 1)}  onTouchEnd={handleMouseUpOrLeave}
            >
              {props.koordinateSystem === 'zylinder'?
              <>+ R</>:
              <>↑ + Y</>} 
            </button>
            {/* Links und Rechts */}
            <div className="flex">
              <button style={{ backgroundColor: props.color }} className=" mr-10 sm:mr-16 style={{ backgroundColor: props.color }} text-white w-20 h-20 sm:w-32 sm:h-32 rounded-xl sm:mb-2 text-3xl flex items-center justify-center hover:bg-black border-4 border-black"
                onMouseDown={() => handleMouseDown('x', -1)} onMouseUp={handleMouseUpOrLeave} onMouseLeave={handleMouseUpOrLeave}
                onTouchStart={() => handleMouseDown('x', -1)}  onTouchEnd={handleMouseUpOrLeave}
              >
              {props.koordinateSystem === 'zylinder'?
                <>Phi -</>:
                <>← - X</>} 
              </button>
              <button style={{ backgroundColor: props.color }} className="ml-10 sm:ml-16  text-white w-20 h-20 sm:w-32 sm:h-32 rounded-xl sm:mb-2 text-3xl flex items-center justify-center hover:bg-black border-4 border-black"
                onMouseDown={() => handleMouseDown('x', 1)} onMouseUp={handleMouseUpOrLeave} onMouseLeave={handleMouseUpOrLeave}
                onTouchStart={() => handleMouseDown('x', 1)}  onTouchEnd={handleMouseUpOrLeave}
              >
                {props.koordinateSystem === 'zylinder'?
                <>Phi +</>:
                <>→ + X</>}
              </button>
            </div>
            {/* Unten */}
            <button style={{ backgroundColor: props.color }} className=" text-white w-20 h-20 sm:w-32 sm:h-32 rounded-xl sm:mb-2 text-3xl flex items-center justify-center hover:bg-black border-4 border-black"
               onMouseDown={() => handleMouseDown('y', -1)} onMouseUp={handleMouseUpOrLeave} onMouseLeave={handleMouseUpOrLeave}
               onTouchStart={() => handleMouseDown('y', -1)}  onTouchEnd={handleMouseUpOrLeave}
            >
              {props.koordinateSystem === 'zylinder'?
              <>- R</>:
              <>↓ - Y</>} 
            </button>
          </div>
        )}
        {props.modi === "buttons" && (
          
          <div  className=" flex flex-col justify-center border-4 border-black rounded-2xl p-2">
            {/* +z Button */}
            <button style={{ backgroundColor: props.color }} className=" text-white w-20 h-20 sm:w-32 sm:h-32 rounded-xl mb-16 text-3xl flex items-center justify-center hover:bg-black border-4 border-black"
               onMouseDown={() => handleMouseDown('z', 1)} onMouseUp={handleMouseUpOrLeave} onMouseLeave={handleMouseUpOrLeave}
               onTouchStart={() => handleMouseDown('z', 1)}  onTouchEnd={handleMouseUpOrLeave}
            >
              ↑ +z
            </button>
            {/* -z Button */}
            <button style={{ backgroundColor: props.color }} className="  text-white  w-20 h-20 sm:w-32 sm:h-32 rounded-xl text-3xl flex items-center justify-center hover:bg-black border-4 border-black"
               onMouseDown={() => handleMouseDown('z', -1)} onMouseUp={handleMouseUpOrLeave} onMouseLeave={handleMouseUpOrLeave}
               onTouchStart={() => handleMouseDown('z', -1)}  onTouchEnd={handleMouseUpOrLeave}
            >
              ↓ -z
            </button>
          </div>
        )}
  
          {/* Neue Tasten für +phi und -phi */}
        {props.modi === "buttons" && (
          <div className="flex flex-row items-center justify-center border-4 border-black rounded-2xl p-2">
            {/* Phi + Button */}
            <button style={{ backgroundColor: props.color }} className=" text-white w-20 h-20 sm:w-32 sm:h-32  rounded-xl text-3xl mr-2 flex items-center justify-center hover:bg-black  border-4 border-black"
              onMouseDown={() => handleMouseDown('phi', 1)} onMouseUp={handleMouseUpOrLeave} onMouseLeave={handleMouseUpOrLeave}
              onTouchStart={() => handleMouseDown('phi', 1)}  onTouchEnd={handleMouseUpOrLeave}
            >
              {props.koordinateSystem === 'zylinder'?
              <>Theta +</>:
              <>Phi +</>} 
            </button>
            {/* Phi - Button */}
            <button style={{ backgroundColor: props.color }} className=" text-white w-20 h-20 sm:w-32 sm:h-32  rounded-xl text-3xl ml-2 flex items-center justify-center hover:bg-black border-4 border-black"
              onMouseDown={() => handleMouseDown('phi', -1)} onMouseUp={handleMouseUpOrLeave} onMouseLeave={handleMouseUpOrLeave}
              onTouchStart={() => handleMouseDown('phi', -1)}  onTouchEnd={handleMouseUpOrLeave}
            >
              {props.koordinateSystem === 'zylinder'?
              <>Theta -</>:
              <>Phi -</>} 
            </button>
          </div>
        )}
          {/* Neue Tasten für +z und -z */}
        {(props.gripper === '1option' || props.gripper === '2option') &&(
          <div className="flex flex-row items-center   sm:flex-col justify-center border-4 border-black rounded-2xl p-2">
           
            <button onClick={() =>setActuator('On')} style={{ backgroundColor: props.color }} className="   text-white w-20 h-20 sm:w-32 sm:h-32 rounded-xl mr-2 sm:mr-0 sm:mb-16 text-3xl flex items-center justify-center hover:bg-black border-4 border-black">
              On
            </button>
            
            <button onClick={() =>setActuator('Off')} style={{ backgroundColor: props.color }} className="  text-white  w-20 h-20 sm:w-32 sm:h-32 rounded-xl ml-2 sm:ml-0 text-3xl flex items-center justify-center hover:bg-black border-4 border-black">
              Off
            </button>
          </div>
        )}
        {props.gripper === '3option' &&(
          <div className=" w-full sm:w-2/3 flex flex-col items-center justify-center gap-4 sm:hidden"> 
            <Slider color={props.color}  label="parallel gripper" min={0} max={100} externalValue={parseInt(actuator, 10)} onChange={(value) =>setActuator(value.toString())} />
          </div>
        )}
        
          <div className="flex flex-col border-4 border-black rounded-2xl sm:p-4 ">
            {/* Container für die Anzeige der Werte */}
            <div style={{ backgroundColor: props.color }} className=" text-white w-64 h-16 rounded-xl mb-2 mt-5 mr-2 ml-2 smm:mr-20 smm:ml-20 sm:mr-0 sm:ml-0 text-xl flex items-center px-4 hover:bg-black border-4 border-black">
              {props.koordinateSystem === 'zylinder' ? <span>Phi Winkel:</span> : <span>X Position:</span>}
              <input
                type="number"
                value={xValue}
                onChange={(e) => setXValue(e.target.value)}
                style={{ backgroundColor: props.color }}// Markiert das Feld als schreibgeschützt
                className="text-white w-20  rounded ml-2" />
            </div>
            <div style={{ backgroundColor: props.color }} className=" text-white w-64 h-16 rounded-xl mb-2 text-xl mr-2 ml-2 smm:mr-20 smm:ml-20 sm:mr-0 sm:ml-0 flex items-center px-4 hover:bg-black border-4 border-black">
              {props.koordinateSystem === 'zylinder' ? <span>R Radius:</span> : <span>Y Position:</span>}
              <input
                type="number"
                value={yValue}
                onChange={(e) => setYValue(e.target.value)}
                style={{ backgroundColor: props.color }}// Markiert das Feld als schreibgeschützt
                className="text-white w-20  rounded ml-2" />
            </div>
            <div style={{ backgroundColor: props.color }} className=" text-white w-64 h-16 rounded-xl mb-2 text-xl mr-2 ml-2 smm:mr-20 smm:ml-20 sm:mr-0 sm:ml-0 flex items-center px-4 hover:bg-black border-4 border-black">
              {props.koordinateSystem === 'zylinder' ? <span>Z Höhe:</span> : <span>Z Position:</span>}
              <input
                type="number"
                value={zValue}
                onChange={(e) => setZValue(e.target.value)}
                style={{ backgroundColor: props.color }}// Markiert das Feld als schreibgeschützt
                className="text-white w-20  rounded ml-2" />
            </div>
            <div style={{ backgroundColor: props.color }} className=" text-white w-64 h-16 rounded-xl mb-2 text-xl mr-2 ml-2 smm:mr-20 smm:ml-20 sm:mr-0 sm:ml-0 flex items-center px-4 hover:bg-black border-4 border-black">
              {props.koordinateSystem === 'zylinder' ? <span>Theta:</span> : <span>Phi:</span>}
              <input
                type="number"
                value={phiValue}
                onChange={(e) => setPhiValue(e.target.value)}
                style={{ backgroundColor: props.color }}// Markiert das Feld als schreibgeschützt
                className="text-white w-20  rounded ml-2" />
            </div>
            <div style={{ backgroundColor: props.color }} className=" text-white w-64 h-16 rounded-xl mb-5 text-xl mr-2 ml-2 smm:mr-20 smm:ml-20 sm:mr-0 sm:ml-0 flex items-center px-4 hover:bg-black border-4 border-black">
              Actuator
              <input
                value={actuator}
                readOnly 
                style={{ backgroundColor: props.color }}// Markiert das Feld als schreibgeschützt
                className="text-white w-20  rounded ml-2" />
            </div>
          </div>

          
  
          
        </div>
      </>
    );
  };

  export default ManuellMode