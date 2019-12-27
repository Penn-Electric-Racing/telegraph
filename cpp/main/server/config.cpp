#include "config.hpp"

#include <iostream>

using namespace telegraph;

/**
 * Normalize paths between running inside bazel and a standalone executable
 */
static const std::string normalize_path(const std::string& path) {
    char* workspace_dir = std::getenv("BUILD_WORKSPACE_DIRECTORY");

    if (workspace_dir) {
        // If we are running inside Bazel, set all paths relative to
        // the bazel workspace directory
        return std::string(workspace_dir) + "/" + path;
    } else {
        // If we are running normally, just return the path
        return path;
    }
}

/**
 * Read the tree from the config file at the given path (normalized if
 * we are running inside bazel)
 */
tree* read_config_file(const std::string& path) {
    hocon_parser parser;
    json j = parser.parse_file(normalize_path(path));
    return tree::unpack(j["root"]);
}

// TODO: handle wiring everything up with UART
