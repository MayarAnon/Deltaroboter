import React, { useState } from "react";

// ConfirmationModal component: Renders a modal for confirmation with optional password input
const ConfirmationModal = ({
  color,
  isOpen,
  onClose,
  onConfirm,
  text,
  requirePassword = false,
  correctPassword,
}) => {
  const [password, setPassword] = useState("");
  const [passwordError, setPasswordError] = useState(false);
  const handleConfirm = () => {
    // If password is required and incorrect password is entered
    if (requirePassword && password !== correctPassword) {
      setPasswordError(true);
      setPassword("");
      return;
    }
    // Call onConfirm callback and reset password
    onConfirm();
    setPassword("");
  };
  // Function to handle modal close
  const handleClose = () => {
    onClose();
  };
  // If modal is not open, return null
  if (!isOpen) return null;
  return (
    <div
      style={{ zIndex: 1000 }}
      className="fixed inset-0 bg-gray-600 bg-opacity-50 overflow-y-auto h-full   w-full"
      id="my-modal"
    >
      <div
        style={{ backgroundColor: color }}
        className="relative top-20 mx-auto p-5 border-4 border-white w-96 shadow-lg rounded-2xl "
      >
        <div className="mt-3 text-center">
          <h3 className="text-lg leading-6 font-medium text-white">{text}</h3>
          {requirePassword && (
            <div className="mt-2 px-7 py-3">
              <input
                type="password"
                value={password}
                onChange={(e) => {
                  setPassword(e.target.value);
                  setPasswordError(false);
                }}
                className="text-black rounded px-2 py-1"
              />
              {passwordError && (
                <p className="text-sm text-red-500">Wrong Password</p>
              )}
            </div>
          )}
          <div className="mt-2 px-7 py-3">
            <p className="text-sm text-white">Do you want to continue?</p>
          </div>
          <div className="items-center px-4 py-3">
            <button
              onClick={handleClose}
              style={{ backgroundColor: color }}
              className="px-4 py-2 border-4 border-white  text-white rounded hover:bg-black focus:outline-none mr-2"
            >
              Abort
            </button>
            <button
              onClick={handleConfirm}
              style={{ backgroundColor: color }}
              className="px-4 py-2 border-4 border-white text-white rounded hover:bg-black focus:outline-none"
            >
              Continue
            </button>
          </div>
        </div>
      </div>
    </div>
  );
};

export default ConfirmationModal;
