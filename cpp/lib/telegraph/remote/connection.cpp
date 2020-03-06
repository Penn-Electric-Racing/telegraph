#include "connection.hpp"

namespace telegraph {
    connection::connection(bool count_down) : count_down_(count_down) {}

    void
    connection::received(io::yield_ctx yield, const api::Packet& p) {
    }
}
