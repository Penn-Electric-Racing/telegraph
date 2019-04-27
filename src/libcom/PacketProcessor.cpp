#include "PacketProcessor.hpp"

namespace libcom {
    PacketProcessor::PacketProcessor() : _handlers() {}

    void
    PacketProcessor::add(Msg::Id id, const Handler& h) {
        _handlers[id].push_back(h);
    }

    void
    PacketProcessor::clear() {
        _handlers.clear();
    }

    void
    PacketProcessor::process(const Packet& p) {
        try {
            const std::vector<Handler>& handlers = _handlers.at(p.id());
            for (const Handler& h : handlers) {
                h(p);
            }
        } catch (const std::out_of_range& e) {}
    }
}
