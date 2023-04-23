@echo off

set profiles=dev release-lto
set packages=plugin_wasm_test_model_minimum plugin_wasm_test_motion_minimum plugin_wasm_test_model_full plugin_wasm_test_motion_full

rustup target add wasm32-wasi
cargo install cargo-deny
cargo deny check
for %%profile in %profiles%
  for %%package in %packages%
    cargo build --profile %%profile --package %%package --target wasm32-wasi
  done
done
cargo build --profile release-lto
cargo test --profile release-lto
