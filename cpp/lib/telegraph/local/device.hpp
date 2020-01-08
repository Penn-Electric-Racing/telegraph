#ifndef __TELEGRAPH_LOCAL_DEVICE_HPP__
#define __TELEGRAPH_LOCAL_DEVICE_HPP__

#include "context.hpp"
#include "task.hpp"

#include <string>
#include <memory>

namespace telegraph {
    class io_task;
    class local_device : public local_context {
    public:
        local_device(const std::string& port, const std::vector<int>& bauds);

        const std::string& get_port() const { return port_; }

        // TODO: override the destroy to also dispose of any
        // associated tasks

        // You still should register
        // the task with the namespace before starting it
        std::shared_ptr<local_task> create_task(const std::string& name, int baud);

        friend class io_task;
    private:
        std::string port_;
        std::vector<int> bauds_;
        local_task* task_;
    };
}
#endif
