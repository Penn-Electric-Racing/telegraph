#ifndef __PER_NODE_HPP__
#define __PER_NODE_HPP__

#include <memory>

namespace per {
    class node {
    public:
        node();
        node(const std::string& name);

        constexpr const std::string& get_name() const { return name_; }

        constexpr const std::string& get_pretty() const { return pretty_; }
        constexpr const std::string& get_desc() const { return desc_; }

        // Note: These things should not change after the node is added to a tree
        inline void set_name(const std::string& name) { name_ = name; }
        inline void set_pretty(const std::string& pretty) { pretty_ = pretty; }
        inline void set_desc(const std::string& desc) { desc_ = desc; }
    private:
        // note that name cannot change
        std::string name_;
        std::string pretty_; // For display
        std::string desc_; // For documentation
    };

    class group;
    typedef std::shared_ptr<group> shared_group;
    
    class variable;
    typedef std::shared_ptr<variable> shared_variable;

    typedef std::shared_ptr<node> shared_node;
}

#endif
