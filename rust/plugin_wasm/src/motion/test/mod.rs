/*
  Copyright (c) 2015-2021 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

use std::{
    collections::{HashMap, HashSet},
    env::current_dir,
};

use anyhow::Result;
use pretty_assertions::assert_eq;
use rand::{thread_rng, Rng};
use serde_derive::{Deserialize, Serialize};
use serde_json::Value;
use wasmer::Store;
use wasmer_wasi::{Pipe, WasiEnv, WasiState};

use super::plugin::{MotionIOPlugin, MotionIOPluginController};

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

fn inner_create_controller(path: &str, env: &mut WasiEnv) -> Result<MotionIOPluginController> {
    let path = current_dir()?.parent().unwrap().join(path);
    let store = Store::default();
    let bytes = std::fs::read(path)?;
    let plugin = MotionIOPlugin::new(&bytes, &store, env)?;
    Ok(MotionIOPluginController::new(vec![plugin]))
}

#[test]
fn from_path() -> Result<()> {
    let ty = if cfg!(debug_assertions) {
        "debug"
    } else {
        "release"
    };
    let path = current_dir()?
        .parent()
        .unwrap()
        .join(format!("target/wasm32-wasi/{}/deps", ty));
    let store = Store::default();
    let mut env = create_wasi_env()?;
    let mut controller = MotionIOPluginController::from_path(&path, &store, &mut env)?;
    let mut names = vec![];
    for plugin in controller.all_plugins_mut() {
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
            "plugin_wasm_test_motion_full",
            "plugin_wasm_test_motion_minimum",
        ],
        names
    );
    Ok(())
}

mod full;
mod minimum;
