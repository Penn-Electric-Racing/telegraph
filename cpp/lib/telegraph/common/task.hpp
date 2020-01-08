#ifndef __TELEGRAPH_TASK_HPP__
#define __TELEGRAPH_TASK_HPP__

#include <memory>

#include "../utils/uuid.hpp"
#include "../utils/io.hpp"
#include "../utils/info.hpp"

namespace telegraph {
    class task : public std::enable_shared_from_this<task> {
    public:
        inline task(uuid id, const std::string& name, 
                const std::string& type, const info& i) : 
            uuid_(id), name_(name), type_(type), info_(i) {}

        // queue the task into the io_context
        virtual void start(io::yield_context, io::io_context&) = 0;
        virtual void stop(io::yield_context) = 0;

        virtual void destroy() = 0;
    private:
        uuid uuid_;
        std::string name_;
        std::string type_;
        info info_;
    };
    using task_ptr = std::shared_ptr<task>;
}

#endif
