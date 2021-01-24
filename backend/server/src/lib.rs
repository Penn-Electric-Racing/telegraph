use futures_util::StreamExt;
use tokio::net::TcpStream;
use anyhow::{anyhow, Context, Result};
use prost::Message as _; // Import the message trait so that we can decode `Packet`s

pub mod types;
pub mod nodes;
// Disable clippy warnings for generated code, since we can't control/fix them
#[allow(clippy::all)]
pub mod wire;

pub struct Server {}

impl Server {
    pub fn new() -> Server {
        Server {}
    }

    pub async fn accept_connection(self, stream: TcpStream) -> Result<()> {
        let (mut _write, mut read) = tokio_tungstenite::accept_async(stream)
            .await
            .context("failed to accept the connection")?
            .split();

        while let Some(msg) = read.next().await {
            match msg {
                Ok(tungstenite::Message::Binary(bytes)) => {
                    let packet =
                        wire::api::Packet::decode(bytes.as_slice()).context("decoding message")?;

                    // Handle the packet
                    println!("{:?}", packet);
                }
                Ok(message) => {
                    return Err(anyhow!("got unexpected message: {:?}", message));
                }
                Err(error) => {
                    return Err(anyhow!("error while reading a message: {}", error));
                }
            }
        }

        Ok(())
    }
}
