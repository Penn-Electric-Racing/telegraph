#ifndef __H_LIBCOM_MSG_HPP__
#define __H_LIBCOM_MSG_HPP__

#include "Packet.hpp"

#include <cinttypes>
#include <ostream>
#include <istream>
#include <vector>

#define LIBCOM_MSG_MAX_LENGTH 255

namespace libcom {
    class Msg {
    public:
        typedef uint32_t Id;

        inline Msg() {}
        std::string hex() const;

        inline size_t len() const { return _buffer.size(); }
        inline const uint8_t* data() const { return &_buffer[0]; }

        // for extracting packets
        inline void rewind() { _pos = 0; }

        // whether we have space for this packet
        inline bool hasSpace(const Packet& p) { 
            return LIBCOM_MSG_MAX_LENGTH - _buffer.size() > p.size(); 
        }

        // whether there is space to extract another packet
        inline bool hasNext() {
            return _buffer.size() - _pos > 1 && _buffer[_pos] != 0x10 && _buffer[_pos] != 0x00;
        }

        // copy's the packet's memory into the buffer
        Msg& operator<<(const Packet& p);

        // extracts a packet from the message
        // but the memory of the packet stays in the Message
        // (i.e Packet does not own the memory!)
        Msg& operator>>(Packet& p);

        // format is:
        // 0xFFFFF + length (1 byte) + length  + data bytes + 4 checksum bytes of data

        // will also call rewind() on read
        friend std::istream& operator>>(std::istream& i, Msg& p);

        friend std::ostream& operator<<(std::ostream& o, const Msg& p);

        static void drain(std::istream& i); // read until next header
    private:
        // the data buffer
        std::vector<uint8_t> _buffer;
        size_t _pos; // read-out position for the packets
    };

    std::istream& operator>>(std::istream& o, Msg& p);
    std::ostream& operator<<(std::ostream& o, const Msg& p);
}

#endif

