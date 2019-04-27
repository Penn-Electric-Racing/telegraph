#ifndef __H_LIBCOM_ERROR_HPP__
#define __H_LIBCOM_ERROR_HPP__

#include <stdexcept>

namespace libcom {
    class ParseError : public std::runtime_error {
    public:
        inline explicit ParseError(const std::string& s) : runtime_error(s) {}
    };
}

#endif
