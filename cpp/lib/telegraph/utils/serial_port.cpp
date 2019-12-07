#include "serial_port.hpp"

#include "../errors.hpp"

#include <cstdint>

extern "C" {
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
}
#include <cstring>

namespace telegraph {
    static int port_open(const char* file, int baud);
    static int port_close(int fd);

    static ssize_t port_write(int fd, const void* buf, size_t count);
    static ssize_t port_read(int fd, long timeout, void* buf, size_t count);

    serial_buf::serial_buf(int fd, long timeout, 
            size_t rb, size_t putback, size_t wb): 
                put_back_(putback), 
                rb_size_(std::max(rb, put_back_) + put_back_), rb_(nullptr),
                wb_size_(wb + 1), wb_(nullptr), fd_(fd), timeout_(timeout) {
        rb_ = new char[rb_size_];
        wb_ = new char[wb_size_];

        char* end = rb_ + rb_size_;
        // set the get buffer
        setg(end, end, end);
        // set the put buffer
        char* base = wb_;
        setp(base, base + wb_size_ - 1);
    }

    serial_buf::~serial_buf() {
        delete[] rb_;
        delete[] wb_;
    }

    int
    serial_buf::underflow() {
        if (gptr() < egptr()) return traits_type::to_int_type(*gptr());

        char* base = rb_;
        char* start = base;

        if (eback() == base) {
            std::memmove(base, egptr() - put_back_, put_back_);
            start += put_back_;
        }

        ssize_t n = port_read(fd_, timeout_, start, rb_size_ - (start - base));
        if (n == 0) {
            return traits_type::eof();
        } else if (n == -1) {
            throw timeout();
        } else if (n < 0) {
            throw io_error("failed to read from port");
        }

        setg(base, start, start + n);
        return traits_type::to_int_type(*gptr());
    }

    int
    serial_buf::overflow(int val) {
        if (val == traits_type::eof()) {
            return traits_type::eof();
        }
        if (!std::less_equal<char *>()(pptr(), epptr())) {
            throw "Buffer screwed up!";
        }
        char c = traits_type::to_char_type(val);
        *pptr() = c;
        pbump(1);
        if (!sync()) return val;
        else return traits_type::eof();
    }

    int
    serial_buf::sync() {
        size_t n = pptr() - pbase();
        pbump(-n);
        return port_write(fd_, pbase(), n) ? 0 : -1;
    }


    serial_port::serial_port(const std::string& name, int baud, long timeout) :
                                std::iostream(&buf_),
                                fd_(port_open(name.c_str(), baud)),
                                buf_(fd_, timeout, 1024, 16, 1024) {
                                
        // enable exceptions by default
        exceptions(serial_port::badbit | serial_port::failbit);
    }

    serial_port::~serial_port() {
        port_close(fd_);
    }


    static int port_open(const char* file, int baud) {
        int fd = open(file, O_RDWR | O_NOCTTY);
        struct termios tty;
        struct termios tty_old;
        std::memset(&tty, 0, sizeof tty);

        /* Error Handling */
        if (tcgetattr(fd, &tty ) != 0) {
            throw "Could not get port attributes";
        }

        /* Save old tty parameters */
        tty_old = tty;

        /* Set Baud Rate */
        speed_t speed = 0;
        switch (baud) {
            case 600:  speed = (speed_t) B600;  break;
            case 2400: speed = (speed_t) B2400; break;
            case 4800: speed = (speed_t) B4800; break;
            case 9600: speed = (speed_t) B9600; break;
            case 19200: speed = (speed_t) B19200; break;
            case 38400: speed = (speed_t) B38400; break;
            case 57600: speed = (speed_t) B57600; break;
            case 115200: speed = (speed_t) B115200; break;
            case 230400: speed = (speed_t) B230400; break;
            case 460800: speed = (speed_t) B460800; break;
            case 921600: speed = (speed_t) B921600; break;
            default: throw "Unknown baud rate";
        }
        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);

        /* Setting other Port Stuff */
        tty.c_cflag     &=  ~PARENB;            // Make 8n1
        tty.c_cflag     &=  ~CSTOPB;
        tty.c_cflag     &=  ~CSIZE;
        tty.c_cflag     |=  CS8;

        tty.c_cflag     &=  ~CRTSCTS;           // no flow control
        tty.c_lflag     &=  ~ICANON;            // set non-canonical mode
        tty.c_cc[VMIN]   =   1;                 // read doesn't block
        tty.c_cc[VTIME]  =   1;                 // 0.1 seconds read timeout
        tty.c_cflag     |=  CREAD | CLOCAL;     // turn on READ & ignore ctrl lines

        /* Make raw */
        cfmakeraw(&tty);

        /* Flush Port, then applies attributes */
        tcflush(fd, TCIFLUSH );
        if ( tcsetattr(fd, TCSANOW, &tty ) != 0) {
            throw "Error setting port attributes";
        }

        return fd;
    }

    static int port_close(int fd) {
        return close(fd);
    }

    static ssize_t port_write(int fd, const void* buf, size_t count) {
        size_t n = write(fd, buf, count);
        tcflush(fd, TCIFLUSH);
        return n;
    }

    static ssize_t port_read(int fd, long timeout, void* buf, size_t count) {
        if (timeout > 0) {
            fd_set set;
            FD_ZERO(&set);
            FD_SET(fd, &set);

            timeval timeout_val;
            timeout_val.tv_sec = timeout / 1000;
            timeout_val.tv_usec = (timeout % 1000) * 1000;
            int rv = select(fd + 1, &set, nullptr, nullptr, &timeout_val);
            if (rv < 0) return -2;
            else if (rv == 0) return -1;
            else return read(fd, buf, count);
        } else {
            return read(fd, buf, count);
        }
    }
}
