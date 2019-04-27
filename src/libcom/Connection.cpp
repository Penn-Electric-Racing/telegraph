#include "Connection.hpp"

#include "Error.hpp"

namespace libcom {
    Connection::Connection() : _handlers() {}

    void
    Connection::add(Msg::Id id, const Handler& h) {
        _handlers[id].push_back(h);
    }

    void
    Connection::clear() {
        _handlers.clear();
    }

    void
    Connection::process(const Packet& p) {
        try {
            const std::vector<Handler>& handlers = _handlers.at(p.id());
            for (const Handler& h : handlers) {
                h(p);
            }
        } catch (const std::out_of_range& e) {}
    }

    void
    Connection::run(std::iostream* io) {
        bool ping = true;
        while (true) {
            try {
                if (ping) {
                    Msg p;
                    *io << p << std::flush;
                    ping = false;
                }
                Msg::drain(*io); // wait for header

                Msg m;
                *io >> m; // read into msg

                Packet packet;
                while (m.hasNext()) {
                    m >> packet; // read a packet
                    process(packet);
                }
            } catch (const Timeout& t) {
                io->clear(); // clear bad bit
                ping = true; // send ping next time
            } catch (const ParseError& e) {
                std::cout << "Error: " << e.what() << std::endl;
            } catch (const IOError& e) {
                std::cout << "IO Error: " << e.what() << std::endl;
                return;
            }
        }
    }

    Connection&
    Connection::operator+=(GenericParam* p) {
        add(p->id(), [p] (const Packet& pack) {
                p->update(pack.data(), pack.size());
            });
        return (*this);
    }

    Connection&
    Connection::operator+=(const std::shared_ptr<GenericParam> &p) {
        add(p->id(), [p] (const Packet& pack) {
                p->update(pack.data(), pack.size());
            });
        return (*this);
    }
}
