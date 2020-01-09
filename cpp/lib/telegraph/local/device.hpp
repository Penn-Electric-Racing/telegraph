#ifndef __TELEGRAPH_LOCAL_DEVICE_HPP__
#define __TELEGRAPH_LOCAL_DEVICE_HPP__

#include "namespace.hpp"

#include <string>
#include <memory>

namespace telegraph {
    class device_task;
    class local_device : public local_context {
    public:
        local_device(const std::string& name, const std::shared_ptr<node>& tree,
                     device_task* task);
    private:
        local_task* task_;
    };

    class device_task : public local_task {
    public:
        device_task(const std::string& name, 
                const std::string& port, int baud);
    private:
        std::shared_ptr<local_device> dev_;
    };
}
#endif
