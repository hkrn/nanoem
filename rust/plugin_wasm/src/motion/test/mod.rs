/*
  Copyright (c) 2015-2023 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

use std::{
    any::Any,
    collections::{HashMap, HashSet},
    env::current_dir,
    io::IoSliceMut,
    sync::Arc,
};

use anyhow::Result;
use parking_lot::Mutex;

use pretty_assertions::assert_eq;
use rand::{thread_rng, Rng};
use serde::{Deserialize, Serialize};
use serde_json::Value;
use wasi_common::{
    file::{FileType, Filestat},
    snapshots::preview_1::types::Error,
    sync::WasiCtxBuilder,
    WasiFile,
};
use wasmtime::{Engine, Linker};

use crate::{motion::controller::MotionIOPluginController, Store};

use super::plugin::MotionIOPlugin;

#[derive(Debug, Default, PartialEq, Eq, Serialize, Deserialize)]
struct PluginOutput {
    function: String,
    arguments: Option<HashMap<String, Value>>,
}

pub(super) struct Pipe {
    content: Arc<Mutex<Vec<u8>>>,
}

impl Pipe {
    pub fn channel() -> (Box<Self>, Box<Self>) {
        let content = Arc::new(Mutex::new(Vec::new()));
        (
            Box::new(Self {
                content: Arc::clone(&content),
            }),
            Box::new(Self {
                content: Arc::clone(&content),
            }),
        )
    }
}

#[async_trait::async_trait]
impl WasiFile for Pipe {
    fn as_any(&self) -> &dyn Any {
        self
    }
    async fn get_filetype(&self) -> Result<FileType, Error> {
        Ok(FileType::RegularFile)
    }
    async fn get_filestat(&self) -> Result<Filestat, Error> {
        Ok(Filestat {
            device_id: 0,
            inode: 0,
            filetype: self.get_filetype().await?,
            nlink: 0,
            size: self.content.lock().len() as _,
            atim: None,
            mtim: None,
            ctim: None,
        })
    }
    async fn read_vectored<'a>(&self, _bufs: &mut [std::io::IoSliceMut<'a>]) -> Result<u64, Error> {
        let guard = self.content.lock();
        for slice in _bufs.iter_mut() {
            slice.copy_from_slice(guard.as_slice());
        }
        Ok(guard.len() as u64)
    }
    async fn write_vectored<'a>(&self, _bufs: &[std::io::IoSlice<'a>]) -> Result<u64, Error> {
        let mut guard = self.content.lock();
        let size = guard.len();
        for slice in _bufs.iter() {
            guard.extend(slice.iter());
        }
        Ok((guard.len() - size) as u64)
    }
}

fn build_type_and_flags() -> (&'static str, &'static str) {
    if cfg!(debug_assertions) {
        ("debug", "")
    } else {
        ("release-lto", " --profile release-lto")
    }
}

fn create_random_data(size: usize) -> Vec<u8> {
    let mut data = vec![];
    let mut rng = thread_rng();
    for _ in 0..size {
        let v: u8 = rng.r#gen();
        data.push(v);
    }
    data
}

fn read_plugin_output(pipe: Box<dyn WasiFile>) -> Result<Vec<PluginOutput>> {
    let stat = futures::executor::block_on(pipe.get_filestat())?;
    let mut buffer = vec![0; stat.size as _];
    futures::executor::block_on(pipe.read_vectored(&mut [IoSliceMut::new(&mut buffer)]))?;
    let s = String::from_utf8(buffer)?;
    let mut data = s.split('\n').collect::<Vec<_>>();
    data.pop();
    Ok(data
        .iter()
        .map(|s| serde_json::from_str(s).unwrap())
        .collect())
}

fn inner_create_controller(
    pipe: Box<dyn WasiFile>,
    path: &str,
) -> Result<MotionIOPluginController> {
    let path = current_dir()?.parent().unwrap().join(path);
    let engine = Engine::default();
    let data = WasiCtxBuilder::new().stdout(pipe).build();
    let store = Store::new(&engine, data);
    let bytes = std::fs::read(&path)?;
    let mut linker = Linker::new(&engine);
    wasi_common::sync::add_to_linker(&mut linker, |ctx| ctx)?;
    let plugin = MotionIOPlugin::new(&linker, &path, &bytes, store)?;
    let watcher = notify::recommended_watcher(|_res| {})?;
    Ok(MotionIOPluginController::new(
        Arc::new(Mutex::new(vec![plugin])),
        watcher,
    ))
}

#[test]
fn from_path() -> Result<()> {
    let (ty, _) = build_type_and_flags();
    let path = current_dir()?
        .parent()
        .unwrap()
        .join(format!("target/wasm32-wasi/{ty}/deps"));
    let mut controller = MotionIOPluginController::from_path(&path, |_builder| ())?;
    let mut names = vec![];
    for plugin in controller.all_plugins_mut().lock().iter_mut() {
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
