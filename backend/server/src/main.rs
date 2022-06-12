use clap::Parser;
use clap_verbosity_flag::{Verbosity, InfoLevel};
use api_schema::api::{ApiSchema, Query, Mutation, Subscription, TreeManager};
use perxml::AssignedCanIds;
use server::backend::{Backend, CanProvider, SourceProvider};
use warp::Filter;
use std::convert::Infallible;
use std::sync::Arc;


#[derive(Debug, Parser)]
#[clap(author, version, about, long_about = None)]
struct Cli {
    #[clap(long)]
    percan_xml: Option<String>,
    #[clap(long)]
    perdos_xml: Option<String>,
    // For connecting to can
    #[clap(long)]
    connect_can: Vec<String>,
    #[clap(long)]
    connect_serial: Vec<String>,
    #[clap(flatten)]
    verbosity: Verbosity<InfoLevel>,
}

#[tokio::main]
async fn main() {
    let args = Cli::parse();
    env_logger::Builder::new()
        .filter_level(args.verbosity.log_level_filter())
        .init();

    log::info!("Starting server...");

    let mut connections = Vec::new();
    let xml = std::fs::read_to_string(args.percan_xml.unwrap()).unwrap();
    let canids = AssignedCanIds::parse(xml).unwrap();
    let mut provider = CanProvider::new(canids);
    for can in args.connect_can {
        let sid = provider.add_source(&can);
        connections.push((provider.uuid(), sid))
    }
    let backend = Backend::new(vec![Arc::new(provider)]);

    for (pid, sid) in connections {
        backend.create_tree(pid, sid).await.unwrap();
    }

    let schema = ApiSchema::build(Query, Mutation, Subscription)
                        .data(backend).finish();
    let filter = async_graphql_warp::graphql(schema).and_then(|(schema, request): (ApiSchema, async_graphql::Request)| async move {
        // Execute query
        let resp = schema.execute(request).await;

        // Return result
        Ok::<_, Infallible>(async_graphql_warp::GraphQLResponse::from(resp))
    });
    warp::serve(filter).run(([0, 0, 0, 0], 8000)).await;
}