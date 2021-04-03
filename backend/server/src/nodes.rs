use std::collections::HashMap;
use std::fmt;
use std::ops::Index;

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
    // /// TODO: can this be a reference with a lifetime? Or is this even necessary? What do we use
    // /// a node's parent for anyway?
    // // parent: Option<Weak<Node>>

    // TODO: How should we handle contexts?
    // /// The enclosing context
    // // ctx: Weak<Refcell<Context>>
}

impl NodeInfo {
    pub fn new<S: AsRef<str>>(id: NodeID, name: S, pretty: S, desc: S) -> Self {
        Self {
            id,
            name: String::from(name.as_ref()),
            pretty: String::from(pretty.as_ref()),
            desc: String::from(desc.as_ref()),
        }
    }
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
        let info = NodeInfo::new(id, name, pretty, desc);
        Node::Group(Group::new(info, children, schema.as_ref().to_string(), version))
    }

    pub fn action<S: AsRef<str>>(
        id: NodeID,
        name: S,
        pretty: S,
        desc: S,
        arg_type: Type,
        ret_type: Type,
    ) -> Self {
        let info = NodeInfo::new(id, name, pretty, desc);
        Node::Action(Action::new(info, arg_type, ret_type))
    }

    pub fn variable<S: AsRef<str>>(
        id: NodeID,
        name: S,
        pretty: S,
        desc: S,
        data_type: Type,
    ) -> Self {
        let info = NodeInfo::new(id, name, pretty, desc);
        Node::Variable(Variable::new(info, data_type))
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

    // TODO: what would this enable us to do? It would make lifetimes significantly more complicated
    // pub fn get_parent(&self) -> Option<Weak<Node>> {
    //     self.parent.clone()
    // }

    // TODO: do we need this? or does Rust's ownership handle this? Why do we need to know the node's owner?
    // virtual void set_owner(const std::weak_ptr<context>& c) {
    //     if (owner_.lock() && c.lock())
    //         throw tree_error("node already has owner!");
    //     owner_ = c;
    // }

    // virtual void set_unowned() { owner_.reset(); }

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
    children: HashMap<String, Node>,
    schema: String,
    version: i32,
}

impl Group {
    pub fn new(info: NodeInfo, children: Vec<Node>, schema: String, version: i32) -> Self {
        Self {
            info,
            children: children
                .into_iter()
                .map(|node| (node.name().clone(), node))
                .collect(),
            schema,
            version,
        }
    }

    pub fn children_iter(&self) -> impl Iterator<Item = &Node> {
        self.children.values()
    }

    pub fn children_iter_mut(&mut self) -> impl Iterator<Item = &mut Node> {
        self.children.values_mut()
    }

    pub fn get_child<S: AsRef<str>>(&self, idx: S) -> Option<&Node> {
        self.children.get(&idx.as_ref().to_string())
    }

    pub fn get_child_mut<S: AsRef<str>>(&mut self, idx: S) -> Option<&mut Node> {
        self.children.get_mut(&idx.as_ref().to_string())
    }

    pub fn schema(&self) -> &String {
        &self.schema
    }

    pub fn version(&self) -> i32 {
        self.version
    }

    pub fn pack_children(&self) -> Vec<wire::Node> {
        self.children_iter().map(|n| n.pack()).collect()
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
                .map(|node| (node.name().clone(), node))
                .collect(),
            schema: group.schema.clone(),
            version: group.version,
        })
    }

    // void set_owner(const std::weak_ptr<context>& c) override {
    //     node::set_owner(c);
    //     for (node* child : children_) child->set_owner(c);
    // }

    // void set_unowned() override {
    //     node::set_unowned();
    //     for (node* c : children_) c->set_unowned();
    // }

    // node* from_path(const std::vector<std::string_view>& p,
    //     size_t idx=0) override {
    //     if (idx > p.size()) return nullptr;
    //     else if (idx == p.size()) return this;
    //     else {
    //         auto it = children_map_.find(p[idx]);
    //         if (it == children_map_.end()) return nullptr;
    //         return it->second->from_path(p, ++idx);
    //     }
    // }
    // const node* from_path(const std::vector<std::string_view>& p,
    //     size_t idx=0) const override {
    //     if (idx > p.size()) return nullptr;
    //     else if (idx == p.size()) return this;
    //     else {
    //         auto it = children_map_.find(p[idx]);
    //         if (it == children_map_.end()) return nullptr;
    //         return it->second->from_path(p, ++idx);
    //     }
    // }

    // template<typename M>
    //     bool resolve_placeholders(M* nodes) {
    //         for (node::id p : placeholders_) {
    //             if (nodes->find(p) == nodes->end()) return false;
    //         }
    //         for (node::id p : placeholders_) {
    //             auto nodes_it = nodes->find(p);
    //             if (nodes_it == nodes->end()) return false;
    //             node* child = nodes_it->second;

    //             children_.push_back(child);
    //             child->set_parent(this);
    //             children_map_[child->get_name()] = child;

    //             nodes->erase(nodes_it);
    //         }
    //         return true;
    //     }
}

impl<S: AsRef<str>> Index<S> for Group {
    type Output = Node;

    fn index(&self, idx: S) -> &Self::Output {
        &self.children[&idx.as_ref().to_string()]
    }
}

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

    pub fn get_arg_type(&self) -> &Type {
        &self.arg_type
    }

    pub fn get_ret_type(&self) -> &Type {
        &self.ret_type
    }
}

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

    pub fn get_type(&self) -> &Type {
        &self.data_type
    }

    // bool compatible_with(node* other) const override;
}

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
