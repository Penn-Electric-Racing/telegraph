const { execSync } = require("child_process");
const { copyFileSync, mkdirSync, existsSync, chmodSync } = require("fs");

const args = process.argv.slice(2);

let build = args.length > 0 && args[0] == "--rebuild";

if (!existsSync("resources")) {
  mkdirSync("resources");
  build = true;
}

build = false;
if (build) {
  execSync("bazel build server", {
    cwd: "../cpp/",
    stdio: ["pipe", process.stdout, process.stderr]
  });

  copyFileSync("../bazel-bin/cpp/server", "resources/server");

  // Fix the permissions on the binary
  chmodSync("resources/server", 0o777);
}
