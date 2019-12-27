import { app } from 'electron';
import path from 'path';
import { exec } from 'child_process';
import * as fs from'fs';


const resourcesPath = app.isPackaged ?
  path.dirname(app.getAppPath()) :
  path.resolve('resources');

export default () => {
  exec("./server", {
    cwd: resourcesPath,
    stdio: ["pipe", process.stdout, process.stderr]
  }, () => {
    console.log("Server exited");
  });
}
