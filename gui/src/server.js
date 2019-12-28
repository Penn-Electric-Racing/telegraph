

export default () => {
  exec("./server", {
    cwd: resourcesPath,
    stdio: ["pipe", process.stdout, process.stderr]
  }, () => {
    console.log("Server exited");
  });
}
