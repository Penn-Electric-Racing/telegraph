#ifndef __PER_NODE_HPP__
#define __PER_NODE_HPP__

#include <memory>
#include <ostream>
#include <functional>

#include "../utils/signal.hpp"

namespace per {
    class group;
    typedef std::shared_ptr<group> shared_group;
    
    class variable;
    typedef std::shared_ptr<variable> shared_variable;

    class action;
    typedef std::shared_ptr<action> shared_action;

    class node;
    typedef std::shared_ptr<node> shared_node;
    typedef std::shared_ptr<const node> shared_const_node;

    std::ostream& operator<<(std::ostream&, const node&);

    class node : public std::enable_shared_from_this<node> {
        friend std::ostream& operator<<(std::ostream&, const node&);
    public:
        node(const std::string& name, 
             const std::string& pretty, const std::string& desc);

        constexpr const std::string& get_name() const { return name_; }
        constexpr const std::string& get_pretty() const { return pretty_; }
        constexpr const std::string& get_desc() const { return desc_; }

        constexpr const std::weak_ptr<group>& get_parent() const { return parent_; }

        inline void set_parent(const std::weak_ptr<group>& parent) { parent_ = parent; }

        // will return the /-prefixed path to this node
        std::string get_path() const;

        // will find a node based on a path
        shared_node find(const std::string& path);
        shared_const_node find(const std::string& path) const;

        // will get a child based on a name
        virtual shared_node operator[](const std::string& name);
        virtual shared_const_node operator[](const std::string& name) const;


        // will pass a visitor to all childen, 
        // with a bool for whether to visit children before or after this node
        virtual void visit(const std::function<void(const shared_node&)>& visitor, bool preorder=true);
        virtual void visit(const std::function<void(const shared_const_node&)>& visitor, bool preorder=true) const;

        virtual void print(std::ostream& o, int ident) const = 0;

        // signals for when a node is added or removed
        // anywhere in the tree
        signal<const shared_node&> node_added;
        signal<const shared_node&> node_removed;
    public:
        // note that name cannot change
        std::string name_;
        std::string pretty_; // For display
        std::string desc_; // For documentation
        std::weak_ptr<group> parent_; // The parent node
    };

}

#endif
