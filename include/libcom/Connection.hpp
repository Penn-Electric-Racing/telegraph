#ifndef __H_LIBCOM_CONNECTION_HPP__
#define __H_LIBCOM_CONNECTION_HPP__

#include "Msg.hpp"
#include "Param.hpp"

#include <functional>
#include <map>
#include <vector>
#include <iostream>
#include <memory>

namespace libcom {
    class GenericParam;

    class Connection {
    public:
        typedef std::function<void(const Packet&)> Handler;

        Connection();

        void add(Msg::Id, const Handler& f);

        void clear(); // clears the table

        void process(const Packet& p);

        void run(std::iostream* s); // runs the main thread

        // operator to add a handlers for parameters
        Connection& operator+=(GenericParam* p);
        Connection& operator+=(const std::shared_ptr<GenericParam> &p);
    private:
        std::map<Msg::Id, std::vector<Handler>> _handlers;
    };
}

#endif
