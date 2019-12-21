#ifndef __TELEGRAPH_UTILS_SERIAL_PORT_HPP__
#define __TELEGRAPH_UTILS_SERIAL_PORT_HPP__

#include <string>
#include <iostream>

namespace telegraph {

    class serial_buf : public std::streambuf {
    public:
        serial_buf(int fd, long timeout, 
                size_t rb, size_t putback, size_t wb);
        ~serial_buf();

        int underflow() override;
        int overflow(int c) override;
        int sync() override;
    private:
        size_t put_back_;
        size_t rb_size_;
        char* rb_;

        size_t wb_size_;
        char* wb_;

        int fd_;
        long timeout_;
    };


    class serial_port : public std::iostream {
    public:
        serial_port(const std::string& name, int baud, long timeout=-1);
        ~serial_port();
    private:
        int fd_;
        serial_buf buf_;
    };
}

#endif
