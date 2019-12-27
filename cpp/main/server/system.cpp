#include "system.hpp"

using namespace telegraph;

/**
 * Creates a special "system" tree that can be used by the clients to
 * control things about the server.
 */
tree* get_system_tree() {
    group* root_group = new group("root", "root", "controls the server", "none", 1);
    tree* system_tree = new tree(root_group);

    return system_tree;
}

// TODO: implement all of the nodes that we want to control the server
