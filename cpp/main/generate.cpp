#include <telegraph/generator.hpp>
#include <telegraph/config.hpp>
#include <telegraph/id_map.hpp>

#include <telegraph/nodes/tree.hpp>
#include <telegraph/utils/hocon.hpp>

#include <iostream>
#include <vector>

using namespace telegraph;

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "Must pass file, config name, and output file";
        return 1;
    }
    char* dir = std::getenv("BUILD_WORKSPACE_DIRECTORY");
    std::string file = (dir ? std::string(dir) : ".") + "/" + argv[1];
    std::string config_name = argv[2];
    std::string output_file = argv[3];

    hocon_parser parser;
    json j = parser.parse_file(file);
    
    tree* t = tree::unpack(j["root"]);

    std::vector<config*> configs = config::unpack(t, j["configs"]); 

    // find the config corresponding to the passed name
    config* conf = nullptr;
    for (config* c : configs) {
        if (c->get_name() == config_name) {
            conf = c;
            break;
        }
    }

    id_map ids;
    ids.add_leaves(t);

    // now create the generator
    generator g;
    g.set_tree(output_file, t);
    g.add_config(output_file, conf);
    g.set_output_dir(dir);
    g.run(ids);

    // delete the tree, will delete any children
    for (config* c : configs) delete c;
    delete t;
}

