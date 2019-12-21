#include <telegraph/generator.hpp>
#include <telegraph/config.hpp>
#include <telegraph/id_map.hpp>

#include <telegraph/nodes/tree.hpp>
#include <telegraph/utils/hocon.hpp>

#include <iostream>
#include <vector>
#include <filesystem>

using namespace telegraph;

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "Must pass config file, config name, output file";
        return 1;
    }

    std::filesystem::path config_path = argv[1];
    std::string config_name = argv[2];
    std::filesystem::path output_path = argv[3];

    hocon_parser parser;
    json j = parser.parse_file(config_path);
    
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
    ids.add_all(t);

    // now create the generator
    generator g;
    g.set_tree(output_path.filename(), t);
    g.add_config(output_path.filename(), conf);
    g.set_output_dir(output_path.parent_path());
    g.run(ids);

    // delete the tree, will delete any children
    for (config* c : configs) delete c;
    delete t;
}

