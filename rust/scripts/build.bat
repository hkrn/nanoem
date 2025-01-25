@echo off

set profiles=dev release-lto
set packages=plugin_wasm_test_model_minimum plugin_wasm_test_motion_minimum plugin_wasm_test_model_full plugin_wasm_test_motion_full

rustup target add wasm32-wasip1
cargo install cargo-deny
cargo deny check
for %%i in (%profiles%) do (
  for %%j in (%packages%) do (
    cargo build --profile %%i --package %%j --target wasm32-wasip1
  )
)
cargo build --profile release-lto
cargo test --profile release-lto
