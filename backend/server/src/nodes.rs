use std::fmt;

use crate::errors::UnpackError;
use crate::types::Type;
use crate::wire;

/*
 * Node: things common to every node type
 */

// TODO: according to the protobuf this is an i32?

/// We use unsigned 16-bit integers to store the ID for a node
pub type NodeID = u16;

/// Represents a particular node in some device's tree
pub enum Node {
    Group(Group),
    Variable(Variable),
    Action(Action),
    Placeholder(Placeholder),
}

#[derive(Clone)]
/// All of the information that is common to all node variants
pub struct NodeInfo {
    /// The node's ID
    pub id: NodeID,
    /// The node's short name
    pub name: String,
    /// The node's ''pretty'', descriptive name
    pub pretty: String,
    /// The node's description
    pub desc: String,
    // /// The node's parent.
    // ///
    // /// We store this using a weak reference so that a node doesn't accidentally keep its parent
    // /// alive.
    // /// TODO: can this be a reference with a lifetime? Or is this necessary?
    // // parent: Option<Weak<Node>>

    // TODO: How should we handle contexts?
    // /// The enclosing context
    // // ctx: Weak<Refcell<Context>>
}

impl fmt::Display for NodeInfo {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "id: {}, name: {}, pretty: {}, description: {}",
            self.id, self.name, self.pretty, self.desc
        )
    }
}

impl Node {
    pub fn group<S: AsRef<str>>(
        id: NodeID,
        name: S,
        pretty: S,
        desc: S,
        children: Vec<Node>,
        schema: S,
        version: i32,
    ) -> Self {
        let info = NodeInfo {
            id,
            name: String::from(name.as_ref()),
            pretty: String::from(pretty.as_ref()),
            desc: String::from(desc.as_ref()),
        };
        Node::Group(Group {
            info,
            children,
            schema: String::from(schema.as_ref()),
            version,
        })
    }

    pub fn action<S: AsRef<str>>(
        id: NodeID,
        name: S,
        pretty: S,
        desc: S,
        arg_type: Type,
        ret_type: Type,
    ) -> Self {
        let info = NodeInfo {
            id,
            name: String::from(name.as_ref()),
            pretty: String::from(pretty.as_ref()),
            desc: String::from(desc.as_ref()),
        };
        Node::Action(Action {
            info,
            arg_type,
            ret_type,
        })
    }

    pub fn variable<S: AsRef<str>>(
        id: NodeID,
        name: S,
        pretty: S,
        desc: S,
        data_type: Type,
    ) -> Self {
        let info = NodeInfo {
            id,
            name: String::from(name.as_ref()),
            pretty: String::from(pretty.as_ref()),
            desc: String::from(desc.as_ref()),
        };
        Node::Variable(Variable { info, data_type })
    }

    pub fn placeholder(id: NodeID) -> Self {
        Node::Placeholder(Placeholder(id))
    }

    /// Get all of the information that is common to all node variants
    pub fn info(&self) -> &NodeInfo {
        match self {
            Node::Action(action) => &action.info,
            Node::Group(group) => &group.info,
            Node::Placeholder(_) => panic!("Tried to get info for a placeholder"),
            Node::Variable(variable) => &variable.info,
        }
    }

    /// The node's ID
    pub fn id(&self) -> NodeID {
        self.info().id
    }

    /// The node's short name
    pub fn name(&self) -> &String {
        &self.info().name
    }

    /// The node's ''pretty'', descriptive name
    pub fn pretty(&self) -> &String {
        &self.info().pretty
    }

    /// The node's description
    pub fn desc(&self) -> &String {
        &self.info().desc
    }

    // pub fn get_parent(&self) -> Option<Weak<Node>> {
    //     self.parent.clone()
    // }

    // fn topic(&self) -> &String;
    // fn path(&self) -> Vec<String>;

    // fn from_path(&self, p: &Vec<&str>, idx: usize) -> Weak<dyn Node>;

    // pub fn children(&self) -> Vec<Weak<Node>> {
    pub fn children(&self) -> Option<&Vec<Node>> {
        if let Node::Group(group) = self {
            Some(&group.children)
        } else {
            None
        }
    }

    // TODO: figure out what this is for
    //         virtual void set_owner(const std::weak_ptr<context>& c) {
    //             if (owner_.lock() && c.lock())
    //                 throw tree_error("node already has owner!");
    //             owner_ = c;
    //         }

    //         virtual void set_unowned() { owner_.reset(); }

    /// Deserialize this node from a protobuf node
    pub fn unpack(proto: &wire::Node) -> Result<Self, UnpackError> {
        use wire::node::Node as wNode;
        match &proto.node {
            None => Err(UnpackError::NullNodeError),
            Some(wNode::Group(group)) => Ok(Node::Group(Group::unpack(&group)?)),
            Some(wNode::Var(var)) => Ok(Node::Variable(Variable::unpack(&var)?)),
            Some(wNode::Action(action)) => Ok(Node::Action(Action::unpack(&action)?)),
            Some(wNode::Placeholder(placeholder)) => {
                Ok(Node::Placeholder(Placeholder::unpack(*placeholder)))
            }
        }
    }

    /// Serialize this node back into a protobuf node
    pub fn pack(&self) -> wire::Node {
        match self {
            Node::Group(group) => group.pack(),
            Node::Variable(variable) => variable.pack(),
            Node::Action(action) => action.pack(),
            Node::Placeholder(placeholder) => placeholder.pack(),
        }
    }

    /// The type of this node, as a string
    pub fn type_str(&self) -> &'static str {
        match self {
            Node::Action(_) => "Action",
            Node::Group(_) => "Group",
            Node::Placeholder(_) => "Placeholder",
            Node::Variable(_) => "Variable",
        }
    }

    //         virtual std::unique_ptr<node> clone() const = 0;
    //     protected:
    //         constexpr void set_parent(group* g) { parent_ = g; }
    //         virtual void print(std::ostream& o, int ident=0) const;

    //         id id_;
    //         std::string name_;
    //         std::string pretty_; // For display
    //         std::string desc_; // For documentation

    //         group* parent_;
    //         std::weak_ptr<context> owner_;
    //     }
}

impl fmt::Display for Node {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            // TODO: what else do we want to know about an action?
            Node::Action(action) => write!(f, "Action: {}", action.info),
            // TODO: what else do we want to know about a group? Should we recursively print its
            // children too?
            Node::Group(group) => write!(f, "Group: {}", group.info),
            Node::Placeholder(placeholder) => write!(f, "Placeholder: id: {}", placeholder.0),
            // TODO: what else do we want to know about a variable?
            Node::Variable(variable) => write!(f, "Variable: {}", variable.info),
        }
    }
}

/*
 * Groups
 */

/// A group of nodes (an internal vertex of the node tree)
pub struct Group {
    info: NodeInfo,
    // TODO: should this be a vector? And then we store an auxiliary hashmap? That's the way that
    // the C++ does it, but I don't see any advantage to doing it that way
    // pub children: HashMap<String, Rc<Node>>,
    // TODO: for now I am making this a vector, but I am still not convinced which it should be
    pub children: Vec<Node>,

    pub schema: String,
    pub version: i32,
}

impl Group {
    pub fn new(info: NodeInfo, children: Vec<Node>, schema: String, version: i32) -> Self {
        Self {
            info,
            children,
            schema,
            version,
        }
    }

    pub fn pack_children(&self) -> Vec<wire::Node> {
        unimplemented!()
    }

    /// Serialize this node back into a protobuf node
    pub fn pack(&self) -> wire::Node {
        wire::Node {
            node: Some(wire::node::Node::Group(wire::Group {
                id: self.info.id as i32,
                name: self.info.name.clone(),
                pretty: self.info.pretty.clone(),
                desc: self.info.desc.clone(),
                schema: self.schema.clone(),
                version: self.version,
                children: self.pack_children(),
            })),
        }
    }

    /// Deserialize this node from a protobuf node
    pub fn unpack(group: &wire::Group) -> Result<Self, UnpackError> {
        Ok(Group {
            info: NodeInfo {
                id: group.id as NodeID,
                name: group.name.clone(),
                pretty: group.pretty.clone(),
                desc: group.desc.clone(),
            },
            children: group
                .children
                .iter()
                .flat_map(|node| Node::unpack(node))
                .collect(),
            schema: group.schema.clone(),
            version: group.version,
        })
    }
}

// TODO: figure out how to make indexing work. Do we want to be able to index by ints?
// impl Index<usize> for Node {
//     type Output = Option<Weak<Node>>;

//     fn index(&self, idx: usize) -> &Self::Output {
//         if let NodeType::Group(group) = &self.node_type {

//         } else {
//             &None
//         }
//     }
// }

// class group : public node {
// public:
//     group(id i, const std::string_view& name, const std::string_view& pretty,
//                 const std::string_view& desc, const std::string_view& schema, int version,
//                 std::vector<node*>&& children) :
//             node(i, name, pretty, desc),
//             schema_(schema), version_(version), placeholders_(),
//             children_(children), children_map_() {
//         // populate the children map
//         for (node* n : children_) {
//             n->set_parent(this);
//             children_map_[n->get_name()] = n;
//         }
//     }
//     group(id i, const std::string_view& name, const std::string_view& pretty,
//                 const std::string_view& desc, const std::string_view& schema, int version,
//                 std::vector<node::id>&& placeholders) :
//             node(i, name, pretty, desc),
//             schema_(schema), version_(version), placeholders_(std::move(placeholders)),
//             children_(), children_map_() {
//         // populate the children map
//         for (node* n : children_) {
//             n->set_parent(this);
//             children_map_[n->get_name()] = n;
//         }
//     }
//     group(const group& g) :
//         node(g), schema_(g.get_schema()),
//         version_(g.get_version()), children_(),
//         children_map_() {
//         for (const node* n : g) {
//             std::unique_ptr<node> nn = n->clone();
//             node* r = nn.release();
//             r->set_parent(this);
//             children_.push_back(r);
//             children_map_[r->get_name()] = r;
//         }
//     }
//     ~group() {
//         for (node* n : children_) delete n;
//     }

//     const std::string& get_schema() const { return schema_; }
//     int get_version() const { return version_; }

//     void set_owner(const std::weak_ptr<context>& c) override {
//         node::set_owner(c);
//         for (node* child : children_) child->set_owner(c);
//     }

//     void set_unowned() override {
//         node::set_unowned();
//         for (node* c : children_) c->set_unowned();
//     }

//     node* from_path(const std::vector<std::string_view>& p,
//                             size_t idx=0) override {
//         if (idx > p.size()) return nullptr;
//         else if (idx == p.size()) return this;
//         else {
//             auto it = children_map_.find(p[idx]);
//             if (it == children_map_.end()) return nullptr;
//             return it->second->from_path(p, ++idx);
//         }
//     }
//     const node* from_path(const std::vector<std::string_view>& p,
//                                     size_t idx=0) const override {
//         if (idx > p.size()) return nullptr;
//         else if (idx == p.size()) return this;
//         else {
//             auto it = children_map_.find(p[idx]);
//             if (it == children_map_.end()) return nullptr;
//             return it->second->from_path(p, ++idx);
//         }
//     }

//     std::vector<node*> nodes() override {
//         std::vector<node*> n;
//         n.push_back(this);
//         for (node* c : children_) {
//             for (node* d : c->nodes()) {
//                 n.push_back(d);
//             }
//         }
//         return n;
//     }
//     std::vector<const node*> nodes() const override {
//         std::vector<const node*> n;
//         n.push_back(this);
//         for (const node* c : children_) {
//             for (const node* d : c->nodes()) {
//                 n.push_back(d);
//             }
//         }
//         return n;
//     }

//     const std::vector<node::id>& placeholders() const {
//         return placeholders_;
//     }

//     node* operator[](size_t idx) override {
//         if (idx >= children_.size()) return nullptr;
//         return children_[idx];
//     }
//     const node* operator[](size_t idx) const override {
//         if (idx >= children_.size()) return nullptr;
//         return children_[idx];
//     }
//     node* operator[](const std::string& child) override {
//         try {
//             return children_map_.at(child);
//         } catch (const std::out_of_range& e) {
//             return nullptr;
//         }
//     }
//     const node* operator[](const std::string& child) const override {
//         try {
//             return children_map_.at(child);
//         } catch (const std::out_of_range& e) {
//             return nullptr;
//         }
//     }
//     inline std::vector<node*>::iterator begin() { return children_.begin(); }
//     inline std::vector<node*>::const_iterator begin() const { return children_.begin(); }
//     inline std::vector<node*>::iterator end() { return children_.end(); }
//     inline std::vector<node*>::const_iterator end() const { return children_.end(); }

//     inline size_t num_children() const { return children_.size(); }

//     void pack(Group* group) const;
//     virtual void pack(Node* proto) const override;
//     static group* unpack(const Group& g);

//     template<typename M>
//         bool resolve_placeholders(M* nodes) {
//             for (node::id p : placeholders_) {
//                 if (nodes->find(p) == nodes->end()) return false;
//             }
//             for (node::id p : placeholders_) {
//                 auto nodes_it = nodes->find(p);
//                 if (nodes_it == nodes->end()) return false;
//                 node* child = nodes_it->second;

//                 children_.push_back(child);
//                 child->set_parent(this);
//                 children_map_[child->get_name()] = child;

//                 nodes->erase(nodes_it);
//             }
//             return true;
//         }

//     std::unique_ptr<node> clone() const override {
//         return std::make_unique<group>(*this);
//     }
// private:
//     void print(std::ostream& o, int ident=0) const override;

//     std::string schema_;
//     int version_;

//     std::vector<node::id> placeholders_;
//     std::vector<node*> children_;
//     std::map<std::string, node*, std::less<>> children_map_;
// };

/*
 * Actions
 */

/// An action that can be called on the device associated with this tree.
pub struct Action {
    info: NodeInfo,
    arg_type: Type,
    ret_type: Type,
}

impl Action {
    pub fn new(info: NodeInfo, arg_type: Type, ret_type: Type) -> Self {
        Self {
            info,
            arg_type,
            ret_type,
        }
    }

    pub fn arg_type(&self) -> &Type {
        &self.arg_type
    }

    pub fn ret_type(&self) -> &Type {
        &self.ret_type
    }

    pub fn pack(&self) -> wire::Node {
        wire::Node {
            node: Some(wire::node::Node::Action(wire::Action {
                id: self.info.id as i32,
                name: self.info.name.clone(),
                pretty: self.info.pretty.clone(),
                desc: self.info.desc.clone(),
                arg_type: Some(self.arg_type.pack()),
                ret_type: Some(self.arg_type.pack()),
            })),
        }
    }

    pub fn unpack(proto: &wire::Action) -> Result<Self, UnpackError> {
        Ok(Action {
            info: NodeInfo {
                id: proto.id as NodeID,
                name: proto.name.clone(),
                pretty: proto.pretty.clone(),
                desc: proto.desc.clone(),
            },
            arg_type: match &proto.arg_type {
                None => return Err(UnpackError::NullFieldError),
                Some(t) => Type::unpack(&t)?,
            },
            ret_type: match &proto.ret_type {
                None => return Err(UnpackError::NullFieldError),
                Some(t) => Type::unpack(&t)?,
            },
        })
    }
}

// class action : public node {
// public:
//     action(id i, const std::string_view& name,
//             const std::string_view& pretty, const std::string_view& desc,
//             const value_type& arg_type, const value_type& ret_type) :
//             node(i, name, pretty, desc),
//             arg_type_(arg_type), ret_type_(ret_type) {}
//     action(const action& a) : node(a),
//         arg_type_(a.get_arg_type()),
//         ret_type_(a.get_ret_type()) {}
//     const value_type& get_arg_type() const { return arg_type_; }
//     const value_type& get_ret_type() const { return ret_type_; }

//     void pack(Action* proto) const;
//     virtual void pack(Node* proto) const override;
//     static action* unpack(const Action& proto);

//     std::unique_ptr<node> clone() const override {
//         return std::make_unique<action>(*this);
//     }
// private:
//     void print(std::ostream& o, int ident=0) const override;
//     value_type arg_type_;
//     value_type ret_type_;
// };

/*
 * Variables
 */

/// A variable containing information about the device associated with this tree.
pub struct Variable {
    info: NodeInfo,
    data_type: Type,
}

impl Variable {
    pub fn new(info: NodeInfo, data_type: Type) -> Self {
        Self { info, data_type }
    }

    pub fn pack(&self) -> wire::Node {
        wire::Node {
            node: Some(wire::node::Node::Var(wire::Variable {
                id: self.info.id as i32,
                name: self.info.name.clone(),
                pretty: self.info.pretty.clone(),
                desc: self.info.desc.clone(),
                data_type: Some(self.data_type.pack()),
            })),
        }
    }

    pub fn unpack(proto: &wire::Variable) -> Result<Self, UnpackError> {
        Ok(Variable {
            info: NodeInfo {
                id: proto.id as NodeID,
                name: proto.name.clone(),
                pretty: proto.pretty.clone(),
                desc: proto.desc.clone(),
            },
            data_type: match &proto.data_type {
                None => return Err(UnpackError::NullFieldError),
                Some(t) => Type::unpack(&t)?,
            },
        })
    }
}

// class variable : public node {
// public:
//     variable(id i, const std::string_view& name,
//             const std::string_view& pretty, const std::string_view& desc,
//             const value_type& t) : node(i, name, pretty, desc), data_type_(t) {}
//     variable(const variable& v) :
//         node(v), data_type_(v.get_type()) {}
//     const value_type& get_type() const { return data_type_; }

//     bool compatible_with(node* other) const override;

//     void pack(Variable* var) const;
//     virtual void pack(Node* proto) const override;
//     static variable* unpack(const Variable& proto);

//     std::unique_ptr<node> clone() const override {
//         return std::make_unique<variable>(*this);
//     }
// private:
//     void print(std::ostream& o, int ident=0) const override;
//     value_type data_type_;
// };

/*
 * Placeholder
 */

/**
  A placeholder node.

  This represents a node in the nodetree that is yet to be populated. We use these because UART
  is unreliable for large messages, so we build the tree incrementally and then fill in
  placeholders at the end.
*/
pub struct Placeholder(NodeID);

impl Placeholder {
    pub fn new(id: NodeID) -> Self {
        Self(id)
    }

    pub fn pack(&self) -> wire::Node {
        wire::Node {
            node: Some(wire::node::Node::Placeholder(self.0 as i32)),
        }
    }

    pub fn unpack(proto: i32) -> Self {
        Placeholder(proto as NodeID)
    }
}
