#![cfg_attr(
    all(not(debug_assertions), target_os = "windows"),
    windows_subsystem = "windows"
)]

use bytes::{Bytes, BytesMut};
use futures_util::stream::{SplitStream, StreamExt};
use futures_util::{sink::SinkExt, stream::SplitSink};
use log::{debug, error, info};
use perdat2::{PerDos, PerDosMessage};
use std::collections::HashMap;
use std::convert::TryInto;
use std::io::ErrorKind;
use std::path::PathBuf;
use std::time::Duration;
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::sync::mpsc::{self, Receiver, Sender};
use tokio::sync::oneshot;
use tokio_serial::{SerialPortBuilderExt, SerialStream};
use tokio_util::codec::{Decoder, Framed};

#[derive(Debug, Clone, serde::Serialize)]
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

#[derive(Debug, Clone, serde::Serialize)]
struct PerDosValue {
    ty: per::value::Type,
    access_string: String,
    value: Option<Value>,
}

impl PerDosValue {
    pub fn set(&mut self, payload: Vec<u8>) {
        use per::value::Type;
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
    UpdateMessage {
        message: PerDosMessage,
    },
    Get {
        responder: oneshot::Sender<HashMap<u32, PerDosValue>>,
    },
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
    // info!("Starting read task!");
    while let Some(frame) = stream.next().await {
        match frame {
            Ok(frame) => {
                // info!("Received frame: {frame:?}");
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

async fn data_task(mut values: HashMap<u32, PerDosValue>, mut rx: Receiver<Command>) {
    while let Some(command) = rx.recv().await {
        match command {
            Command::UpdateMessage { message } => {
                if message.id == 10 {
                    // Ignore timestamp updates for now, since we are not saving
                    // the logging info anywhere
                    continue;
                }

                values.get_mut(&message.id).unwrap().set(message.payload);
                // info!("Updated database: {values:?}")
            }
            Command::Get { responder } => responder.send(values.clone()).unwrap(),
        }
    }
}

async fn setup(
    values: HashMap<u32, PerDosValue>,
    tx: Sender<Command>,
    rx: Receiver<Command>,
) -> Result<(), String> {
    // TODO: setup frequency
    let serial: tokio_serial::SerialStream = tokio_serial::new("/dev/ttyACM0", 921600)
        .timeout(std::time::Duration::from_secs(30))
        .parity(tokio_serial::Parity::None)
        .open_native_async()
        .map_err(|e| e.to_string())?;

    let framed = PerDos.framed(serial);
    let (sink, stream) = framed.split();

    tauri::async_runtime::spawn(async move { heartbeat_task(sink).await });
    tauri::async_runtime::spawn(async move { read_task(stream, tx).await });
    tauri::async_runtime::spawn(async move { data_task(values, rx).await });

    Ok(())
}

#[tauri::command]
fn greet(name: &str) -> String {
    format!("Hello, {}! You've been greeted from Rust!", name)
}

struct AsyncProcInput {
    tx: tokio::sync::Mutex<Option<Sender<Command>>>,
}

#[tauri::command]
async fn get_values(
    state: tauri::State<'_, AsyncProcInput>,
) -> Result<HashMap<u32, PerDosValue>, ()> {
    let (resp_tx, resp_rx) = oneshot::channel();
    let cmd = Command::Get { responder: resp_tx };

    // Send the GET request
    let tx_opt = state.tx.lock().await;
    let tx = tx_opt.as_ref().unwrap();
    tx.send(cmd).await.unwrap();

    // Await the response
    let res = resp_rx.await;
    Ok(res.unwrap())
}

#[tauri::command]
async fn list_devices() -> Result<Vec<String>, String> {
    Ok(tokio_serial::available_ports()
        .map_err(|err| err.to_string())?
        .iter()
        .map(|port| port.port_name.clone())
        .collect())
}

#[tauri::command]
async fn parse_display_file(display_file: &str) -> Result<per::display::Layouts, String> {
    let display_file = std::fs::File::open(display_file).map_err(|e| e.to_string())?;

    let mut layouts = per::display::Layouts::read_from(display_file).map_err(|e| e.to_string())?;
    Ok(layouts)
}

// #[tauri::command]
// async fn parse_layout(display_layout: &str) -> Result<LayoutFile, String> {
//     let display_layout = std::fs::File::open(
//         "/home/will/git/Penn-Electric-Racing/embedded/rev7/AssignedPerDosIds.xml",
//     )
//     .map_err(|e| e.to_string())?;

//     Ok(())
// }

#[tauri::command]
async fn connect_to_device(
    device: &str,
    can_ids_file: &str,
    state: tauri::State<'_, AsyncProcInput>,
) -> Result<HashMap<u32, PerDosValue>, String> {
    let assigned_perdos_ids = std::fs::File::open(can_ids_file).map_err(|e| e.to_string())?;

    let mut values: HashMap<u32, PerDosValue> =
        per::assigned_perdos_ids::AssignedPerDosIds::read_from(assigned_perdos_ids)
            .map_err(|e| e.to_string())?
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

    let (tx, rx) = mpsc::channel::<Command>(32);
    *state.tx.lock().await = Some(tx.clone());
    let setup_values = values.clone();
    setup(setup_values, tx, rx).await?;

    Ok(values)
}

fn main() {
    env_logger::init();

    // info!("Hello, world!");

    tauri::Builder::default()
        .invoke_handler(tauri::generate_handler![
            greet,
            get_values,
            list_devices,
            connect_to_device,
            parse_display_file
        ])
        .manage(AsyncProcInput {
            tx: tokio::sync::Mutex::new(None),
        })
        .setup(move |app| Ok(()))
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
