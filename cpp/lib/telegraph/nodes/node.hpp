#ifndef __TELEGRAPH_NODE_HPP__
#define __TELEGRAPH_NODE_HPP__

#include <vector>
#include <memory>
#include <ostream>
#include <functional>

#include "../utils/signal.hpp"

namespace telegraph {
    class node;
    class group;
    std::ostream& operator<<(std::ostream&, const node&);

    class node {
        friend std::ostream& operator<<(std::ostream&, const node&);
        friend group;
    public:
        node(const std::string& name, 
             const std::string& pretty, const std::string& desc);
        virtual ~node();

        constexpr const std::string& get_name() const { return name_; }
        constexpr const std::string& get_pretty() const { return pretty_; }
        constexpr const std::string& get_desc() const { return desc_; }

        constexpr group* get_parent() { return parent_; }
        constexpr const group* get_parent() const { return parent_; }

        // will return the /-prefixed path to this node
        std::string get_path() const;

        // will find a node based on a path
        node* find(const std::string& path);
        const node* find(const std::string& path) const;

        // will get a child based on a name or index
        virtual node* operator[](const std::string& name);
        virtual const node* operator[](const std::string& name) const;

        inline std::vector<node*> descendants(bool include_this = false, 
                                                bool postorder = false) { 
            std::vector<node*> n; 
            add_descendants(&n, include_this, postorder);
            return n;
        }

        inline std::vector<const node*> descendants(bool include_this = false, 
                                               bool postorder = false) const {
            std::vector<const node*> n; 
            add_descendants(&n, include_this, postorder);
            return n;
        }

        signal<node*> on_descendant_added;
        signal<node*> on_descendant_removed;

        signal<> on_dispose;
    protected:
        virtual void add_descendants(std::vector<node*>* n, 
                                 bool include_this=false, 
                                 bool postorder=false);
        virtual void add_descendants(std::vector<const node*>* n, 
                                 bool include_this=false, 
                                 bool postorder=false)  const;

        virtual void print(std::ostream& o, int ident=0) const = 0;
        constexpr void set_parent(group* parent) { parent_ = parent; }

        std::string name_;
        std::string pretty_; // For display
        std::string desc_; // For documentation
        group* parent_;
    };

}

#endif
