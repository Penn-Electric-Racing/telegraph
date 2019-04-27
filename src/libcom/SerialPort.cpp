#include "SerialPort.hpp"

extern "C" {
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <termios.h>
}
#include <cstring>

// these change per platform

static int popen(const char* file, int baud, long timeout) {
    int handle = open(file, O_RDWR | O_NOCTTY);
    struct termios tty;
    struct termios tty_old;
    std::memset(&tty, 0, sizeof tty);

    /* Error Handling */
    if (tcgetattr ((int) handle, &tty ) != 0) {
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
    tty.c_cc[VMIN]   =   0;                 // read doesn't block
    tty.c_cc[VTIME]  =   0;                 // 0.1 seconds read timeout
    tty.c_cflag     |=  CREAD | CLOCAL;     // turn on READ & ignore ctrl lines

    /* Make raw */
    cfmakeraw(&tty);

    /* Flush Port, then applies attributes */
    tcflush(handle, TCIFLUSH );
    if ( tcsetattr(handle, TCSANOW, &tty ) != 0) {
        throw "Error setting port attributes";
    }
    return handle;
}

static int pclose(int fd) {
    return close(fd);
}

static ssize_t pwrite(int fd, const void* buf, size_t count) {
    size_t n = write(fd, buf, count);
    tcflush(fd, TCIFLUSH);
    return n;
}

static ssize_t pread(int fd, void* buf, size_t count) {
    return read(fd, buf, count);
}

// Underlying buffer for the port
namespace libcom {

    SerialBuf::SerialBuf(int handle, size_t rb, size_t put_back, size_t wb) :
            _handle(handle), _put_back(16), _rb(nullptr), _wb(nullptr), _rb_size(0), _wb_size(0) {
        _rb_size = std::max(rb, put_back) + put_back;
        _wb_size = wb + 1;
        _rb = new char[_rb_size];
        _wb = new char[_wb_size];
        char* end  = _rb + _rb_size;
        //set the get buffer
        setg(end, end, end);
        // set the put buffer
        char* base = _wb;
        setp(base, base + _wb_size - 1);
    }
    SerialBuf::~SerialBuf() {
        delete _rb;
        delete _wb;
    }

    // for reading when we have run out of data to read
    int
    SerialBuf::underflow() {
        if (gptr() < egptr())
            return traits_type::to_int_type(*gptr());

        char *base = _rb;
        char *start = base;
        // true when this isn't the first fill
        if (eback() == base) {
            std::memmove(base, egptr() - _put_back, _put_back);
            start += _put_back;
        }

        // start is now the start of the buffer, proper.
        // Read from fptr_ in to the provided buffer
        std::cout << "reading " << (size_t) start << " " << _rb_size - (start - base) << std::endl;
        size_t n = pread(_handle, start, _rb_size - (start - base));
        std::cout << "n: " << (int) n << std::endl;
        if (n == 0) return traits_type::eof();

        // Set buffer pointers
        setg(base, start, start + n);

        return traits_type::to_int_type(*gptr());
    }

    // for writing when we overflow the amount
    // being written
    int
    SerialBuf::overflow(int val) {
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
    SerialBuf::sync() {
        size_t n = pptr() - pbase();
        pbump(-n);
        return pwrite(_handle, pbase(), n) ? 0 : -1;
    }

    // port wrapper
    SerialPort::SerialPort(const std::string& name, int baud, long timeout) : 
                                    _handle(popen(name.c_str(), baud, timeout)),
                                    _buf(_handle, 1024, 16, 1024),
                                    std::iostream(&_buf) {}
    SerialPort::~SerialPort() {
        pclose(_handle);
    }
}
