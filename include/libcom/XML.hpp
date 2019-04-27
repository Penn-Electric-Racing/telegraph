#ifndef __H_LIBCOM_XML_HPP__
#define __H_LIBCOM_XML_HPP__

#include "Param.hpp"

#include <string>
#include <vector>
#include <memory>

namespace libcom {
    namespace xml {
        // 
        std::vector<std::shared_ptr<GenericParam>>
                readPerDosFile(const std::string& file);
    }
}

#endif
