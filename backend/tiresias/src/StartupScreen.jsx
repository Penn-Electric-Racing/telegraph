import { useState, useEffect } from "react";
import { useSelector, useDispatch } from "react-redux";
import { invoke } from "@tauri-apps/api/tauri";
import * as dialog from "@tauri-apps/api/dialog";

import Dropdown from "react-bootstrap/Dropdown";
import Alert from "react-bootstrap/Alert";
import Form from "react-bootstrap/Form";
import Button from "react-bootstrap/Button";
import { IoRefresh, IoFileTray } from "react-icons/io5";

function App() {
  const [serialDevices, setSerialDevices] = useState([]);
  const [selectedSerialDevice, setSelectedSerialDevice] = useState(null);
  const [assignedCanIds, setAssignedCanIds] = useState(null);
  const [layouts, setLayouts] = useState([]);
  const [selectedLayout, setSelectedLayout] = useState(null);
  const [displayParseError, setDisplayParseError] = useState(null);
  const [connectionError, setConnectionError] = useState(null);

  const dispatch = useDispatch();

  const refreshDevices = () => {
    invoke("list_devices", {}).then((o) => {
      setSerialDevices(o);

      if (o.indexOf(selectedSerialDevice) === -1) {
        setSelectedSerialDevice(null);
      }
    });
  };

  const selectCanIdsFile = async () => {
    setAssignedCanIds(await dialog.open());
  };

  const selectDisplayLayoutFile = async () => {
    const layoutFile = await dialog.open();
    if (layoutFile !== null) {
      try {
        const layouts = (
          await invoke("parse_display_file", {
            displayFile: layoutFile,
          })
        ).Device;

        console.log(layouts);

        if (layouts && layouts !== []) {
          setLayouts(layouts);
        }
        setDisplayParseError(null);
      } catch (e) {
        setLayouts([]);
        setDisplayParseError(e);
      }
      setSelectedLayout(null);
    }
  };

  const connectToDevice = async () => {
    try {
      let variables = await invoke("connect_to_device", {
        device: selectedSerialDevice,
        canIdsFile: assignedCanIds,
      });

      console.log(variables);
      dispatch({
        type: "CONNECT",
        layout: selectedLayout,
        variables: variables,
      });
    } catch (e) {
      setConnectionError(e);
    }
  };

  useEffect(refreshDevices, []);

  return (
    <>
      {displayParseError && (
        <Alert
          variant="danger"
          onClose={() => setDisplayParseError(null)}
          dismissible
        >
          {displayParseError}
        </Alert>
      )}
      {connectionError && (
        <Alert
          variant="danger"
          onClose={() => setConnectionError(null)}
          dismissible
        >
          {connectionError}
        </Alert>
      )}
      <p>Assigned CAN IDs file</p>
      <Button onClick={selectCanIdsFile}>
        <IoFileTray />
      </Button>
      <p>{assignedCanIds ?? "Select a CAN ID file"}</p>

      <p>Display layout file</p>
      <Button onClick={selectDisplayLayoutFile}>
        <IoFileTray />
      </Button>

      <Dropdown
        onSelect={(k, _) => {
          let layout = layouts.find((o) => o.Name == k);
          console.log(layout);
          setSelectedLayout(layout);
        }}
      >
        <Dropdown.Toggle
          variant={layouts.length === 0 ? "disabled" : "primary"}
          id="serial-dropdown"
          disabled={layouts.length === 0}
        >
          {selectedLayout?.Name ??
            (layouts.length === 0
              ? "Open a display layouts file"
              : "Select a layout")}
        </Dropdown.Toggle>

        <Dropdown.Menu>
          {layouts.map((layout) => (
            <Dropdown.Item key={layout.Name} eventKey={layout.Name}>
              {layout.Name}
            </Dropdown.Item>
          ))}
        </Dropdown.Menu>
      </Dropdown>

      <Dropdown
        onSelect={(k, _) => {
          setSelectedSerialDevice(k);
        }}
      >
        <Dropdown.Toggle
          variant={serialDevices.length === 0 ? "disabled" : "primary"}
          id="serial-dropdown"
          disabled={serialDevices.length === 0}
        >
          {selectedSerialDevice ??
            (serialDevices.length === 0
              ? "No serial devices detected"
              : "Select a serial device")}
        </Dropdown.Toggle>

        <Dropdown.Menu>
          {serialDevices.map((device) => (
            <Dropdown.Item key={device} eventKey={device}>
              {device}
            </Dropdown.Item>
          ))}
        </Dropdown.Menu>
      </Dropdown>
      <Button onClick={refreshDevices}>
        <IoRefresh />
      </Button>

      <Button
        onClick={connectToDevice}
        disabled={!selectedSerialDevice || !assignedCanIds || !selectedLayout}
      >
        Connect
      </Button>
    </>
  );
}

export default App;
