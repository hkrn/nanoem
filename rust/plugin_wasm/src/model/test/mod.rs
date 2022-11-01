/*
  Copyright (c) 2015-2021 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

use std::{
    collections::{HashMap, HashSet},
    env::current_dir,
    io::Read,
    sync::Arc,
};

use anyhow::Result;
use pretty_assertions::assert_eq;
use rand::{thread_rng, Rng};
use serde_derive::{Deserialize, Serialize};
use serde_json::Value;
use wasmer::Store;
use wasmer_wasi::{
    types::__WASI_STDOUT_FILENO, Pipe, WasiBidirectionalSharedPipePair, WasiFunctionEnv, WasiState,
};

use super::plugin::{ModelIOPlugin, ModelIOPluginController};

#[derive(Debug, Default, PartialEq, Eq, Serialize, Deserialize)]
struct PluginOutput {
    function: String,
    arguments: Option<HashMap<String, Value>>,
}

fn build_type_and_flags() -> (&'static str, &'static str) {
    if cfg!(debug_assertions) {
        ("debug", "")
    } else {
        ("release-lto", " --profile release-lto")
    }
}

fn create_wasi_env(
    pipe: &mut WasiBidirectionalSharedPipePair,
    store: &mut Store,
) -> Result<WasiFunctionEnv> {
    Ok(WasiState::new("nanoem")
        .stdout(Box::new(pipe.clone()))
        .finalize(store)?)
}

fn create_random_data(size: usize) -> Vec<u8> {
    let mut data = vec![];
    let mut rng = thread_rng();
    for _ in 0..size {
        let v: u8 = rng.gen();
        data.push(v);
    }
    data
}

fn flush_plugin_output(pipe: &mut WasiBidirectionalSharedPipePair) -> Result<()> {
    let mut v = vec![];
    pipe.read_to_end(&mut v)?;
    Ok(())
}

fn read_plugin_output(pipe: &mut WasiBidirectionalSharedPipePair) -> Result<Vec<PluginOutput>> {
    let mut s = String::new();
    pipe.read_to_string(&mut s)?;
    let mut data: Vec<_> = s.split('\n').collect();
    data.pop();
    Ok(data
        .iter()
        .map(|s| serde_json::from_str(s).unwrap())
        .collect())
}

fn inner_create_controller(
    pipe: &mut WasiBidirectionalSharedPipePair,
    path: &str,
) -> Result<ModelIOPluginController> {
    let path = current_dir()?.parent().unwrap().join(path);
    tracing::info!(path = ?path);
    let mut store = Store::default();
    let env = create_wasi_env(pipe, &mut store)?;
    let bytes = std::fs::read(path)?;
    let plugin = ModelIOPlugin::new(&bytes, env, store)?;
    Ok(ModelIOPluginController::new(vec![plugin]))
}

#[test]
fn from_path() -> Result<()> {
    let (ty, _) = build_type_and_flags();
    let path = current_dir()?
        .parent()
        .unwrap()
        .join(format!("target/wasm32-wasi/{}/deps", ty));
    let mut controller = ModelIOPluginController::from_path(&path)?;
    let mut names = vec![];
    for plugin in controller.all_plugins_mut() {
        let stdout = Box::new(Pipe::new());
        let state = Arc::clone(&plugin.wasi_env().state);
        let inodes = state.inodes.read().unwrap();
        state.fs.swap_file(&inodes, __WASI_STDOUT_FILENO, stdout)?;
        plugin.create()?;
        names.push(plugin.name()?);
    }
    let mut names = names
        .iter()
        .cloned()
        .collect::<HashSet<_>>()
        .iter()
        .cloned()
        .collect::<Vec<_>>();
    names.sort();
    assert_eq!(
        vec![
            "plugin_wasm_test_model_full",
            "plugin_wasm_test_model_minimum",
        ],
        names
    );
    Ok(())
}

mod full;
mod minimum;
