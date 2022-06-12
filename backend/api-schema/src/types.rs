use async_graphql::{SimpleObject, Union};
use uuid::Uuid;
use super::value::Type;

#[derive(SimpleObject)]
pub struct Tree {
    pub id: Uuid,
    pub name: String,
    pub root: Node
}

#[derive(SimpleObject)]
pub struct NodeInfo {
    pub id: Uuid,
    pub name: String, // variable name (in code)
    pub pretty: String, // name, but pretty
    pub description: String,
}

impl NodeInfo {
    pub fn new(name: String, pretty: String, desc: String) -> NodeInfo {
        NodeInfo { id: Uuid::new_v4(), name, pretty, description: desc }
    }
}

#[derive(SimpleObject)]
pub struct Group {
    pub info: NodeInfo,
    pub children: Vec<Node>
}

impl Group {
    pub fn root() -> Self {
        Group { info: NodeInfo::new("".to_string(), "".to_string(), "".to_string()), children: Vec::new() }
    }
    pub fn new(name: String) -> Self {
        Group { info: NodeInfo::new(name, "".to_string(), "".to_string()), children: Vec::new() }
    }
}

#[derive(SimpleObject)]
pub struct Variable {
    pub info: NodeInfo,
    pub data_type: Type
}

#[derive(SimpleObject)]
pub struct Action {
    info: NodeInfo,
    arg_type: Type,
    ret_type: Type 
}

#[derive(Union)]
pub enum Node {
    Group(Group),
    Variable(Variable),
    Action(Action)
}

#[derive(SimpleObject)]
pub struct Source {
    pub provider_id: Uuid,
    pub source_id: Uuid,
    pub dev_type: String,
    pub name: String
}