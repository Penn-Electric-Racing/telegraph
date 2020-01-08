#ifndef __TELEGRAPH_LOCAL_TASK_HPP__
#define __TELEGRAPH_LOCAL_TASK_HPP__

#include "../common/task.hpp"

namespace telegraph {
    class local_namespace;

    class local_task : public task {
    public:
        local_task(const std::string& name,
                const std::string& type, const info& i);

        void reg(local_namespace* ns);

        void destroy() override;
    protected:
        local_namespace* ns_;
    };
}

#endif
