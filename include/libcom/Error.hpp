#ifndef __H_LIBCOM_ERROR_HPP__
#define __H_LIBCOM_ERROR_HPP__

#include <stdexcept>

namespace libcom {
    class ParseError : public std::runtime_error {
    public:
        inline explicit ParseError(const std::string& s) : runtime_error(s) {}
    };
    
    class IOError : public std::runtime_error {
    public:
        inline explicit IOError(const std::string& s) : runtime_error(s) {}
    };

    class Timeout : public std::exception {
    public:
        inline explicit Timeout() : exception() {}

        inline const char* what() const noexcept override { return "timeout"; }
    };
}

#endif
