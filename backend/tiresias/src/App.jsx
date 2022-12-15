import { useState } from "react";
import { useSelector } from "react-redux";
import reactLogo from "./assets/react.svg";
import { invoke } from "@tauri-apps/api/tauri";
import "./App.css";
import StartupScreen from "./StartupScreen.jsx";
import Dashboard from "./Dashboard.jsx";

function App() {
  // const [greetMsg, setGreetMsg] = useState({});
  // const [name, setName] = useState("");

  const mode = useSelector((state) => state.mode);

  // async function greet() {
  //   let greet_message = await invoke("get_values", {});
  //   console.log(greet_message);
  //   setGreetMsg(greet_message);
  // }

  switch (mode) {
    case "STARTUP":
      return <StartupScreen />;
    case "CONNECTED":
      return <Dashboard />;
    default:
      return <p>Unknown state {mode} :(</p>;
  }

  // return (
  //   <div className="container">
  //     <h1>Welcome to Tauri!</h1>

  //     <div className="row">
  //       <a href="https://vitejs.dev" target="_blank">
  //         <img src="/vite.svg" className="logo vite" alt="Vite logo" />
  //       </a>
  //       <a href="https://tauri.app" target="_blank">
  //         <img src="/tauri.svg" className="logo tauri" alt="Tauri logo" />
  //       </a>
  //       <a href="https://reactjs.org" target="_blank">
  //         <img src={reactLogo} className="logo react" alt="React logo" />
  //       </a>
  //     </div>

  //     <p>Click on the Tauri, Vite, and React logos to learn more.</p>

  //     <div className="row">
  //       <div>
  //         <input
  //           id="greet-input"
  //           onChange={(e) => setName(e.currentTarget.value)}
  //           placeholder="Enter a name..."
  //         />
  //         <button type="button" onClick={() => greet()}>
  //           Greet
  //         </button>
  //       </div>
  //     </div>

  //     <div>
  //       {Object.entries(greetMsg).map(([k, v]) => (
  //         <p>
  //           {v.access_string}: {v.value && Object.entries(v.value)[0][1]}
  //         </p>
  //       ))}
  //     </div>
  //   </div>
  // );
}

export default App;
