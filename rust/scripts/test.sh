#!/bin/bash -eu

rustup target add wasm32-wasi
cargo install cargo-deny
cargo deny check
cargo build --release --package plugin_wasm_test_model_minimum --target wasm32-wasi
cargo build --release --package plugin_wasm_test_motion_minimum --target wasm32-wasi
cargo build --release --package plugin_wasm_test_model_full --target wasm32-wasi
cargo build --release --package plugin_wasm_test_motion_full --target wasm32-wasi
cargo test --release
