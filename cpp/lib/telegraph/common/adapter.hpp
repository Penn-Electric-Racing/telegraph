#ifndef __TELEGRAPH_COMMON_ADAPTER_HPP__
#define __TELEGRAPH_COMMON_ADAPTER_HPP__

#include <functional>

#include "data.hpp"
#include "../utils/io_fwd.hpp"

namespace telegraph {
    class adapter {
    public:
        adapter(const std::function<bool(io::yield_ctx&, uint32_t, uint32_t)>& change,
                        const std::function<bool(io::yield_ctx&)> cancel) 
                : change_sub_(change), cancel_sub_(cancel), actual_() {}

        subscription_ptr subscribe(uint32_t min_interval, uint32_t max_interval);
    private:
        std::function<bool(io::yield_ctx&, uint32_t, uint32_t)> change_sub_;
        std::function<bool(io::yield_ctx&)> cancel_sub_;
        subscription_ptr actual_;
    };
}


#endif
