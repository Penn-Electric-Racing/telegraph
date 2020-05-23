const { execSync } = require("child_process");
const { copyFileSync, mkdirSync, existsSync, chmodSync } = require("fs");

const args = process.argv.slice(2);

let build = args.length > 0 && args[0] == "--build-server";

if (!existsSync("resources")) {
  mkdirSync("resources");
}

if (build) {
  execSync("bazel build server", {
    cwd: "../cpp/",
    stdio: ["pipe", process.stdout, process.stderr]
  });

  if (existsSync("../bazel-bin/cpp/server.exe")) {
    copyFileSync("../bazel-bin/cpp/server.exe", "resources/server");
  } else if (existsSync("../bazel-bin/cpp/server")) {
    copyFileSync("../bazel-bin/cpp/server", "resources/server");
  }
  // Fix the permissions on the binary
  if (existsSync("resources/server")) {
    chmodSync("resources/server", 0o777);
  }
} else {
  if (existsSync("../bazel-bin/cpp/server.exe")) {
    copyFileSync("../bazel-bin/cpp/server.exe", "resources/server");
  } else if (existsSync("../bazel-bin/cpp/server")) {
    copyFileSync("../bazel-bin/cpp/server", "resources/server");
  }
  // Fix the permissions on the binary
  if (existsSync("resources/server")) {
    chmodSync("resources/server", 0o777);
  }
}
