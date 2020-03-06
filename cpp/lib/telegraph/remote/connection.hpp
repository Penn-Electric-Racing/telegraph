#ifndef __TELEGRAPH_CONNECTION_HPP__
#define __TELEGRAPH_CONNECTION_HPP__

#include "../utils/io.hpp"

namespace telegraph {
    namespace api {
        class Packet;
    }

    class connection {
    public:
        connection(bool count_down);
        void received(io::yield_ctx yield, const api::Packet& p);
        virtual void send(io::yield_ctx& yield, const api::Packet& p) = 0;
    private:
        bool count_down_;
    };
}

#endif
