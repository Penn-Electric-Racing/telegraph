#include "forwarder.hpp"

namespace telegraph {

    forwarder::forwarder(connection& conn, namespace_* ns)
        : conn_(conn), ns_(ns) {
        // set the handlers
    }

    forwarder::~forwarder() {
        // unset the handlers
    }
}
