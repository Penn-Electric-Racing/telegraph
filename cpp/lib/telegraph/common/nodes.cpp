#include "nodes.hpp"

namespace telegraph {

    std::string
    node::topic() const {
        return parent_ ? parent_->topic() + '/' + name_ : "";
    }

    std::vector<std::string>
    node::path() const {
        if (parent_) {
            std::vector<std::string> p = parent_->path();
            p.push_back(name_);
            return p;
        }
        std::vector<std::string> path;
        path.push_back(name_);
        return path;
    }


    node*
    node::unpack(const Node& proto) {
        switch (proto.node_case()) {
        case Node::kGroup: return group::unpack(proto.group());
        case Node::kVar: return variable::unpack(proto.var());
        case Node::kAction: return action::unpack(proto.action());
        default: return nullptr;
        }
    }

    void
    node::print(std::ostream& o, int ident) const {
        while (ident--) {
            o << ' ';
        }
    }

    void
    group::pack(Group* proto) const {
        proto->set_id(get_id());
        proto->set_name(get_name());
        proto->set_pretty(get_pretty());
        proto->set_desc(get_desc());
        proto->set_schema(get_schema());
        proto->set_version(get_version());
        for (const node* n : children_) 
            n->pack(proto->add_children());
    }

    void
    group::pack(Node* proto) const {
        pack(proto->mutable_group());
    }

    group*
    group::unpack(const Group& g) {
        std::vector<node*> children;
        for (int i = 0; g.children_size(); i++) {
            children.push_back(node::unpack(g.children(i)));
        }
        return new group(g.id(), g.name(), g.pretty(),
                         g.desc(), g.schema(), g.version(),
                         std::move(children));
    }

    void
    group::print(std::ostream& o, int ident) const {
        node::print(o, ident);
        o << name_ << ": " << schema_ << '/' << version_ << " (" << pretty_ << ')';
        for (const node* c : children_) {
            o << std::endl;
            c->print(o, ident + 4);
        }
    }

    void
    variable::pack(Variable* var) const {
        var->set_id(get_id());
        var->set_name(get_name());
        var->set_pretty(get_pretty());
        var->set_desc(get_desc());
        data_type_.pack(var->mutable_data_type());
    }

    void
    variable::pack(Node* n) const {
        pack(n->mutable_var());
    }

    variable*
    variable::unpack(const Variable& proto) {
        return new variable(proto.id(), proto.name(), 
                            proto.pretty(), proto.desc(),
                            type::unpack(proto.data_type()));
    }

    void
    variable::print(std::ostream& o, int ident) const {
        node::print(o, ident);
        o << name_ << ": " << data_type_.to_str() << " (" << pretty_ << ")";
    }

    void
    action::pack(Action* proto) const {
        proto->set_id(get_id());
        proto->set_name(get_name());
        proto->set_pretty(get_pretty());
        proto->set_desc(get_desc());
        arg_type_.pack(proto->mutable_arg_type());
        ret_type_.pack(proto->mutable_ret_type());
    }

    void
    action::pack(Node* n) const {
        pack(n->mutable_action());
    }

    action*
    action::unpack(const Action& proto) {
        return new action(proto.id(), proto.name(),
                        proto.pretty(), proto.desc(),
                        type::unpack(proto.arg_type()),
                        type::unpack(proto.ret_type()));
    }

    void
    action::print(std::ostream& o, int ident) const {
        node::print(o, ident);
        o << name_ << ": " << arg_type_.to_str() << " -> " << ret_type_.to_str()
                   << " (" << pretty_ << ")";
    }
}
