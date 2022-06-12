use async_graphql::{Object, Subscription, Context, Schema};
pub use async_graphql::{Result, Error};
use crate::types::{Tree, Source};
use crate::value::{Value, Datapoint};
use uuid::Uuid;
pub struct Query;

use std::sync::Arc;
use async_trait::async_trait;

pub type ApiSchema = Schema<Query, Mutation, Subscription>;

#[async_trait]
pub trait TreeManager : Sync + Send {
    async fn lookup_tree<'ctx>(&'ctx self, id: Uuid) -> Result<Option<Arc<Tree>>>;
    async fn live_tree<'ctx>(&'ctx self) -> Result<Option<Arc<Tree>>>;
    async fn trees<'ctx>(&'ctx self) -> Result<Vec<Arc<Tree>>>;
    async fn sources(&self) -> Result<Vec<Arc<Source>>>;
    // Mutations
    async fn delete_tree(&self, tree_id: Uuid) -> Result<bool>;
    async fn create_tree(&self, provider_id: Uuid, source_id: Uuid) -> Result<Option<Arc<Tree>>>;
    async fn set_live(&self, tree_id: Uuid) -> Result<Option<Arc<Tree>>>;
    async fn get_connection<'ctx>(&'ctx self, tree_id: Uuid) -> Result<&'ctx dyn Connection>;
}

#[async_trait]
pub trait Connection : Sync + Send {
    // Will set a particular tree to be live
    fn tree(&self) -> Arc<Tree>;
    async fn poll(&self, var_id: Uuid) -> Result<Option<Datapoint>>;
    async fn send_action(&self, action_id: Uuid, val: Value) -> Result<Option<Datapoint>>;
    async fn subscribe(&self, var_id: Uuid) -> Result<futures_lite::stream::Boxed<Datapoint>>;
}

#[Object]
impl Query {
    /// Returns the sum of a and b
    async fn lookup_tree<'ctx>(&self, ctx: &Context<'ctx>, id: Uuid) 
                -> Result<Option<Arc<Tree>>> {
        return ctx.data::<&dyn TreeManager>()?.lookup_tree(id).await;
    }
    async fn live_tree<'ctx>(&self, ctx: &Context<'ctx>) -> Result<Option<Arc<Tree>>> {
        return ctx.data::<&dyn TreeManager>()?.live_tree().await;
    }
    async fn trees<'ctx>(&self, ctx: &Context<'ctx>) -> Result<Vec<Arc<Tree>>> {
        return ctx.data::<&dyn TreeManager>()?.trees().await;
    }

    async fn sources<'ctx>(&self, ctx: &Context<'ctx>) -> Result<Vec<Arc<Source>>> {
        return ctx.data::<&dyn TreeManager>()?.sources().await;
    }
}

pub struct Mutation;

#[Object]
impl Mutation {
    async fn delete_tree(&self, ctx: &Context<'_>, tree_id: Uuid) -> Result<bool> { 
        return ctx.data::<&dyn TreeManager>()?.delete_tree(tree_id).await;
    }
    async fn create_tree(&self, ctx: &Context<'_>, provider_id: Uuid, source_id: Uuid) -> Result<Option<Arc<Tree>>> {
        return ctx.data::<&dyn TreeManager>()?.create_tree(provider_id, source_id).await;
    }
    // Will set a particular tree to be live
    async fn set_live(&self, ctx: &Context<'_>, tree_id: Uuid) -> Result<Option<Arc<Tree>>> {
        return ctx.data::<&dyn TreeManager>()?.set_live(tree_id).await;
    }

    // Tree actions
    async fn poll(&self, ctx: &Context<'_>, tree_id: Uuid, var_id: Uuid) -> Result<Option<Datapoint>> {
        let conn = ctx.data::<&dyn TreeManager>()?.get_connection(tree_id).await?;
        return conn.poll(var_id).await;
    }
    async fn send_action<'ctx>(&self, ctx: &Context<'ctx>, tree_id: Uuid, action_id: Uuid, val: Value) -> Result<Option<Datapoint>> {
        let conn = ctx.data::<&dyn TreeManager>()?.get_connection(tree_id).await?;
        return conn.send_action(action_id, val).await;
    }
}

pub struct Subscription;

#[Subscription]
impl Subscription {
    async fn variable(&self, ctx: &Context<'_>, tree_id: Uuid, variable_id: Uuid) -> Result<futures_lite::stream::Boxed<Datapoint>> {
        let conn = ctx.data::<&dyn TreeManager>()?.get_connection(tree_id).await?;
        return conn.subscribe(variable_id).await;
    }
}
