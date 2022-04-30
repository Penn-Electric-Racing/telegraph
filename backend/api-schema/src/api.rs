use async_graphql::{Object, Subscription};
use futures_core::stream::Stream;
use futures_lite::stream;
use crate::types::{Tree, Source};
use crate::value::{Value, Datapoint};
use uuid::Uuid;
pub struct Query;

#[Object]
impl Query {
    /// Returns the sum of a and b
    async fn tree_by_id(&self, id: Uuid) -> Option<Tree> {
        todo!()
    }

    async fn trees_by_name(&self, name: String) -> Vec<&Tree> {
        todo!()
    }

    async fn live_tree(&self) -> Option<&Tree> {
        todo!()
    }

    async fn trees(&self) -> Vec<Tree> {
        todo!()
    }

    async fn sources(&self) -> Vec<Source> {
        todo!()
    }
}

pub struct Mutation;

#[Object]
impl Mutation {
    async fn delete_tree(&self, tree_id: Uuid) -> bool { 
        todo!()
    }
    async fn create_tree(&self, source_id: Uuid) -> Option<Tree> {
        todo!()
    }
    // Will set a particular tree to be live
    async fn set_live(&self, tree_id: Uuid) -> Option<&Tree> {
        todo!()
    }

    // Tree actions
    async fn poll(&self, tree_id: Uuid, var_id: Uuid) -> Option<Datapoint> {
        None
    }
    async fn send_action(&self, tree_id: Uuid, action_id: Uuid, val: Value) -> Option<Datapoint> {
        todo!()
    }
}

pub struct Subscription;

#[Subscription]
impl Subscription {
    async fn variable(&self, tree_id: Uuid, variable_id: Uuid) -> impl Stream<Item=Datapoint> {
        stream::iter(vec![])
    }
}
