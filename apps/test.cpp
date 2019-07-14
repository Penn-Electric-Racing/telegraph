#include <libcom/hocon.hpp>

#include <iostream>

using namespace per;

int main(int argc, char** argv) {
    hocon_parser parser;
    json j = parser.parse_file("../apps/example.conf");
    std::cout << j.dump(4) << std::endl;
}
