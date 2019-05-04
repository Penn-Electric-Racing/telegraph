#include "XML.hpp"

#include "Error.hpp"

#include <pugixml.hpp>
#include <iostream>
#include <algorithm>

namespace libcom::xml {
    std::vector<std::shared_ptr<GenericParam>>
        readPerDosFile(const std::string& file) {
            std::vector<std::shared_ptr<GenericParam>> params;

            pugi::xml_document doc;
            pugi::xml_parse_result result = doc.load_file(file.c_str());

            if (!result) throw IOError("File not found");
            
            pugi::xpath_node_set idNodes = doc.select_nodes("PerDos/PerDosId");
            for (pugi::xpath_node paramPath : idNodes) {
                pugi::xml_node paramNode = paramPath.node();
                GenericParam::Id id = (GenericParam::Id) paramNode.attribute("Id").as_int();
                std::string name = paramNode.attribute("AccessString").value();
                std::string type = paramNode.attribute("Type").value();

                // replace dots with slashes, [ with ( and ] with )
                std::replace(name.begin(), name.end(), '.', '/');
                std::replace(name.begin(), name.end(), '[', '/');
                name.erase(std::remove(name.begin(), name.end(), ']'), name.end());
                // prepend a slash
                name = "/" + name;

                std::shared_ptr<GenericParam> param;
                if (type == "bool") param = std::make_shared<Param<bool>>(id, name);
                else if (type == "uint8") param = std::make_shared<Param<uint8_t>>(id, name);
                else if (type == "uint16") param = std::make_shared<Param<uint16_t>>(id, name);
                else if (type == "uint32") param = std::make_shared<Param<uint32_t>>(id, name);
                else if (type == "uint64") param = std::make_shared<Param<uint64_t>>(id, name);
                else if (type == "int8") param = std::make_shared<Param<int8_t>>(id, name);
                else if (type == "int16") param = std::make_shared<Param<int16_t>>(id, name);
                else if (type == "int32") param = std::make_shared<Param<int16_t>>(id, name);
                else if (type == "int64") param = std::make_shared<Param<int16_t>>(id, name);
                else if (type == "float") param = std::make_shared<Param<float>>(id, name);
                else if (type == "double") param = std::make_shared<Param<double>>(id, name);
                else throw ParseError("Unknown perdos id type");

                params.push_back(param);
            }
            return params;
        }
}
