#ifndef __TELEGRAPH_ERRORS_HPP__
#define __TELEGRAPH_ERRORS_HPP__

#include <stdexcept>

namespace telegraph {

    class bad_type_error: public std::logic_error {
    public:
        inline bad_type_error(const std::string& error) : std::logic_error(error) {}
    };

    class missing_error: public std::logic_error {
    public:
        inline missing_error(const std::string& error) : std::logic_error(error) {}
    };

    class query_error: public std::logic_error {
    public:
        inline query_error(const std::string& error) : std::logic_error(error) {}
    };


    class generate_error : public std::logic_error {
    public:
        inline generate_error(const std::string& error) : std::logic_error(error) {}
    };

    class parse_error : public std::logic_error {
    public:
        inline parse_error(const std::string& error) : std::logic_error(error) {}
    };

    class remote_error : public std::logic_error {
    public:
        inline remote_error(const std::string& error) : std::logic_error(error) {}
    };

    class tree_error : public std::logic_error {
    public:
        inline tree_error(const std::string& error) : std::logic_error(error) {}
    };

    class io_error: public std::logic_error {
    public:
        inline io_error(const std::string& error) : std::logic_error(error) {}
    };
}

#endif
