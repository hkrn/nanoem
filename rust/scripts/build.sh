#!/bin/bash -eu

rustup target add wasm32-wasip1
cargo install cargo-deny
cargo deny check
profiles=('dev' 'release-lto')
packages=(
  'plugin_wasm_test_model_minimum'
  'plugin_wasm_test_motion_minimum'
  'plugin_wasm_test_model_full'
  'plugin_wasm_test_motion_full'
)
for profile in "${profiles[@]}"; do
  for package in "${packages[@]}"; do
    cargo build --profile "${profile}" --package "${package}" --target wasm32-wasip1
  done
done
cargo build --profile release-lto
cargo test --profile release-lto
