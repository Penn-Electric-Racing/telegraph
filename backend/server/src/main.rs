use server::Server;

use std::convert::Infallible;

use log::{error, info};
use tokio::net::TcpListener;

use anyhow::{bail, Result};

mod graphql;
use graphql::Database;

use std::{env, pin::Pin, sync::Arc, time::Duration};

use futures::{FutureExt as _, Stream};
use juniper::{
    graphql_object, graphql_subscription, graphql_value, EmptyMutation, FieldError, GraphQLEnum,
    RootNode,
};
use juniper_graphql_ws::ConnectionConfig;
use juniper_warp::{playground_filter, subscriptions::serve_graphql_ws};
use warp::{http::Response, Filter};

use std::net::SocketAddr;

// TODO: turn this main function into `try_main` and wrap it with a little bit of logic that
// pretty-prints any error messages before terminating with an appropriate error code

#[tokio::main]
async fn main() -> Result<()> {
    let mut logger_builder = env_logger::Builder::new();
    if cfg!(debug_assertions) {
        logger_builder.filter_level(log::LevelFilter::Debug);
    } else {
        // TODO: we might want to ratchet this up a bit when we start shipping the server
        logger_builder.filter_level(log::LevelFilter::Info);
    }
    logger_builder.parse_env("LOG_LEVEL");
    logger_builder.init();

    let addr = match env::var("TELEGRAPH_SERVER_ADDR") {
        Ok(addr) => addr,
        Err(env::VarError::NotPresent) => "0.0.0.0".to_string(),
        Err(err) => bail!("error reading the server address: {}", err),
    };

    let graphql_server = graphql_server(&addr, "8088");
    let websocket_server = websocket_server(&addr, "8081");

    let (graphql_result, websocket_result) =
        futures_util::future::join(graphql_server, websocket_server).await;
    return graphql_result.and(websocket_result);
}

async fn graphql_server(addr: &str, port: &str) -> Result<()> {
    let log = warp::log("telegraph server");

    let homepage = warp::path::end().map(|| {
        Response::builder()
            .header("content-type", "text/html")
            .body("<html><h1>Telegraph Server</h1><div>visit <a href=\"/playground\">graphql playground</a></html>".to_string())
    });

    let qm_schema = graphql::schema();
    let qm_state = warp::any().map(move || graphql::Database {});
    let qm_graphql_filter = juniper_warp::make_graphql_filter(qm_schema, qm_state.boxed());

    let root_node = Arc::new(graphql::schema());

    log::info!("Listening on {}:{}", addr, port);

    let routes = (warp::path("subscriptions")
        .and(warp::ws())
        .map(move |ws: warp::ws::Ws| {
            let root_node = root_node.clone();
            ws.on_upgrade(move |websocket| async move {
                serve_graphql_ws(websocket, root_node, ConnectionConfig::new(Database {}))
                    .map(|r| {
                        if let Err(e) = r {
                            println!("Websocket error: {}", e);
                        }
                    })
                    .await
            })
        }))
    .map(|reply| {
        // TODO#584: remove this workaround
        warp::reply::with_header(reply, "Sec-WebSocket-Protocol", "graphql-ws")
    })
    .or(warp::post()
        .and(warp::path("graphql"))
        .and(qm_graphql_filter))
    .or(warp::get()
        .and(warp::path("playground"))
        .and(playground_filter("/graphql", Some("/subscriptions"))))
    .or(homepage)
    .with(log);

    warp::serve(routes)
        .run(format!("{}:{}", addr, port).parse::<SocketAddr>()?)
        .await;

    Ok(())
}

async fn websocket_server(addr: &str, port: &str) -> Result<()> {
    info!("Listening for websocket connections on: {}:{}", addr, port);

    let listener = TcpListener::bind(format!("{}:{}", addr, port)).await?;

    loop {
        let (stream, _) = listener.accept().await?;

        tokio::spawn(async move {
            let server = Server::new();

            if let Err(error) = server.accept_connection(stream).await {
                error!("Closing connection due to error: {}", error)
            }
        });
    }
}
