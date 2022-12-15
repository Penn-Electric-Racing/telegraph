// import { combineReducers } from "redux";

const rootReducer = (state, action) => {
  if (typeof state === "undefined") {
    state = {
      mode: "STARTUP",
    }; // If state is undefined, initialize it with a default value
  }

  switch (action.type) {
    case "CONNECT":
      return {
        mode: "CONNECTED",
        variables: action.variables,
        layout: action.layout,
        values: action.variables,
      };
    case "UPDATE":
      return {
        ...state,
        values: action.values,
      };
    default:
      break;
  }

  return state;
};

// const rootReducer = combineReducers({
//   layout: layoutReducer,
// });

export default rootReducer;
