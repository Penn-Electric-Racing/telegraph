use std::pin::Pin;

use futures::Stream;
use juniper::{
    graphql_object, graphql_subscription, EmptyMutation, FieldError, GraphQLObject, GraphQLUnion,
    RootNode,
};

use std::time::Duration;

#[derive(Clone)]
pub struct Database;

impl juniper::Context for Database {}

impl Database {
    pub fn new() -> Self {
        Self {}
    }
}

pub struct Query;

#[graphql_object(context = Database)]
impl Query {
    fn hello_world() -> &'static str {
        "Hello World!"
    }
}

pub struct Subscription;

#[derive(GraphQLObject)]
struct Context {
    name: String,
    // TODO: what is this?
    headless: bool,
    r#type: String,
    // TODO: this should be a full object of some kind
    params: i32,
    uuid: String,
}

#[derive(GraphQLObject)]
struct DestroyedContext {
    destroyed_uuid: String,
}

#[derive(GraphQLUnion)]
enum NamespacePacket {
    Created(Context),
    Destroyed(DestroyedContext),
}

fn deleted<S: ToString>(uuid: S) -> NamespacePacket {
    NamespacePacket::Destroyed(DestroyedContext {
        destroyed_uuid: uuid.to_string(),
    })
}

fn created<S: ToString, T: ToString, U: ToString>(
    name: S,
    headless: bool,
    r#type: T,
    params: i32,
    uuid: U,
) -> NamespacePacket {
    NamespacePacket::Created(Context {
        name: name.to_string(),
        headless,
        r#type: r#type.to_string(),
        // TODO: accept a params object
        params,
        uuid: uuid.to_string(),
    })
}

type NamespacePacketStream =
    Pin<Box<dyn Stream<Item = Result<NamespacePacket, FieldError>> + Send>>;

#[graphql_subscription(context = Database)]
impl Subscription {
    async fn namespace_packet_stream() -> NamespacePacketStream {
        let stream = futures::stream::unfold(false, |created_last| async move {
            tokio::time::sleep(Duration::from_secs(5)).await;
            if created_last {
                Some((Ok(deleted("test2")), false))
            } else {
                Some((
                    Ok(created(
                        "Dummy device",
                        false,
                        "dummy_device",
                        0,
                        "dummy_device",
                    )),
                    true,
                ))
            }
        });

        Box::pin(stream)
    }
}

type Schema = RootNode<'static, Query, EmptyMutation<Database>, Subscription>;

pub fn schema() -> Schema {
    Schema::new(Query {}, EmptyMutation::new(), Subscription {})
}
