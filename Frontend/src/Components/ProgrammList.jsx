import React, { useState, useEffect } from "react";
import axios from "axios";
import { HighlightedCode } from "./Texteditor";
import { useRecoilState,useRecoilValue } from "recoil";
import {settingAtom, gCodeStringAtom, gCodeModeAtom } from "../utils/atoms";
import ConfirmationModal from "./ConfirmationModal";

const LoadProgrammList = () => {
  const server = process.env.REACT_APP_API_URL;
  const [programmlist, SetProgrammList] = useState([]);
  const settings = useRecoilValue(settingAtom);
  const [reloadTrigger, setReloadTrigger] = useState(0);
  useEffect(() => {
    // Here we call the Express endpoint when the component mounts
    axios
      .get(`${server}/loadGCodeFiles`)
      .then((response) => {
        // Saving the received data into the component's state
        console.log(response.data);
        SetProgrammList(response.data);
      })
      .catch((error) => {
        console.error("Fehler beim Abrufen der Daten:", error);
      });
  }, [reloadTrigger]); // The empty array as the second argument ensures this effect runs only once on component mount
  const triggerReload = () => {
    setReloadTrigger((prev) => prev + 1); // Incrementing the trigger
  };
  return (
    <>
      {programmlist && programmlist.length > 0 ? (
        programmlist.map((programm) => (
          <LoadProgramm
            key={programm.fileName}
            color={settings.color}
            name={programm.fileName}
            content={programm.content}
            onDelete={triggerReload}
          />
        ))
      ) : (
        <>
          <div className="flex justify-center items-center p-10">
            <img
              src="loading.gif"
              className="object-contain object-center w-24 h-24"
              alt="Load Icon"
            />
          </div>
        </>
      )}
    </>
  );
};
// Component to load individual programs
const LoadProgramm = ({ color, name, content, onDelete }) => {
  const server = process.env.REACT_APP_API_URL;
  const [sharedString, setSharedString] = useRecoilState(gCodeStringAtom);

  const [GCodemode, setGCodemode] = useRecoilState(gCodeModeAtom);
  const [expand, SetExpand] = useState(false);
  const [isMenuHidden, setMenuHidden] = useState(false);
  // Function to toggle expand state
  const handleExpandChange = () => {
    SetExpand(!expand);
  };
  // Function to toggle menu visibility
  const toggleMenu = () => {
    setMenuHidden(!isMenuHidden);
  };
  // Function to load the program
  const handleLoadProgramm = () => {
    setSharedString({ name, content });
    setGCodemode(1);
  };
  // Function to delete the program
  const handleDelete = () => {
    const address = `${server}/deleteGCode?name=${name}`; // Hier name durch den tatsächlichen Dateinamen ersetzen

    axios
      .delete(address)
      .then((response) => {
        console.log("Datei erfolgreich gelöscht:", response.data);
        onDelete(); // Calling onDelete function to trigger reload
      })
      .catch((error) => {
        console.error("Fehler beim Löschen der Datei:", error);
      });
  };
  const handleRun = () => {
    const address = `${server}/pickandplace/program`;

    // Creating the data object to be sent
    const programData = {
      program: name, // Replace content with the actual G-Code here
    };

    // Sending a POST request using axios
    axios
      .post(address, programData, {
        headers: {
          "Content-Type": "application/json",
        },
      })
      .then((response) => {
        console.log("Programm erfolgreich übermittelt:", response.data);
      })
      .catch((error) => {
        console.error("Fehler beim Publizieren des Programms:", error);
      });
  };

  const [isModalOpenRun, setIsModalOpenRun] = useState(false);
  const [isModalOpenDelete, setIsModalOpenDelete] = useState(false);

  const RunhandleOpenModal = () => {
    setIsModalOpenRun(true);
  };

  const RunhandleCloseModal = () => {
    setIsModalOpenRun(false);
  };

  const RunhandleConfirm = () => {
    handleRun();
    setIsModalOpenRun(false);
  };
  const DeletehandleOpenModal = () => {
    setIsModalOpenDelete(true);
  };

  const DeletehandleCloseModal = () => {
    setIsModalOpenDelete(false);
  };

  const DeletehandleConfirm = () => {
    handleDelete();
    setIsModalOpenDelete(false);
  };

  return (
    <div>
      <>
        <ConfirmationModal
          color={color}
          isOpen={isModalOpenRun}
          onClose={RunhandleCloseModal}
          onConfirm={RunhandleConfirm}
          text={"Run Program"}
        />
        <ConfirmationModal
          color={color}
          isOpen={isModalOpenDelete}
          onClose={DeletehandleCloseModal}
          onConfirm={DeletehandleConfirm}
          text={"Delete Program"}
        />
        <div
          style={{ backgroundColor: color }}
          className="ml-5 mr-5 p-4 mt-5 border-4 border-black rounded-2xl flex items-center justify-between"
        >
          <div className="flex justify-between items-center w-full">
            <label className=" flex item-center text-xl text-white mb-2 mr-2 font-bold">
              {name.slice(0, -6)}
            </label>
            <div className="rounded p-2 flex-grow mr-5" />
            <button
              className="sm:hidden"
              id="burgerheader"
              onClick={toggleMenu}
            >
              <img
                src="Burgermenu.png"
                className="object-contain object-center w-10 h-10"
                alt="img"
              ></img>
            </button>
            <button
              className="hidden sm:block px-4 py-2 border-2 border-white text-white text-l font-bold rounded hover:bg-black"
              onClick={RunhandleOpenModal}
            >
              Run
            </button>
            <button
              className="hidden sm:block mx-2 px-4 py-2 border-2 border-white text-white text-l font-bold rounded hover:bg-black"
              onClick={handleLoadProgramm}
            >
              Edit
            </button>
            <button
              className="hidden sm:block mr-2 px-4 py-2 border-2 border-white text-white text-l font-bold rounded hover:bg-black"
              onClick={handleExpandChange}
            >
              {expand ? "Close" : "Expand"}
            </button>
            <button
              className="hidden sm:block mr-2 px-4 py-2 border-2 bg-red-600 hover:bg-red-700 text-white rounded"
              onClick={DeletehandleOpenModal}
            >
              Delete
            </button>
          </div>
        </div>
        {isMenuHidden === true && (
          <div
            style={{ backgroundColor: color }}
            className="sm:hidden mx-5 p-4 border-4 border-black rounded-2xl flex items-center justify-between"
          >
            <div className="flex justify-between items-center w-full">
              <button
                className=" px-3 py-2 border-2 border-white text-white text-l font-bold rounded hover:bg-black"
                onClick={RunhandleOpenModal}
              >
                Run
              </button>
              <button
                className=" px-3 py-2 border-2 border-white text-white text-l font-bold rounded hover:bg-black"
                onClick={handleLoadProgramm}
              >
                Edit
              </button>
              <button
                className=" px-3 py-2 border-2 border-white text-white text-l font-bold rounded hover:bg-black"
                onClick={handleExpandChange}
              >
                {expand ? "Close" : "Expand"}
              </button>
              <button
                className=" px-3 py-2 border-2 bg-red-600 hover:bg-red-700 text-white rounded"
                onClick={DeletehandleOpenModal}
              >
                Delete
              </button>
            </div>
          </div>
        )}
        {expand === true && (
          <div
            style={{ backgroundColor: color }}
            className="p-4 text-white rounded-xl mx-5 border-4 border-black overflow-auto"
          >
            <HighlightedCode text={content} />
          </div>
        )}
      </>
    </div>
  );
};

export default LoadProgrammList;
