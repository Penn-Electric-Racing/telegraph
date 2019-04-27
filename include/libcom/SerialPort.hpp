#ifndef __H_LIBCOM_PORT_HPP__
#define __H_LIBCOM_PORT_HPP__

#include <string>
#include <iostream>

namespace libcom {
    typedef int serialhandle_t;
    class SerialBuf : public std::streambuf {
    public:
        SerialBuf(serialhandle_t handle, size_t rb, size_t put_back, size_t wb);
        ~SerialBuf();

        int underflow() override;
        int overflow(int c) override;
        int sync() override;
    private:
        serialhandle_t _handle;
        size_t _put_back;
        // TODO: replace with fixed-size allocated buffers
        // read buffer
        char* _rb;
        char* _wb;
        size_t _rb_size;
        size_t _wb_size;
    };
    class SerialPort : public std::iostream {
    public:
        SerialPort(const std::string& name, int baud, long timeout=500);
        ~SerialPort();
    private:
        // underlying buffer
        serialhandle_t _handle; // file handle
        SerialBuf _buf;
    };
}

#endif
