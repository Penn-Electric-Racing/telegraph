use quicli::prelude::*;
use structopt::StructOpt;
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio_serial::SerialPortBuilderExt;

#[derive(Debug, StructOpt)]
struct Cli {
    #[structopt(flatten)]
    verbosity: Verbosity,

    /// Print all of the available serial devices and then quit
    #[structopt(long = "browse", short = "b")]
    browse: bool,
}

#[tokio::main]
async fn main() -> CliResult {
    let args = Cli::from_args();
    args.verbosity.setup_env_logger("server")?;

    info!("Hello, world!");

    if args.browse {
        browse()?
    } else {
        // let mut serial = tokio_serial::new("/dev/ttyACM0", 115200)
        //     .timeout(std::time::Duration::from_secs(30))
        //     .parity(tokio_serial::Parity::None)
        //     .open()?;
        let mut serial: tokio_serial::SerialStream = tokio_serial::new("/dev/ttyACM0", 921600)
            .timeout(std::time::Duration::from_secs(30))
            .parity(tokio_serial::Parity::None)
            .open_native_async()?;

        // TODO: investigate what happens when we open the serial port twice. I
        // think that it might make sense, for the keep-alive messages, to try
        // to open the serial port in non-exclusive mode? Idk, I don't want all
        // of the messages to get interleaved and clobbered, so the question is
        // how this operation mode is implemented on the host OS.

        info!("Writing the start message");
        // TODO: this isn't a "start message", it just extends the timeout a little
        // bit. A better solution will be to create a thread that repeatedly
        // updates the timeout, like
        // https://github.com/Penn-Electric-Racing/Penn-Electric-Racing/blob/rev7/desktop/csharp/DataServer/Communications/Client.cs,
        // but for now I just want to see *some* CAN messages, and hopefully
        // setting this once will allow me to read some before the connection
        // timeout expires.
        serial.write(&[0x02]).await?;
        info!("Start byte written, trying to read a response...");
        let mut buffer = [0; 1];
        while serial.read(&mut buffer).await? > 0 {
            println!("{:02x}", buffer[0]);
            // std::io::stdout().flush()?
        }
    }

    Ok(())
}

async fn serial_task(device_path: &impl AsRef<str>) {}

// async fn update_thread(&serial_device) {

// }

fn browse() -> CliResult {
    for port in tokio_serial::available_ports()? {
        println!("{}, which is of type {:?}", port.port_name, port.port_type);
    }

    Ok(())
}
