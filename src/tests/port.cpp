#include <libcom/Msg.hpp>
#include <libcom/Error.hpp>
#include <libcom/Param.hpp>
#include <libcom/Connection.hpp>
#include <libcom/SerialPort.hpp>
#include <libcom/XML.hpp>

#include <sstream>
#include <iostream>

using namespace libcom;


int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "usage: port-test PerDos.xml" << std::endl;
        return 0;
    }

    // essentially a switchboard
    // where we can register Parameters
    Connection conn;

    std::vector<std::shared_ptr<GenericParam>> params = xml::readPerDosFile(argv[1]);

    for (const std::shared_ptr<GenericParam>& p : params) {
        std::cout << "read: " << p->name() << std::endl;
        p->updateHandler([&p] () {
            std::cout << p->name() << " changed to " << p->valueStr() << std::endl;
        });
        conn += p; // will capture the shared_ptr in Connection
    }


    SerialPort port("/dev/ttyACM0", 230400, 100);
    conn.run(&port);

    return 0;
}

