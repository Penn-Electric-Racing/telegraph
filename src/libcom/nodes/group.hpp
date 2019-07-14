#ifndef __PER_GROUP_HPP__
#define __PER_GROUP_HPP__

#include "node.hpp"
#include "../utils/signal.hpp"

#include <memory>
#include <vector>

namespace per {
    class group : public node {
    public:
        group();
        group(const std::string& name, const std::string& schema, int version);

        constexpr const std::string& get_schema() const { return schema_; }
        constexpr int get_version() const { return version_; }

        inline void set_schema(const std::string& schema) { schema_ = schema; }
        constexpr void set_version(int version) { version_ = version; }

        void add_child(const node& node);
        void add_child(const std::shared_ptr<node>& node);

        void remove_child(const node& node);
        void remove_child(const std::shared_ptr<node>& node);

        signal<std::shared_ptr<node>> on_child_added;
        signal<std::shared_ptr<node>> on_child_removed;
    private:
        std::string schema_;
        int version_;

        std::vector<std::shared_ptr<node>> children_;
    };
}

#endif
