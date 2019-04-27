#include <libcom/Msg.hpp>
#include <libcom/Error.hpp>
#include <libcom/SerialPort.hpp>
#include <libcom/Param.hpp>
#include <libcom/PacketProcessor.hpp>

#include <sstream>
#include <iostream>

using namespace libcom;

// parameters
struct {
    Param<bool> error { 1239, "sdl.error" };
} sdl;


int main() {
    // essentially a switchboard
    // where we can register Parameters
    PacketProcessor proc;
    proc += &sdl.error;

    sdl.error.requestHandler([] (bool b) {
                std::cout << sdl.error.name() << " changed to " << b << std::endl;
            });

    SerialPort port("/dev/ttyACM0", 230400);

    // write the (empty) message to the port
    Msg m;
    port << m << std::flush;

    // read from the port
    Msg::drain(port); // wait for first header
    while (true) {
        try {
            // read a message
            std::cout << "reading message" << std::endl;
            port >> m;
            std::cout << m.len() << std::endl;
            Packet packet;
            while (m.hasNext()) {
                m >> packet; // read a packet
                size_t off = packet.data() - m.data();
                //std::cout << "  " << off << "  " << packet.id() << " " << packet.size() << std::endl;
                proc.process(packet);
            }
        } catch (const ParseError& e) {
            std::cout << "Error: " << e.what() << std::endl;
            Msg::drain(port);
        }
    } while (true);
}

