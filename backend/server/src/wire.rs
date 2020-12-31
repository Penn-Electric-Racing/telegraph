include!(concat!(env!("OUT_DIR"), "/telegraph.rs"));

pub mod api {
    include!(concat!(env!("OUT_DIR"), "/telegraph.api.rs"));
}

pub mod stream {
    include!(concat!(env!("OUT_DIR"), "/telegraph.stream.rs"));
}

pub mod log {
    include!(concat!(env!("OUT_DIR"), "/telegraph.log.rs"));
}
