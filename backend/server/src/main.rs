use bytes::{Bytes, BytesMut};
use futures_util::stream::{SplitStream, StreamExt};
use futures_util::{sink::SinkExt, stream::SplitSink};
use perdat2::{PerDos, PerDosMessage};
use quicli::prelude::*;
use std::collections::HashMap;
use std::convert::TryInto;
use std::io::ErrorKind;
use std::path::PathBuf;
use std::time::Duration;
use structopt::StructOpt;
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::sync::mpsc;
use tokio::sync::mpsc::Sender;
use tokio_serial::{SerialPortBuilderExt, SerialStream};
use tokio_util::codec::{Decoder, Framed};

#[derive(Debug, StructOpt)]
struct Cli {
    #[structopt(flatten)]
    verbosity: Verbosity,

    // /// Print all of the available serial devices and then quit
    // #[structopt(long = "browse", short = "b")]
    // browse: bool,
    assigned_perdos_ids: PathBuf,
}

#[derive(Debug)]
enum Value {
    Bool(bool),
    UInt8(u8),
    UInt16(u16),
    UInt32(u32),
    UInt64(u64),
    Int8(i8),
    Int16(i16),
    Int32(i32),
    Int64(i64),
    Float(f32),
    Double(f64),
}

#[derive(Debug)]
struct PerDosValue {
    ty: core::value::Type,
    access_string: String,
    value: Option<Value>,
}

impl PerDosValue {
    pub fn set(&mut self, payload: Vec<u8>) {
        use core::value::Type;
        self.value = Some(match &self.ty {
            Type::Bool => {
                assert!(payload.len() == 1);
                Value::Bool(payload[0] % 2 == 1)
            }
            Type::UInt8 => {
                assert!(payload.len() == 1);
                Value::UInt8(u8::from_le_bytes(payload.try_into().unwrap()))
            }
            Type::UInt16 => {
                assert!(payload.len() == 2);
                Value::UInt16(u16::from_le_bytes(payload.try_into().unwrap()))
            }
            Type::UInt32 => {
                assert!(payload.len() == 4);
                Value::UInt32(u32::from_le_bytes(payload.try_into().unwrap()))
            }
            Type::UInt64 => {
                assert!(payload.len() == 8);
                Value::UInt64(u64::from_le_bytes(payload.try_into().unwrap()))
            }
            Type::Int8 => {
                assert!(payload.len() == 1);
                Value::Int8(i8::from_le_bytes(payload.try_into().unwrap()))
            }
            Type::Int16 => {
                assert!(payload.len() == 2);
                Value::Int16(i16::from_le_bytes(payload.try_into().unwrap()))
            }
            Type::Int32 => {
                assert!(payload.len() == 4);
                Value::Int32(i32::from_le_bytes(payload.try_into().unwrap()))
            }
            Type::Int64 => {
                assert!(payload.len() == 8);
                Value::Int64(i64::from_le_bytes(payload.try_into().unwrap()))
            }
            Type::Float => {
                assert!(payload.len() == 4);
                Value::Float(f32::from_le_bytes(payload.try_into().unwrap()))
            }
            Type::Double => {
                assert!(payload.len() == 8);
                Value::Double(f64::from_le_bytes(payload.try_into().unwrap()))
            }
        })
    }
}

#[derive(Debug)]
enum Command {
    UpdateMessage { message: PerDosMessage },
}

#[tokio::main]
async fn main() -> CliResult {
    let args = Cli::from_args();
    args.verbosity.setup_env_logger("server")?;

    let assigned_perdos_ids = std::fs::File::open(args.assigned_perdos_ids)?;
    let mut values: HashMap<u32, PerDosValue> =
        core::assigned_perdos_ids::AssignedPerDosIds::read_from(assigned_perdos_ids)
            .unwrap()
            .ids
            .iter()
            .map(|id| {
                (
                    id.id,
                    PerDosValue {
                        ty: id.value.ty.clone(),
                        access_string: id.access_string.clone(),
                        value: None,
                    },
                )
            })
            .collect();

    info!("Hello, world!");

    let serial: tokio_serial::SerialStream = tokio_serial::new("/dev/ttyACM0", 115200)
        .timeout(std::time::Duration::from_secs(30))
        .parity(tokio_serial::Parity::None)
        .open_native_async()?;

    let framed = PerDos.framed(serial);
    let (sink, stream) = framed.split();

    let (tx, mut rx) = mpsc::channel::<Command>(32);

    tokio::spawn(async move { heartbeat_task(sink).await });
    tokio::spawn(async move { read_task(stream, tx).await });

    while let Some(command) = rx.recv().await {
        match command {
            Command::UpdateMessage { message } => {
                if message.id == 10 {
                    // Ignore timestamp updates for now, since we are not saving
                    // the logging info anywhere
                    continue;
                }

                values.get_mut(&message.id).unwrap().set(message.payload);
                info!("Updated database: {values:?}")
            }
        }
    }

    loop {}

    Ok(())
}

async fn heartbeat_task(mut sink: SplitSink<Framed<SerialStream, PerDos>, Vec<PerDosMessage>>) {
    loop {
        sink.send(vec![PerDosMessage {
            id: 0x02,
            payload: vec![],
        }])
        .await
        .unwrap();

        tokio::time::sleep(Duration::from_secs_f32(0.25)).await;
    }
}

async fn read_task(mut stream: SplitStream<Framed<SerialStream, PerDos>>, tx: Sender<Command>) {
    info!("Starting read task!");
    while let Some(frame) = stream.next().await {
        match frame {
            Ok(frame) => {
                info!("Received frame: {frame:?}");
                for item in frame {
                    tx.send(Command::UpdateMessage { message: item })
                        .await
                        .unwrap();
                }
            }
            Err(err) => {
                error!("Couldn't receive frame due to error: {err}")
            }
        }
    }
}

fn browse() -> CliResult {
    for port in tokio_serial::available_ports()? {
        println!("{}, which is of type {:?}", port.port_name, port.port_type);
    }

    Ok(())
}
