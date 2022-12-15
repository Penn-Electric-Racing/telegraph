import { useState, useEffect } from "react";
import { useSelector, useDispatch } from "react-redux";
import { invoke } from "@tauri-apps/api/tauri";

import Dropdown from "react-bootstrap/Dropdown";
import Form from "react-bootstrap/Form";
import Button from "react-bootstrap/Button";
import { IoRefresh, IoFileTray } from "react-icons/io5";

const renderValue = (v) => {
  // TODO: take into account some of the parameters, especiall for boolean types
  if (v.value === null) {
    return "null";
  }
  return `${Object.values(v.value)[0]}`;
};

// //create your forceUpdate hook
// function useForceUpdate() {
//   const [value, setValue] = useState(0); // integer state
//   return () => {
//     console.log("forcing update");
//     setValue((value) => value + 1);
//   }; // update state to force render
//   // An function that increment 👆🏻 the previous state like here
//   // is better than directly setting `value + 1`
// }

function Variable({ id, variable }) {
  // const forceUpdate = useForceUpdate();
  // useEffect(() => {
  //   const timerId = setInterval(forceUpdate, 100 /* ms */);
  //   return () => clearInterval(timerId);
  // });

  const value = useSelector(
    (state) => state.values[id]
    //(s1, s2) => s1.values[id] == s2.values[id]
  );

  return (
    <tr>
      <td>{variable.access_string}</td>
      <td>{renderValue(value)}</td>
    </tr>
  );
}

function Value({ value }) {
  const variableRegex = new RegExp(`${value.Regex}`);
  const variables = useSelector((state) =>
    Object.entries(state.variables).filter(([_, v]) =>
      variableRegex.test(v.access_string)
    )
  );

  return (
    <div>
      <table>
        <tbody>
          {variables.map(([k, v]) => (
            <Variable key={k} id={k} variable={v} />
          ))}
        </tbody>
      </table>
    </div>
  );
}

function Group({ group }) {
  return (
    <div>
      <details open={group.StartVisible ?? true}>
        <summary>{group.Name}</summary>
        {group.Value.map((value) => (
          <Value key={value.Name} value={value} />
        ))}
      </details>
    </div>
  );
}

function App() {
  // const [serialDevices, setSerialDevices] = useState([]);
  // const [selectedSerialDevice, setSelectedSerialDevice] = useState(null);
  // const [assignedCanIds, setAssignedCanIds] = useState(null);
  // const [displayLayout, setDisplayLayout] = useState(null);
  // const [parseError, setParseError] = useState(null);

  const dispatch = useDispatch();
  const layout = useSelector(
    (state) => state.layout,
    (s1, s2) => s1.layout == s2.layout
  );

  const refreshData = () => {
    invoke("get_values", {}).then((o) => {
      console.log("new data");
      dispatch({
        type: "UPDATE",
        values: o,
      });
    });
  };

  // const selectCanIdsFile = async () => {
  //   setAssignedCanIds(await dialog.open());
  // };

  // const selectDisplayLayoutFile = async () => {
  //   setDisplayLayout(await dialog.open());
  // };

  // const connectToDevice = () => {
  //   invoke("connect_to_device", {
  //     device: selectedSerialDevice,
  //     canIdsFile: assignedCanIds,
  //     displayLayout: displayLayout,
  //   }).then((o) => {
  //     dispatch({
  //       type: "CONNECT",
  //       ret: o,
  //     });
  //   });
  // };

  useEffect(() => {
    refreshData();
    const timerId = setInterval(refreshData, 100 /* ms */);
    return () => clearInterval(timerId);
  });

  // TODO: need to use this instead:
  // https://github.com/tauri-apps/tauri/issues/4091

  return (
    <>
      {layout.Group.map((group) => (
        <Group key={group.Name} group={group} />
      ))}
    </>
  );
}

export default App;
