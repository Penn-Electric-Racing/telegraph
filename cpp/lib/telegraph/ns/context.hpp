#ifndef __TELEGRAPH_CONTEXT_HPP__
#define __TELEGRAPH_CONTEXT_HPP__

#include <string>
#include <mutex>
#include <unordered_set>

#include "utils/signal.hpp"
#include "tree.hpp"

namespace telegraph {
    class context {
    public:
        inline context(name_space* ns, const std::string& uuid,
                const std::string& name, const std::string& type,
                const json& info) : ns_(ns), uuid_(uuid), name_(name),
                        type_(type), info_(info) {}

        const std::string& get_name() const { return name_; }
        const std::string& get_uuid() const { return uuid_; }
    private:
        name_space* ns_;
        std::string name_;
        std::string uuid_;
    };
}

#endif
