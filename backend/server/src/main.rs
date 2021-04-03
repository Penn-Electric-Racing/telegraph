use server::Server;

use std::env;

use log::{error, info};
use tokio::net::TcpListener;

use anyhow::{bail, Result};

// TODO: turn this main function into `try_main` and wrap it with a little bit of logic that
// pretty-prints any error messages before terminating with an appropriate error code

#[tokio::main]
async fn main() -> Result<()> {
    env_logger::init();

    let addr = match env::var("TELEGRAPH_SERVER_ADDR") {
        Ok(addr) => addr,
        Err(env::VarError::NotPresent) => "0.0.0.0:8081".to_string(),
        Err(err) => bail!("error reading the server address: {}", err),
    };

    let listener = TcpListener::bind(&addr).await?;

    info!("Listening on: {}", addr);

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
