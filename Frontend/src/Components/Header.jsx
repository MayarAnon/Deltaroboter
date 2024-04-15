import React,{ useState} from 'react';
import ConfirmationModal from './ConfirmationModal';



// Header-Komponente
const Header = (props) => {
    // State-Hook, um den Anzeigezustand des Menüs zu verwalten
    const [isMenuHidden, setMenuHidden] = React.useState(true);
    const [darkMode, setDarkMode] = React.useState(0); // Zustand für Dark Mode
  
    const ManualMode = () => {
      props.callback(1)
    }
    const PickPlaceMode = () => {
      props.callback(2)
    }
    const SettingsMode = () => {
      props.callback(3)
    }
    const GCode = () =>{
      props.callback(4)
    }


    // Event-Handler-Funktion, um den Menüzustand zu ändern
    const toggleMenu = () => {
      setMenuHidden(!isMenuHidden);
    };
  
    const toggleDarkMode = () => {
      if(darkMode===10){
        setDarkMode(0)
        document.body.classList.add('dark-mode2');
        console.log("https://www.youtube.com/watch?v=dQw4w9WgXcQ")
      }else if(darkMode <10){
        document.body.classList.remove('dark-mode2');
        setDarkMode(darkMode +1)
        document.body.classList.toggle('dark-mode'); 
        
    };
      
    };
    
    
    const [icon,setIcon] = useState(0)
    const toggle = () =>{
      if(icon >= 3){
        setIcon(0)
      }else if(icon<3){
        setIcon(icon + 1);
      }
      
    }
  
    const [isModalOpen, setIsModalOpen] = useState(false);
    
  
    return (
    <>
      <div style={{ backgroundColor: props.color }} className={`p-4 text-white rounded-xl font-bold  mt-10 mx-5 flex items-center justify-between border-4 border-black`}>
        <div className="text-3xl sm:text-l flex item-center">Deltaroboter
        <button
              className=" inline-block ml-4 "
              onClick={toggle}
            >
              {icon === 3?
               <img src="js.gif" className=" rounded-3xl "/>:
               <img src="Delta.png" className="hidden smm:block object-contain object-center w-10 h-10 "/>}
            </button>
            
        </div>
        
        <button className="md:hidden" id="burgerheader" onClick={toggleMenu}>
          <img src="Burgermenu.png" className="object-contain object-center w-10 h-10"></img>
        </button>
        <div className={`hidden md:flex md:space-x-2`} id="menu">
        <button onClick={GCode} className="px-4 py-2 ml-2 border-2 border-white rounded hover:bg-black">
          <img src="GCode.png" className=" object-contain object-center w-10 h-10"></img>
        </button>
          <button onClick={ManualMode} className="px-4 py-2 border-2 border-white rounded hover:bg-black">
          <img src="JoystickIcon.png" className="object-contain object-center w-10 h-10"></img>
          </button>
          <button onClick={PickPlaceMode} className="px-4 py-2 border-2 border-white rounded hover:bg-black">
          <img src="Addpositionicon.png" className="object-contain object-center w-10 h-10"></img>
            </button>
          <button onClick={SettingsMode} className="px-4 py-2 border-2 border-white rounded hover:bg-black">
          <img src="Settingsicon.png" className=" object-contain object-center w-10 h-10"></img>
          </button>
          <button onClick={toggleDarkMode} className="px-4 py-2 border-2 border-white rounded hover:bg-black"> 
            <img src="Darkmodeicon.png" className="object-contain object-center w-10 h-10"></img>
          </button>
        </div>
      </div>
      {isMenuHidden === true && (
      <div style={{ backgroundColor: props.color }} className="md:hidden mx-5 p-4 border-4 border-black rounded-2xl flex items-center justify-between">
            <div className="flex justify-between items-center w-full">
              <button onClick={GCode} className="px-2 smm:px-4 py-2 ml-2 border-2 border-white rounded hover:bg-black">
                <img src="GCode.png" className=" object-contain object-center w-10 h-10"></img>
              </button>
              
              <button onClick={ManualMode} className="px-2 smm:px-4 py-2 border-2 border-white rounded hover:bg-black">
                <img src="JoystickIcon.png" className="object-contain object-center w-10 h-10"></img>
              </button>
              <button onClick={PickPlaceMode} className="px-2 smm:px-4 py-2 border-2 border-white rounded hover:bg-black">
                <img src="Addpositionicon.png" className="object-contain object-center w-10 h-10"></img>
              </button>
              <button onClick={SettingsMode} className="px-2 smm:px-4 py-2 border-2 border-white rounded hover:bg-black">
                <img src="Settingsicon.png" className=" object-contain object-center w-10 h-10"></img>
              </button>
              <button onClick={toggleDarkMode} className="px-2 smm:px-4 py-2 border-2 border-white rounded hover:bg-black"> 
                <img src="Darkmodeicon.png" className="object-contain object-center w-10 h-10"></img>
              </button>
            </div>
      </div>)}
    </>
    );
  };

  export default Header;
  