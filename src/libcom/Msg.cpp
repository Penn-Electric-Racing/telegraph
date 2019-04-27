#include "Packet.hpp"
#include "Msg.hpp"
#include "Error.hpp"

#include <iostream>
#include <sstream>
#include <cmath>
#include <iomanip>


static uint32_t fletcher32(uint16_t const *data, size_t words) {
    //From https://en.wikipedia.org/wiki/Fletcher%27s_checksum
    uint32_t sum1 = 0xffff, sum2 = 0xffff;

    while (words) {
        size_t tlen = std::min((size_t) 359, words);
        words -= tlen;
        do {
            sum2 += sum1 += *data++;
            tlen--;
        } while (tlen);
        sum1 = (sum1 & 0xffff) + (sum1 >> 16);
        sum2 = (sum2 & 0xffff) + (sum2 >> 16);
    }
    /* Second reduction step to reduce sums to 16 bits */
    sum1 = (sum1 & 0xffff) + (sum1 >> 16);
    sum2 = (sum2 & 0xffff) + (sum2 >> 16);
    return (sum2 << 16) | sum1;
}

static uint32_t read_7bit(const std::vector<uint8_t>& data, size_t* pos) {
    uint32_t value = 0;
    int shift = 0;
    while (shift < 32 && *pos < data.size()) {
        uint8_t b = data[*pos]; // get the byte
        (*pos)++;
        value |= (uint32_t) (b & 0x7F) << shift;
        if ((b >> 7) == 0) break;
        shift += 7;
    }
    return value;
}

static void write_7bit(uint32_t value, std::vector<uint8_t>* vec) {
    for (size_t i = 0; i < 5; i++) {
        uint8_t byte = value & 0x7F;
        value >>= 7;
        if (value)
            byte |= 1 << 7;
        vec->push_back(byte);
        if (value == 0)
            break;
    }
}

namespace libcom {
    std::string
    Msg::hex() const {
        std::stringstream ss;
        for (uint8_t v : _buffer) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int) v;
        }
        return ss.str();
    }
    // packet operators
    Msg&
    Msg::operator<<(const Packet& p) {
        write_7bit(p.id(), &_buffer);
        uint8_t sz = (uint8_t) std::min((size_t) 255, p.size());
        _buffer.push_back(sz);
        _buffer.insert(_buffer.end(), p.data(), p.data() + sz);
        return (*this);
    }

    Msg&
    Msg::operator>>(Packet& p) {
        // read starting from pos
        uint32_t id = read_7bit(_buffer, &_pos);
        if (_pos >= _buffer.size()) throw ParseError("Out of space");
        uint8_t size = _buffer[_pos++];
        if (_pos + size > _buffer.size()) throw ParseError("Array goes beyond end");
        uint8_t* data = &_buffer[_pos];
        _pos += size;
        p._id = id;
        p._size = size;
        p._data = data;
        return (*this);
    }

    // reading
    std::istream& operator>>(std::istream& i, Msg& p) {
        p.rewind();
        uint8_t c;
        for (int n = 0; n < 3; n++) {
            i >> c;
            if (c != 0xFF) throw ParseError("malformed header");
        }
        uint8_t len;
        i >> len; // read the length

        p._buffer.resize(len); // resize to length
        // read len bytes into the buffer
        i.read(reinterpret_cast<char*>(&p._buffer.front()), p._buffer.size());

        // read the checksum if we are expecting one
        uint8_t checksum_buf[4];
        i.read((char*) checksum_buf, 4);
        uint32_t checksum = *reinterpret_cast<uint32_t*>(checksum_buf);
        uint32_t expected = fletcher32(reinterpret_cast<const uint16_t*>(&p._buffer.front()), 
                                            p._buffer.size()/sizeof(uint16_t));
        if (checksum != expected) {
            throw ParseError("bad checksum");
        }
        return i;
    }

    // writing
    std::ostream& operator<<(std::ostream& o, const Msg& p) {
        // calculate the checksum
        // header
        o << (uint8_t) 0xFF;
        o << (uint8_t) 0xFF;
        o << (uint8_t) 0xFF;
        o << p._buffer.size();
        // write the data
        o.write(reinterpret_cast<const char*>(&p._buffer.front()), p._buffer.size());
        // write the checksum
        uint32_t checksum = fletcher32(reinterpret_cast<const uint16_t*>(&p._buffer.front()), 
                                            p._buffer.size()/sizeof(uint16_t));
        o.write(reinterpret_cast<const char*>(&checksum), sizeof(checksum));
        return o;
    }

    void
    Msg::drain(std::istream& i) {
        // read until next 3 0xFF's
        int num = 0;
        while (num < 3) {
            uint8_t c;
            i >> c;
            if (c == 0xFF) num++;
            else num = 0;
        }
        // put back 3 0xFF's
        i.putback(0xFF);
        i.putback(0xFF);
        i.putback(0xFF);
    }
}
