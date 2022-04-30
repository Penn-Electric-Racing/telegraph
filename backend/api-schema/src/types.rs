use async_graphql::{SimpleObject, Union};
use uuid::Uuid;
use super::value::Type;

#[derive(SimpleObject)]
pub struct Tree {
    id: Uuid,
    name: String,
    root: Node
}

#[derive(SimpleObject)]
pub struct NodeInfo {
    id: Uuid,
    name: String
}

#[derive(SimpleObject)]
pub struct Group {
    info: NodeInfo,
    children: Vec<Node>
}

#[derive(SimpleObject)]
pub struct Variable {
    info: NodeInfo,
    data_type: Type
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
    id: Uuid,
    name: String
}