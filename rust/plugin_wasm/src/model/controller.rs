/*
  Copyright (c) 2015-2023 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

use std::{ffi::CString, path::Path, sync::Arc};

use anyhow::Result;
use notify::Watcher;
use parking_lot::Mutex;
use walkdir::WalkDir;
use wasmtime::{Engine, Linker};
use wasmtime_wasi::WasiCtxBuilder;

use crate::Store;

use super::plugin::ModelIOPlugin;

pub struct ModelIOPluginController {
    plugins: Arc<Mutex<Vec<ModelIOPlugin>>>,
    function_indices: Vec<(usize, i32, CString)>,
    _watcher: notify::RecommendedWatcher,
    plugin_index: Option<usize>,
    failure_reason: Option<String>,
    recovery_suggestion: Option<String>,
}

impl ModelIOPluginController {
    pub fn new(
        plugins: Arc<Mutex<Vec<ModelIOPlugin>>>,
        watcher: notify::RecommendedWatcher,
    ) -> Self {
        let function_indices = vec![];
        Self {
            plugins,
            function_indices,
            _watcher: watcher,
            plugin_index: None,
            failure_reason: None,
            recovery_suggestion: None,
        }
    }
    pub fn from_path<F>(path: &Path, callback: F) -> Result<Self>
    where
        F: Fn(&mut WasiCtxBuilder) + Copy + std::marker::Sync + std::marker::Send + 'static,
    {
        let engine = Engine::default();
        let plugins = Arc::new(Mutex::new(vec![]));
        let plugins_inner = Arc::clone(&plugins);
        let mut linker = Linker::new(&engine);
        let linker_inner = linker.clone();
        wasmtime_wasi::add_to_linker(&mut linker, |ctx| ctx)?;
        let event_handler = move |res: notify::Result<notify::Event>| match res {
            Ok(ev) => {
                let create_plugin = |path: &Path| -> Result<ModelIOPlugin> {
                    let bytes = std::fs::read(path)?;
                    let mut builder = WasiCtxBuilder::new();
                    callback(&mut builder);
                    let data = builder.build();
                    let store = Store::new(linker_inner.engine(), data);
                    ModelIOPlugin::new(&linker_inner, path, &bytes, store)
                };
                match ev.kind {
                    notify::EventKind::Create(notify::event::CreateKind::File) => {
                        for path in ev.paths.iter() {
                            if path.ends_with(".wasm") {
                                match create_plugin(path) {
                                    Ok(plugin) => {
                                        tracing::info!(
                                            path = ?path,
                                            "WASM model I/O plugin is added",
                                        );
                                        plugins_inner.lock().push(plugin);
                                    }
                                    Err(err) => {
                                        tracing::warn!(
                                            error = %err,
                                            path = ?path,
                                            "Cannot create WASM model I/O plugin",
                                        )
                                    }
                                }
                            }
                        }
                    }
                    notify::EventKind::Modify(_) => {
                        for path in ev.paths.iter() {
                            let mut guard = plugins_inner.lock();
                            if let Some(plugin_mut) = guard
                                .iter_mut()
                                .find(|plugin: &&mut ModelIOPlugin| plugin.path() == path)
                            {
                                match create_plugin(path) {
                                    Ok(plugin) => {
                                        tracing::info!(
                                            path = ?path,
                                            "WASM model I/O plugin is modified and reloaded",
                                        );
                                        *plugin_mut = plugin
                                    }
                                    Err(err) => {
                                        tracing::warn!(
                                            error = %err,
                                            path = ?path,
                                            "Cannot reload WASM model I/O plugin",
                                        )
                                    }
                                }
                            }
                        }
                    }
                    notify::EventKind::Remove(_) => {
                        tracing::info!(
                            path = ?ev.paths,
                            "WASM model I/O plugins are removed",
                        );
                        plugins_inner
                            .lock()
                            .retain(|plugin| !ev.paths.contains(plugin.path()));
                    }
                    _ => {}
                }
            }
            Err(e) => tracing::warn!(error = ?e, "Catched an watch error"),
        };
        let mut watcher = notify::recommended_watcher(event_handler)?;
        for entry in WalkDir::new(path.parent().unwrap()) {
            let entry = entry?;
            let filename = entry.file_name().to_str();
            if filename.map(|s| s.ends_with(".wasm")).unwrap_or(false) {
                let bytes = std::fs::read(entry.path())?;
                let mut builder = WasiCtxBuilder::new();
                callback(&mut builder);
                let data = builder.build();
                let store = Store::new(linker.engine(), data);
                let path = entry.path();
                match ModelIOPlugin::new(&linker, path, &bytes, store) {
                    Ok(plugin) => {
                        watcher.watch(path, notify::RecursiveMode::NonRecursive)?;
                        plugins.lock().push(plugin);
                        tracing::debug!(
                            filename = filename.unwrap(),
                            "Loaded WASM model I/O plugin"
                        );
                    }
                    Err(err) => {
                        tracing::warn!(
                            filename = filename.unwrap(),
                            error = %err,
                            "Cannot load WASM model I/O plugin",
                        )
                    }
                }
            }
        }
        Ok(Self::new(plugins, watcher))
    }
    pub fn initialize(&mut self) -> Result<()> {
        self.plugins
            .lock()
            .iter_mut()
            .try_for_each(|plugin| plugin.initialize())
    }
    pub fn create(&mut self) -> Result<()> {
        let mut guard = self.plugins.lock();
        guard.iter_mut().try_for_each(|plugin| plugin.create())?;
        for (offset, plugin) in guard.iter_mut().enumerate() {
            let name = plugin.name()?;
            let version = plugin.version()?;
            for index in 0..plugin.count_all_functions()? {
                let name = CString::new(
                    &format!("{}: {} ({})", name, plugin.function_name(index)?, version)[..],
                )?;
                self.function_indices.push((offset, index, name));
            }
        }
        Ok(())
    }
    pub fn set_language(&mut self, value: i32) -> Result<()> {
        self.plugins
            .lock()
            .iter_mut()
            .try_for_each(|plugin| plugin.set_language(value))
    }
    pub fn count_all_functions(&self) -> i32 {
        self.function_indices.len() as i32
    }
    pub fn function_name(&self, index: i32) -> Result<&CString> {
        if let Some((_, _, name)) = self.function_indices.get(index as usize) {
            Ok(name)
        } else {
            Err(anyhow::anyhow!("out of bound function index: {}", index))
        }
    }
    pub fn set_function(&mut self, index: i32) -> Result<()> {
        if let Some((plugin_index, function_index, _)) =
            self.function_indices.get(index as usize).cloned()
        {
            let result = self.plugins.lock()[plugin_index].set_function(function_index);
            match result {
                Ok(0) => {
                    self.plugin_index = Some(plugin_index);
                    Ok(())
                }
                Ok(_) => self.set_failure(),
                Err(err) => Err(err),
            }
        } else {
            Err(anyhow::anyhow!("out of bound function index: {}", index))
        }
    }
    pub fn set_all_selected_vertex_indices(&mut self, data: &[i32]) -> Result<()> {
        self.current_plugin(|plugin| plugin.set_all_selected_vertex_indices(data))
    }
    pub fn set_all_selected_material_indices(&mut self, data: &[i32]) -> Result<()> {
        self.current_plugin(|plugin| plugin.set_all_selected_material_indices(data))
    }
    pub fn set_all_selected_bone_indices(&mut self, data: &[i32]) -> Result<()> {
        self.current_plugin(|plugin| plugin.set_all_selected_bone_indices(data))
    }
    pub fn set_all_selected_morph_indices(&mut self, data: &[i32]) -> Result<()> {
        self.current_plugin(|plugin| plugin.set_all_selected_morph_indices(data))
    }
    pub fn set_all_selected_label_indices(&mut self, data: &[i32]) -> Result<()> {
        self.current_plugin(|plugin| plugin.set_all_selected_label_indices(data))
    }
    pub fn set_all_selected_rigid_body_indices(&mut self, data: &[i32]) -> Result<()> {
        self.current_plugin(|plugin| plugin.set_all_selected_rigid_body_indices(data))
    }
    pub fn set_all_selected_joint_indices(&mut self, data: &[i32]) -> Result<()> {
        self.current_plugin(|plugin| plugin.set_all_selected_joint_indices(data))
    }
    pub fn set_all_selected_soft_body_indices(&mut self, data: &[i32]) -> Result<()> {
        self.current_plugin(|plugin| plugin.set_all_selected_soft_body_indices(data))
    }
    pub fn set_audio_description(&mut self, data: &[u8]) -> Result<()> {
        self.current_plugin(|plugin| plugin.set_audio_description(data))
    }
    pub fn set_camera_description(&mut self, data: &[u8]) -> Result<()> {
        self.current_plugin(|plugin| plugin.set_camera_description(data))
    }
    pub fn set_light_description(&mut self, data: &[u8]) -> Result<()> {
        self.current_plugin(|plugin| plugin.set_light_description(data))
    }
    pub fn set_audio_data(&mut self, data: &[u8]) -> Result<()> {
        self.current_plugin(|plugin| plugin.set_audio_data(data))
    }
    pub fn set_input_model_data(&mut self, bytes: &[u8]) -> Result<()> {
        self.current_plugin(|plugin| plugin.set_input_model_data(bytes))
    }
    pub fn execute(&mut self) -> Result<()> {
        let mut result = Ok(0);
        self.current_plugin(|plugin| {
            result = plugin.execute();
            Ok(())
        })?;
        match result {
            Ok(0) => Ok(()),
            Ok(_) => self.set_failure(),
            Err(err) => Err(err),
        }
    }
    pub fn get_output_data(&mut self) -> Result<Vec<u8>> {
        let mut bytes = vec![];
        self.current_plugin(|plugin| {
            bytes = plugin.get_output_data()?;
            Ok(())
        })?;
        Ok(bytes)
    }
    pub fn load_ui_window_layout(&mut self) -> Result<()> {
        let mut result = Ok(0);
        self.current_plugin(|plugin| {
            result = plugin.load_ui_window_layout();
            Ok(())
        })?;
        match result {
            Ok(0) => Ok(()),
            Ok(_) => self.set_failure(),
            Err(err) => Err(err),
        }
    }
    pub fn get_ui_window_layout(&mut self) -> Result<Vec<u8>> {
        let mut bytes = vec![];
        self.current_plugin(|plugin| {
            bytes = plugin.get_ui_window_layout()?;
            Ok(())
        })?;
        Ok(bytes)
    }
    pub fn set_ui_component_layout(
        &mut self,
        id: &str,
        data: &[u8],
        reload: &mut bool,
    ) -> Result<()> {
        let mut result = Ok(0);
        self.current_plugin(|plugin| {
            result = plugin.set_ui_component_layout(id, data, reload);
            Ok(())
        })?;
        match result {
            Ok(0) => Ok(()),
            Ok(_) => self.set_failure(),
            Err(err) => Err(err),
        }
    }
    pub fn failure_reason(&self) -> Option<String> {
        self.failure_reason.clone()
    }
    pub fn recovery_suggestion(&self) -> Option<String> {
        self.recovery_suggestion.clone()
    }
    pub fn assign_failure_reason(&mut self, value: anyhow::Error) {
        self.failure_reason = Some(value.to_string());
    }
    pub fn destroy(&mut self) {
        self.plugins
            .lock()
            .iter_mut()
            .for_each(|plugin| plugin.destroy())
    }
    pub fn terminate(&mut self) {
        self.plugins
            .lock()
            .iter_mut()
            .for_each(|plugin| plugin.terminate())
    }
    #[allow(unused)]
    pub(super) fn all_plugins_mut(&mut self) -> Arc<Mutex<Vec<ModelIOPlugin>>> {
        Arc::clone(&self.plugins)
    }
    fn set_failure(&mut self) -> Result<()> {
        let mut value = String::new();
        self.current_plugin(|plugin| {
            value = plugin.failure_reason()?;
            Ok(())
        })?;
        if !value.is_empty() {
            self.failure_reason = Some(value);
        }
        let mut value = String::new();
        self.current_plugin(|plugin| {
            value = plugin.recovery_suggestion()?;
            Ok(())
        })?;
        if !value.is_empty() {
            self.recovery_suggestion = Some(value);
        }
        Ok(())
    }
    fn current_plugin<T>(&mut self, cb: T) -> Result<()>
    where
        T: FnOnce(&mut ModelIOPlugin) -> Result<()>,
    {
        if let Some(plugin_index) = self.plugin_index {
            let plugins = &mut self.plugins.lock();
            cb(&mut plugins[plugin_index])
        } else {
            Err(anyhow::anyhow!("plugin is not set"))
        }
    }
}
