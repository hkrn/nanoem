/*
  Copyright (c) 2015-2021 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

use std::{collections::HashMap, env::current_dir};

use anyhow::Result;
use rand::{thread_rng, Rng};
use serde_derive::{Deserialize, Serialize};
use serde_json::Value;
use wasmer::Store;
use wasmer_wasi::{Pipe, WasiEnv, WasiState};

use super::plugin::ModelIOPluginController;

#[derive(Debug, Default, PartialEq, Eq, Serialize, Deserialize)]
struct PluginOutput {
    function: String,
    arguments: Option<HashMap<String, Value>>,
}

fn create_wasi_env() -> Result<WasiEnv> {
    let stdout = Box::new(Pipe::new());
    Ok(WasiState::new("nanoem").stdout(stdout).finalize()?)
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

fn flush_plugin_output(env: &mut WasiEnv) -> Result<()> {
    let mut state = env.state();
    let stdout = state.fs.stdout_mut()?.as_mut().unwrap();
    let mut v = vec![];
    stdout.read_to_end(&mut v)?;
    Ok(())
}

fn read_plugin_output(env: &mut WasiEnv) -> Result<Vec<PluginOutput>> {
    let mut state = env.state();
    let stdout = state.fs.stdout_mut()?.as_mut().unwrap();
    let mut s = String::new();
    stdout.read_to_string(&mut s)?;
    let mut data: Vec<_> = s.split('\n').collect();
    data.pop();
    Ok(data
        .iter()
        .map(|s| serde_json::from_str(s).unwrap())
        .collect())
}

fn inner_create_controller(path: &str, env: &mut WasiEnv) -> Result<ModelIOPluginController> {
    let path = current_dir()?.parent().unwrap().join(path);
    let store = Store::default();
    ModelIOPluginController::new(&path, &store, env)
}

mod full;
mod minimum;
