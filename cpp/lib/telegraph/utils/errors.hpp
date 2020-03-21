#ifndef __TELEGRAPH_ERRORS_HPP__
#define __TELEGRAPH_ERRORS_HPP__

#include <exception>
#include <string_view>

namespace telegraph {

    class error : public std::exception {
    public:
        error(const std::string_view& m) : msg(m) {}

        const char* what() const noexcept override {
            return msg.c_str();
        }
    private:
        const std::string msg;
    };

    class bad_type_error: public error {
    public:
        inline bad_type_error(const std::string_view& m) 
            : error(m) {}
    };

    class missing_error: public error {
    public:
        inline missing_error(const std::string_view& m) : 
            error(m) {}
    };


    class generate_error : public error {
    public:
        inline generate_error(const std::string_view& m) : 
            error(m) {}
    };

    class parse_error : public error {
    public:
        inline parse_error(const std::string_view& m)
            : error(m) {}
    };

    class tree_error : public error {
    public:
        inline tree_error(const std::string_view& m) 
            : error(m) {}
    };

    class io_error : public error {
    public:
        inline io_error(const std::string_view& m) 
            : error(m) {}
    };

    class remote_error: public error {
    public:
        inline remote_error(const std::string_view& m) 
            : error(m) {}
    };
}

#endif
