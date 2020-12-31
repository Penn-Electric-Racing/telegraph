fn main() {
    prost_build::compile_protos(
        &[
            "../../api.proto",
            "../../common.proto",
            "../../log.proto",
            "../../stream.proto",
        ],
        &["../.."],
    )
    .unwrap();
}
