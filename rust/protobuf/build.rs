extern crate prost_build;

fn main() {
    let base = &format!(
        "{}/../../emapp/resources/protobuf",
        std::env::var("CARGO_MANIFEST_DIR").unwrap()
    );
    prost_build::compile_protos(&[format!("{}/plugin.proto", base)], &[base.to_owned()]).unwrap()
}
