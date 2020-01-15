#ifndef __TELEGRAPH_IO_HPP__
#define __TELEGRAPH_IO_HPP__

#include <boost/asio/spawn.hpp>
#include <boost/asio/io_context.hpp>

namespace boost {
    namespace asio {

        // wraps a yield_context 
        // so we can forward declare stuff
        // and get a significant compilation
        // speedup
        struct yield_ctx {
            inline yield_ctx(const yield_context& c) : ctx(c) {}
            yield_context ctx;
        };
    }
}

namespace telegraph {
    namespace io = boost::asio;
}

#endif
