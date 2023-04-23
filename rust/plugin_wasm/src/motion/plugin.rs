/*
  Copyright (c) 2015-2023 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

use std::{ffi::CString, mem::size_of, path::Path, slice};

use anyhow::Result;
use tracing::warn;
use walkdir::WalkDir;
use wasmer::{Instance, Module, Store};
use wasmer_wasix::{wasmer_wasix_types::wasi::Errno, WasiEnv, WasiEnvBuilder, WasiFunctionEnv};

use crate::{
    allocate_byte_array_with_data, allocate_status_ptr, inner_count_all_functions,
    inner_create_opaque, inner_destroy_opaque, inner_execute, inner_get_data,
    inner_get_function_name, inner_get_string, inner_initialize_function, inner_load_ui_window,
    inner_set_data, inner_set_function, inner_set_language, inner_set_ui_component_layout,
    inner_terminate_function, release_byte_array, release_status_ptr, ByteArray, OpaquePtr,
    SizePtr, StatusPtr, FREE_FN, MALLOC_FN,
};

pub struct MotionIOPlugin {
    instance: Instance,
    store: Store,
    env: WasiFunctionEnv,
    opaque: Option<OpaquePtr>,
}

fn inner_set_named_data(
    instance: &Instance,
    opaque: &Option<OpaquePtr>,
    name: &str,
    data: &[u32],
    func: &str,
    store: &mut Store,
) -> Result<()> {
    if let Some(opaque) = opaque {
        if let Ok(set_input_model_data) =
            instance
                .exports
                .get_typed_function::<(OpaquePtr, ByteArray, ByteArray, u32, StatusPtr), ()>(
                    store, func,
                )
        {
            let component_size = size_of::<u32>();
            let data_size = data.len();
            let len = data_size * component_size;
            let data = unsafe { slice::from_raw_parts(data.as_ptr() as *const u8, len) };
            let mut name = name.as_bytes().to_vec();
            name.push(0);
            let name_ptr = allocate_byte_array_with_data(instance, name.as_slice(), store)?;
            let data_ptr = allocate_byte_array_with_data(instance, data, store)?;
            let status_ptr = allocate_status_ptr(instance, store)?;
            set_input_model_data.call(
                store,
                *opaque,
                name_ptr,
                data_ptr,
                data_size as u32,
                status_ptr,
            )?;
            release_byte_array(instance, name_ptr, store)?;
            release_byte_array(instance, data_ptr, store)?;
            release_status_ptr(instance, status_ptr, store)?;
        }
    }
    Ok(())
}

fn validate_plugin(instance: &Instance, store: &Store) -> Result<()> {
    let e = &instance.exports;
    e.get_memory("memory")?;
    e.get_typed_function::<u32, OpaquePtr>(store, MALLOC_FN)?;
    e.get_typed_function::<OpaquePtr, ()>(store, FREE_FN)?;
    e.get_typed_function::<(), OpaquePtr>(store, "nanoemApplicationPluginMotionIOCreate")?;
    e.get_typed_function::<OpaquePtr, ByteArray>(store, "nanoemApplicationPluginMotionIOGetName")?;
    e.get_typed_function::<OpaquePtr, ByteArray>(
        store,
        "nanoemApplicationPluginMotionIOGetVersion",
    )?;
    e.get_typed_function::<(OpaquePtr, i32), ()>(
        store,
        "nanoemApplicationPluginMotionIOSetLanguage",
    )?;
    e.get_typed_function::<OpaquePtr, i32>(
        store,
        "nanoemApplicationPluginMotionIOCountAllFunctions",
    )?;
    e.get_typed_function::<(OpaquePtr, i32), ByteArray>(
        store,
        "nanoemApplicationPluginMotionIOGetFunctionName",
    )?;
    e.get_typed_function::<(OpaquePtr, i32, StatusPtr), ()>(
        store,
        "nanoemApplicationPluginMotionIOSetFunction",
    )?;
    e.get_typed_function::<(OpaquePtr, ByteArray, u32, StatusPtr), ()>(
        store,
        "nanoemApplicationPluginMotionIOSetInputMotionData",
    )?;
    e.get_typed_function::<(OpaquePtr, StatusPtr), ()>(
        store,
        "nanoemApplicationPluginMotionIOExecute",
    )?;
    e.get_typed_function::<(OpaquePtr, ByteArray, u32, StatusPtr), ()>(
        store,
        "nanoemApplicationPluginMotionIOGetOutputMotionData",
    )?;
    e.get_typed_function::<(OpaquePtr, SizePtr), ()>(
        store,
        "nanoemApplicationPluginMotionIOGetOutputMotionDataSize",
    )?;
    e.get_typed_function::<OpaquePtr, ByteArray>(
        store,
        "nanoemApplicationPluginMotionIOGetFailureReason",
    )?;
    e.get_typed_function::<OpaquePtr, ()>(store, "nanoemApplicationPluginMotionIODestroy")?;
    Ok(())
}

impl MotionIOPlugin {
    pub fn new(bytes: &[u8], mut env: WasiFunctionEnv, mut store: Store) -> Result<Self> {
        let module = Module::new(&store, bytes)?;
        let imports = env.import_object(&mut store, &module)?;
        let instance = Instance::new(&mut store, &module, &imports)?;
        validate_plugin(&instance, &store)?;
        env.initialize(&mut store, instance.clone())?;
        Ok(Self {
            instance,
            store,
            env,
            opaque: None,
        })
    }
    pub fn initialize(&mut self) -> Result<()> {
        inner_initialize_function(
            &self.instance,
            "nanoemApplicationPluginMotionIOInitialize",
            &mut self.store,
        )
    }
    pub fn create(&mut self) -> Result<()> {
        self.opaque = Some(inner_create_opaque(
            &self.instance,
            "nanoemApplicationPluginMotionIOCreate",
            &mut self.store,
        )?);
        Ok(())
    }
    pub fn set_language(&mut self, value: i32) -> Result<()> {
        inner_set_language(
            &self.instance,
            &self.opaque,
            value,
            "nanoemApplicationPluginMotionIOSetLanguage",
            &mut self.store,
        )
    }
    pub fn name(&mut self) -> Result<String> {
        inner_get_string(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginMotionIOGetName",
            &mut self.store,
        )
    }
    #[allow(unused)]
    pub fn description(&mut self) -> Result<String> {
        inner_get_string(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginMotionIOGetDescription",
            &mut self.store,
        )
    }
    #[allow(unused)]
    pub fn version(&mut self) -> Result<String> {
        inner_get_string(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginMotionIOGetVersion",
            &mut self.store,
        )
    }
    pub fn count_all_functions(&mut self) -> Result<i32> {
        inner_count_all_functions(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginMotionIOCountAllFunctions",
            &mut self.store,
        )
    }
    pub fn function_name(&mut self, index: i32) -> Result<String> {
        inner_get_function_name(
            &self.instance,
            &self.opaque,
            index,
            "nanoemApplicationPluginMotionIOGetFunctionName",
            &mut self.store,
        )
    }
    pub fn set_function(&mut self, index: i32) -> Result<i32> {
        inner_set_function(
            &self.instance,
            &self.opaque,
            index,
            "nanoemApplicationPluginMotionIOSetFunction",
            &mut self.store,
        )
    }
    pub fn set_all_selected_accessory_keyframes(&mut self, value: &[u32]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            value,
            "nanoemApplicationPluginMotionIOSetAllSelectedAccessoryKeyframes",
            &mut self.store,
        )
    }
    pub fn set_all_named_selected_bone_keyframes(
        &mut self,
        name: &str,
        value: &[u32],
    ) -> Result<()> {
        inner_set_named_data(
            &self.instance,
            &self.opaque,
            name,
            value,
            "nanoemApplicationPluginMotionIOSetAllNamedSelectedBoneKeyframes",
            &mut self.store,
        )
    }
    pub fn set_all_selected_camera_keyframes(&mut self, value: &[u32]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            value,
            "nanoemApplicationPluginMotionIOSetAllSelectedCameraKeyframes",
            &mut self.store,
        )
    }
    pub fn set_all_selected_light_keyframes(&mut self, value: &[u32]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            value,
            "nanoemApplicationPluginMotionIOSetAllSelectedLightKeyframes",
            &mut self.store,
        )
    }
    pub fn set_all_selected_model_keyframes(&mut self, value: &[u32]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            value,
            "nanoemApplicationPluginMotionIOSetAllSelectedModelKeyframes",
            &mut self.store,
        )
    }
    pub fn set_all_named_selected_morph_keyframes(
        &mut self,
        name: &str,
        value: &[u32],
    ) -> Result<()> {
        inner_set_named_data(
            &self.instance,
            &self.opaque,
            name,
            value,
            "nanoemApplicationPluginMotionIOSetAllNamedSelectedMorphKeyframes",
            &mut self.store,
        )
    }
    pub fn set_all_selected_self_shadow_keyframes(&mut self, value: &[u32]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            value,
            "nanoemApplicationPluginMotionIOSetAllSelectedSelfShadowKeyframes",
            &mut self.store,
        )
    }
    pub fn set_audio_description(&mut self, data: &[u8]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            data,
            "nanoemApplicationPluginMotionIOSetAudioDescription",
            &mut self.store,
        )
    }
    pub fn set_camera_description(&mut self, data: &[u8]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            data,
            "nanoemApplicationPluginMotionIOSetCameraDescription",
            &mut self.store,
        )
    }
    pub fn set_light_description(&mut self, data: &[u8]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            data,
            "nanoemApplicationPluginMotionIOSetLightDescription",
            &mut self.store,
        )
    }
    pub fn set_audio_data(&mut self, data: &[u8]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            data,
            "nanoemApplicationPluginMotionIOSetInputAudioData",
            &mut self.store,
        )
    }
    pub fn set_input_model_data(&mut self, data: &[u8]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            data,
            "nanoemApplicationPluginMotionIOSetInputActiveModelData",
            &mut self.store,
        )
    }
    pub fn set_input_motion_data(&mut self, bytes: &[u8]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            bytes,
            "nanoemApplicationPluginMotionIOSetInputMotionData",
            &mut self.store,
        )
    }
    pub fn execute(&mut self) -> Result<i32> {
        inner_execute(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginMotionIOExecute",
            &mut self.store,
        )
    }
    pub fn get_output_data(&mut self) -> Result<Vec<u8>> {
        inner_get_data(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginMotionIOGetOutputMotionData",
            "nanoemApplicationPluginMotionIOGetOutputMotionDataSize",
            &mut self.store,
        )
    }
    pub fn load_ui_window_layout(&mut self) -> Result<i32> {
        inner_load_ui_window(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginMotionIOLoadUIWindowLayout",
            &mut self.store,
        )
    }
    pub fn get_ui_window_layout(&mut self) -> Result<Vec<u8>> {
        inner_get_data(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginMotionIOGetUIWindowLayoutData",
            "nanoemApplicationPluginMotionIOGetUIWindowLayoutDataSize",
            &mut self.store,
        )
    }
    pub fn set_ui_component_layout(
        &mut self,
        id: &str,
        data: &[u8],
        reload: &mut bool,
    ) -> Result<i32> {
        inner_set_ui_component_layout(
            &self.instance,
            &self.opaque,
            id,
            data,
            "nanoemApplicationPluginMotionIOSetUIComponentLayoutData",
            reload,
            &mut self.store,
        )
    }
    pub fn failure_reason(&mut self) -> Result<String> {
        inner_get_string(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginMotionIOGetFailureReason",
            &mut self.store,
        )
    }
    pub fn recovery_suggestion(&mut self) -> Result<String> {
        inner_get_string(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginMotionIOGetRecoverySuggestion",
            &mut self.store,
        )
    }
    pub fn destroy(&mut self) {
        inner_destroy_opaque(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginMotionIODestroy",
            &mut self.store,
        )
    }
    pub fn terminate(&mut self) {
        inner_terminate_function(
            &self.instance,
            "nanoemApplicationPluginMotionIOTerminate",
            &mut self.store,
        )
    }
    #[allow(dead_code)]
    pub(super) fn wasi_env(&mut self) -> &mut WasiEnv {
        self.env.env.as_mut(&mut self.store)
    }
}

impl Drop for MotionIOPlugin {
    fn drop(&mut self) {
        self.env
            .cleanup(&mut self.store, Some(Errno::Success.into()));
    }
}

pub struct MotionIOPluginController {
    plugins: Vec<MotionIOPlugin>,
    function_indices: Vec<(usize, i32, CString)>,
    plugin_index: Option<usize>,
    failure_reason: Option<String>,
    recovery_suggestion: Option<String>,
}

impl MotionIOPluginController {
    pub fn new(plugins: Vec<MotionIOPlugin>) -> Self {
        let function_indices = vec![];
        Self {
            plugins,
            function_indices,
            plugin_index: None,
            failure_reason: None,
            recovery_suggestion: None,
        }
    }
    pub fn from_path<F>(path: &Path, builder_callback: F) -> Result<Self>
    where
        F: Fn(&mut WasiEnvBuilder),
    {
        let mut plugins = vec![];
        for entry in WalkDir::new(path.parent().unwrap()) {
            let entry = entry?;
            let filename = entry.file_name().to_str();
            if filename.map(|s| s.ends_with(".wasm")).unwrap_or(false) {
                let bytes = std::fs::read(entry.path())?;
                let mut store = Store::default();
                let mut builder = WasiEnvBuilder::new("nanoem");
                builder_callback(&mut builder);
                let env = builder.finalize(&mut store)?;
                match MotionIOPlugin::new(&bytes, env, store) {
                    Ok(plugin) => {
                        plugins.push(plugin);
                        tracing::debug!(filename = filename.unwrap(), "Loaded motion WASM plugin");
                    }
                    Err(err) => {
                        warn!(
                            "Cannot load motion WASM plugin {}: {}",
                            filename.unwrap(),
                            err
                        )
                    }
                }
            }
        }
        Ok(Self::new(plugins))
    }
    pub fn initialize(&mut self) -> Result<()> {
        self.plugins
            .iter_mut()
            .try_for_each(|plugin| plugin.initialize())
    }
    pub fn create(&mut self) -> Result<()> {
        self.plugins
            .iter_mut()
            .try_for_each(|plugin| plugin.create())?;
        for (offset, plugin) in self.plugins.iter_mut().enumerate() {
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
        if let Some((plugin_index, function_index, _)) = self.function_indices.get(index as usize) {
            match self.plugins[*plugin_index].set_function(*function_index) {
                Ok(0) => {
                    self.plugin_index = Some(*plugin_index);
                    Ok(())
                }
                Ok(_) => self.set_failure(),
                Err(err) => Err(err),
            }
        } else {
            Err(anyhow::anyhow!("out of bound function index: {}", index))
        }
    }
    pub fn set_all_selected_accessory_keyframes(&mut self, value: &[u32]) -> Result<()> {
        self.current_plugin()?
            .set_all_selected_accessory_keyframes(value)
    }
    pub fn set_all_named_selected_bone_keyframes(
        &mut self,
        name: &str,
        value: &[u32],
    ) -> Result<()> {
        self.current_plugin()?
            .set_all_named_selected_bone_keyframes(name, value)
    }
    pub fn set_all_selected_camera_keyframes(&mut self, value: &[u32]) -> Result<()> {
        self.current_plugin()?
            .set_all_selected_camera_keyframes(value)
    }
    pub fn set_all_selected_light_keyframes(&mut self, value: &[u32]) -> Result<()> {
        self.current_plugin()?
            .set_all_selected_light_keyframes(value)
    }
    pub fn set_all_selected_model_keyframes(&mut self, value: &[u32]) -> Result<()> {
        self.current_plugin()?
            .set_all_selected_model_keyframes(value)
    }
    pub fn set_all_named_selected_morph_keyframes(
        &mut self,
        name: &str,
        value: &[u32],
    ) -> Result<()> {
        self.current_plugin()?
            .set_all_named_selected_morph_keyframes(name, value)
    }
    pub fn set_all_selected_self_shadow_keyframes(&mut self, value: &[u32]) -> Result<()> {
        self.current_plugin()?
            .set_all_selected_self_shadow_keyframes(value)
    }
    pub fn set_audio_description(&mut self, data: &[u8]) -> Result<()> {
        self.current_plugin()?.set_audio_description(data)
    }
    pub fn set_camera_description(&mut self, data: &[u8]) -> Result<()> {
        self.current_plugin()?.set_camera_description(data)
    }
    pub fn set_light_description(&mut self, data: &[u8]) -> Result<()> {
        self.current_plugin()?.set_light_description(data)
    }
    pub fn set_audio_data(&mut self, data: &[u8]) -> Result<()> {
        self.current_plugin()?.set_audio_data(data)
    }
    pub fn set_input_model_data(&mut self, bytes: &[u8]) -> Result<()> {
        self.current_plugin()?.set_input_model_data(bytes)
    }
    pub fn set_input_motion_data(&mut self, bytes: &[u8]) -> Result<()> {
        self.current_plugin()?.set_input_motion_data(bytes)
    }
    pub fn execute(&mut self) -> Result<()> {
        match self.current_plugin()?.execute() {
            Ok(0) => Ok(()),
            Ok(_) => self.set_failure(),
            Err(err) => Err(err),
        }
    }
    pub fn get_output_data(&mut self) -> Result<Vec<u8>> {
        self.current_plugin()?.get_output_data()
    }
    pub fn load_ui_window_layout(&mut self) -> Result<()> {
        match self.current_plugin()?.load_ui_window_layout() {
            Ok(0) => Ok(()),
            Ok(_) => self.set_failure(),
            Err(err) => Err(err),
        }
    }
    pub fn get_ui_window_layout(&mut self) -> Result<Vec<u8>> {
        self.current_plugin()?.get_ui_window_layout()
    }
    pub fn set_ui_component_layout(
        &mut self,
        id: &str,
        data: &[u8],
        reload: &mut bool,
    ) -> Result<()> {
        match self
            .current_plugin()?
            .set_ui_component_layout(id, data, reload)
        {
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
        self.plugins.iter_mut().for_each(|plugin| plugin.destroy())
    }
    pub fn terminate(&mut self) {
        self.plugins
            .iter_mut()
            .for_each(|plugin| plugin.terminate())
    }
    #[allow(unused)]
    pub(super) fn all_plugins_mut(&mut self) -> &mut [MotionIOPlugin] {
        &mut self.plugins
    }
    fn set_failure(&mut self) -> Result<()> {
        let value = self.current_plugin()?.failure_reason()?;
        if !value.is_empty() {
            self.failure_reason = Some(value);
        }
        let value = self.current_plugin()?.recovery_suggestion()?;
        if !value.is_empty() {
            self.recovery_suggestion = Some(value);
        }
        Ok(())
    }
    fn current_plugin(&mut self) -> Result<&mut MotionIOPlugin> {
        if let Some(plugin_index) = self.plugin_index {
            Ok(&mut self.plugins[plugin_index])
        } else {
            Err(anyhow::anyhow!("plugin is not set"))
        }
    }
}
