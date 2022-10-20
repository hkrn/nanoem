@echo off

rustup target add wasm32-wasi
cargo install cargo-deny
cargo deny check
cargo build --profile release-lto --package plugin_wasm_test_model_minimum --target wasm32-wasi
cargo build --profile release-lto --package plugin_wasm_test_motion_minimum --target wasm32-wasi
cargo build --profile release-lto --package plugin_wasm_test_model_full --target wasm32-wasi
cargo build --profile release-lto --package plugin_wasm_test_motion_full --target wasm32-wasi
cargo build --profile release-lto
cargo test --profile release-lto
