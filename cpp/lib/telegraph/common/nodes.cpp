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

    bool
    group::compatible_with(node* o) const {
        group* g = dynamic_cast<group*>(o);
        if (!g) return false;
        bool children_compatible = true;
        for (node* c : children_) {
            if (!c->compatible_with((*g)[c->get_name()])) {
                children_compatible = false;
                break;
            }
        }
        bool c = children_compatible && g->get_name() == name_ && g->get_desc() == desc_
                && g->get_pretty() == pretty_ && g->get_version() == version_
                && g->get_schema() == schema_;
        return c;
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
        for (int i = 0; i < g.children_size(); i++) {
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

    bool
    variable::compatible_with(node* o) const {
        variable* v = dynamic_cast<variable*>(o);
        if (!v) return false;
        bool c = name_ == v->get_name() && data_type_ == v->get_type()
                && pretty_ == v->get_pretty() && desc_ == v->get_desc();
        return c;
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
                            value_type::unpack(proto.data_type()));
    }

    void
    variable::print(std::ostream& o, int ident) const {
        node::print(o, ident);
        o << name_ << ": " << data_type_.to_str() << " (" << pretty_ << ")";
    }

    bool
    action::compatible_with(node* o) const {
        action* a = dynamic_cast<action*>(o);
        if (!a) return false;
        return name_ == a->get_name() && arg_type_ == a->get_arg_type() &&
                ret_type_ == a->get_ret_type() &&
                pretty_ == a->get_pretty() && desc_ == a->get_desc();
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
                        value_type::unpack(proto.arg_type()),
                        value_type::unpack(proto.ret_type()));
    }

    void
    action::print(std::ostream& o, int ident) const {
        node::print(o, ident);
        o << name_ << ": " << arg_type_.to_str() << " -> " << ret_type_.to_str()
                   << " (" << pretty_ << ")";
    }
}
