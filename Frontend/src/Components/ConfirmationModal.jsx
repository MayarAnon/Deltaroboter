import React,{ useState} from 'react';


const ConfirmationModal = ({color, isOpen, onClose, onConfirm,text,requirePassword = false, correctPassword }) => {
    
    const [password, setPassword] = useState('');
    const [passwordError, setPasswordError] = useState(false);
    const handleConfirm = () => {
      if (requirePassword && password !== correctPassword) {
        setPasswordError(true);
        setPassword('');
        return;
      }
      onConfirm();
      setPassword('');
    };
    const handleClose = () => {
      onClose();
    };
    
    if (!isOpen) return null;
    return (
      <div style={{ zIndex: 1000 }} className="fixed inset-0 bg-gray-600 bg-opacity-50 overflow-y-auto h-full   w-full" id="my-modal">
        <div style={{ backgroundColor: color }}  className="relative top-20 mx-auto p-5 border-4 border-white w-96 shadow-lg rounded-2xl ">
          <div className="mt-3 text-center">
            <h3 className="text-lg leading-6 font-medium text-white">{text}</h3>
            {requirePassword && (
            <div className="mt-2 px-7 py-3">
              <input
                type="password"
                value={password}
                onChange={(e) => { setPassword(e.target.value); setPasswordError(false); }}
                className="text-black rounded px-2 py-1"
              />
              {passwordError && <p className="text-sm text-red-500">Falsches Passwort</p>}
            </div>
          )}
            <div className="mt-2 px-7 py-3">
              <p className="text-sm text-white">Möchten Sie fortfahren?</p>
            </div>
            <div className="items-center px-4 py-3">
              <button onClick={handleClose} style={{ backgroundColor: color }}   className="px-4 py-2 border-4 border-white  text-white rounded hover:bg-black focus:outline-none mr-2">Abbrechen</button>
              <button onClick={handleConfirm} style={{ backgroundColor: color }}  className="px-4 py-2 border-4 border-white text-white rounded hover:bg-black focus:outline-none">Bestätigen</button>
            </div>
          </div>
        </div>
      </div>
    );
  };

  export default ConfirmationModal;