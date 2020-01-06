#include <telegraph/gen/generator.hpp>
#include <telegraph/gen/config.hpp>
#include <telegraph/utils/hocon.hpp>

#include <iostream>
#include <vector>
#include <filesystem>
#include <fstream>

using namespace telegraph;

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "Must pass config file, config name, output file";
        return 1;
    }

    std::filesystem::path config_path = argv[1];
    std::string config_name = argv[2];

    std::filesystem::path output_path = argv[3];
    std::string output_name = output_path.filename();
    std::string output_dir = output_path.parent_path();

    hocon_parser parser;
    json j = parser.parse_file(config_path);

    // contains the tree
    config c(j);
    profile& p = c.get_profile(config_name);
    const node* t = c.get_tree();

    // now create the generator
    generator g;
    g.set_namespace(output_name, "per");
    g.set_tree(output_name, t);
    g.add_profile(output_name, &p);

    std::vector<generator::result> results = g.generate();
    for (const generator::result& r : results) {
        std::filesystem::path p = std::filesystem::path(output_dir) / r.filename;
        std::ofstream out(p);
        out << r.code << std::flush;
        out.close();
    }
}
