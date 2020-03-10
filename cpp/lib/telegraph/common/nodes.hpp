#ifndef __TELEGRAPH_NODES_HPP__
#define __TELEGRAPH_NODES_HPP__

#include "type.hpp"
#include "value.hpp"
#include "data.hpp"
#include "../utils/errors.hpp"

#include <memory>
#include <vector>
#include <string>
#include <string_view>
#include <unordered_map>

#include "common.pb.h"

namespace telegraph {

    class group;

    class context;
    using context_ptr = std::shared_ptr<context>;


    class node {
        friend std::ostream& operator<<(std::ostream&, const node&);
        friend group;
    public:
        using id = uint16_t;
        inline node(id i, const std::string_view& name, 
             const std::string_view& pretty, const std::string_view& desc) : 
                id_(i), name_(name), pretty_(pretty), 
                desc_(desc), ctx_(), parent_(nullptr) {}
        inline virtual ~node() {}

        virtual inline void set_ctx(const context_ptr& ctx) { 
            if (ctx_) throw tree_error("context already set");
            ctx_ = ctx; 
        }

        constexpr const id get_id() const { return id_; }
        constexpr const std::string& get_name() const { return name_; }
        constexpr const std::string& get_pretty() const { return pretty_; }
        constexpr const std::string& get_desc() const { return desc_; }

        constexpr group* get_parent() { return parent_; }
        constexpr const group* get_parent() const { return parent_; }

        // will return the /-prefixed path to this node
        std::string topic() const;
        std::vector<std::string> path() const;

        // To be overloaded by group
        virtual inline node* from_path(const std::vector<std::string>& p, 
                                       size_t idx=0) {
            return p.size() <= idx ? nullptr : this;
        }
        virtual inline const node* from_path(const std::vector<std::string>& p, 
                                       size_t idx=0) const {
            return p.size() <= idx ? nullptr : this;
        }
        virtual inline std::vector<node*> nodes() {
            std::vector<node*> n; 
            n.push_back(this);
            return n;
        }
        virtual inline std::vector<const node*> nodes() const {
            std::vector<const node*> n; 
            n.push_back(this);
            return n;
        }
        virtual node* operator[](size_t idx) { return nullptr; }
        virtual const node* operator[](size_t idx) const { return nullptr; }
        virtual node* operator[](const std::string& child) { return nullptr; }
        virtual const node* operator[](const std::string& child) const { return nullptr; }

        // pack and unpack functions
        virtual void pack(Node* proto) const = 0;
        static node* unpack(const Node& proto);
    protected:
        constexpr void set_parent(group* g) { parent_ = g; }
        virtual void print(std::ostream& o, int ident=0) const;

        id id_;
        std::string name_;
        std::string pretty_; // For display
        std::string desc_; // For documentation

        context_ptr ctx_;
        group* parent_;
    };

    // print operator
    inline std::ostream& operator<<(std::ostream& o, const node& n) {
        n.print(o, 0);
        return o;
    }

    class group : public node {
    public:
        inline group(id i, const std::string_view& name, const std::string_view& pretty,
                    const std::string_view& desc, const std::string_view& schema, int version,
                    std::vector<node*>&& children) : 
                node(i, name, pretty, desc),
                schema_(schema), version_(version), 
                children_(children), children_map_() {
            // populate the children map
            for (node* n : children_) {
                n->set_parent(this);
                children_map_[n->get_name()] = n;
            }
        }
        inline ~group() {
            for (node* n : children_) delete n;
        }

        const std::string& get_schema() const { return schema_; }
        int get_version() const { return version_; }

        virtual inline void set_ctx(const context_ptr& ctx) { 
            if (ctx_) throw tree_error("context already set");
            ctx_ = ctx; 
            for (node* n : children_) n->set_ctx(ctx);
        }

        inline node* from_path(const std::vector<std::string>& p, 
                                size_t idx=0) override {
            if (idx > p.size()) return nullptr;
            else if (idx == p.size()) return this;
            else {
                try {
                    return children_map_.at(p[idx]);
                } catch (const std::out_of_range& e) {
                    return nullptr;
                }
            }
        }
        inline const node* from_path(const std::vector<std::string>& p, 
                                        size_t idx=0) const override {
            if (idx > p.size()) return nullptr;
            else if (idx == p.size()) return this;
            else {
                try {
                    return children_map_.at(p[idx]);
                } catch (const std::out_of_range& e) {
                    return nullptr;
                }
            }
        }

        inline std::vector<node*> nodes() override {
            std::vector<node*> n; 
            n.push_back(this);
            for (node* c : children_) {
                for (node* d : c->nodes()) {
                    n.push_back(d);
                }
            }
            return n;
        }
        inline std::vector<const node*> nodes() const override {
            std::vector<const node*> n; 
            n.push_back(this);
            for (const node* c : children_) {
                for (const node* d : c->nodes()) {
                    n.push_back(d);
                }
            }
            return n;
        }

        inline node* operator[](size_t idx) override {
            if (idx >= children_.size()) return nullptr;
            return children_[idx];
        }
        inline const node* operator[](size_t idx) const override {
            if (idx >= children_.size()) return nullptr;
            return children_[idx];
        }
        inline node* operator[](const std::string& child) override {
            try {
                return children_map_.at(child);
            } catch (const std::out_of_range& e) {
                return nullptr;
            }
        }
        inline const node* operator[](const std::string& child) const override {
            try {
                return children_map_.at(child);
            } catch (const std::out_of_range& e) {
                return nullptr;
            }
        }
        inline std::vector<node*>::iterator begin() { return children_.begin(); }
        inline std::vector<node*>::const_iterator begin() const { return children_.begin(); }
        inline std::vector<node*>::iterator end() { return children_.end(); }
        inline std::vector<node*>::const_iterator end() const { return children_.end(); }

        inline size_t num_children() const { return children_.size(); }

        void pack(Group* group) const;
        virtual void pack(Node* proto) const override;
        static group* unpack(const Group& g);
    private:
        void print(std::ostream& o, int ident=0) const override;

        std::string schema_;
        int version_;

        std::vector<node*> children_;
        std::unordered_map<std::string, node*> children_map_;
    };


    class variable : public node {
    public:
        variable(id i, const std::string_view& name, 
                const std::string_view& pretty, const std::string_view& desc,
                const type& t) : node(i, name, pretty, desc), data_type_(t) {}
        const type& get_type() const { return data_type_; }

        void pack(Variable* var) const;
        virtual void pack(Node* proto) const override;
        static variable* unpack(const Variable& proto);
    private:
        void print(std::ostream& o, int ident=0) const override;
        type data_type_;
    };

    class action : public node {
    public:
        action(id i, const std::string_view& name,
                const std::string_view& pretty, const std::string_view& desc,
                const type& arg_type, const type& ret_type) : 
                node(i, name, pretty, desc), arg_type_(arg_type), ret_type_(ret_type) {}
        const type& get_arg_type() const { return arg_type_; }
        const type& get_ret_type() const { return ret_type_; }

        void pack(Action* proto) const;
        virtual void pack(Node* proto) const override;
        static action* unpack(const Action& proto);
    private:
        void print(std::ostream& o, int ident=0) const override;
        type arg_type_;
        type ret_type_;
    };
}

#endif
