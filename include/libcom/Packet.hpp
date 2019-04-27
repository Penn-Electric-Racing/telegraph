#ifndef __H_LIBCOM_PACKET_HPP__
#define __H_LIBCOM_PACKET_HPP__

#include <cinttypes>
#include <cstdio>

namespace libcom {
    class Msg;
    class Packet {
    public:
        inline Packet(uint32_t id, size_t s, uint8_t* data) :
            _id(id), _size(s), _data(data) {}
        inline Packet() : _id(0), _size(0), _data(NULL) {}

        inline uint32_t id() const { return _id; }
        inline size_t size() const { return _size; }
        inline uint8_t* data() const { return _data; }
        friend class Msg;
    private:
        uint32_t _id;
        size_t _size;
        uint8_t *_data;
        uint8_t _tmp[8];
    };
}

#endif

