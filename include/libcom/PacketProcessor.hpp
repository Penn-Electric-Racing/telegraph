#ifndef __H_LIBCOM_PACKET_PROCESSOR_HPP__
#define __H_LIBCOM_PACKET_PROCESSOR_HPP__

#include "Msg.hpp"

#include <functional>
#include <map>
#include <vector>

namespace libcom {
    class PacketProcessor {
    public:
        typedef std::function<void(const Packet&)> Handler;

        PacketProcessor();

        void add(Msg::Id, const Handler& f);

        void clear(); // clears the table

        void process(const Packet& p);

    private:
        std::map<Msg::Id, std::vector<Handler>> _handlers;
    };
}

#endif
