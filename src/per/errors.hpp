#ifndef __PER_ERRORS_HPP__
#define __PER_ERRORS_HPP__

#include <stdexcept>

namespace per {

    class bad_type_error: public std::logic_error {
    public:
        inline bad_type_error(const std::string& error) : std::logic_error(error) {}
    };

    class missing_error: public std::logic_error {
    public:
        inline missing_error(const std::string& error) : std::logic_error(error) {}
    };

    class parse_error : public std::logic_error {
    public:
        inline parse_error(const std::string& error) : std::logic_error(error) {}
    };
}

#endif
