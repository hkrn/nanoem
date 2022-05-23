/*
  Copyright (c) 2015-2021 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

use std::{ffi::CString, mem::size_of, path::Path, slice};

use anyhow::Result;
use tracing::warn;
use walkdir::WalkDir;
use wasmer::{Instance, Module, Store};
use wasmer_wasi::WasiEnv;

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
    opaque: Option<OpaquePtr>,
}

fn inner_set_named_data(
    instance: &Instance,
    opaque: &Option<OpaquePtr>,
    name: &str,
    data: &[u32],
    func: &str,
) -> Result<()> {
    if let Some(opaque) = opaque {
        if let Ok(set_input_model_data) =
            instance
                .exports
                .get_native_function::<(OpaquePtr, ByteArray, ByteArray, u32, StatusPtr), ()>(func)
        {
            let component_size = size_of::<u32>();
            let data_size = data.len();
            let len = data_size * component_size;
            let data = unsafe { slice::from_raw_parts(data.as_ptr() as *const u8, len) };
            let mut name = name.as_bytes().to_vec();
            name.push(0);
            let name_ptr = allocate_byte_array_with_data(instance, name.as_slice())?;
            let data_ptr = allocate_byte_array_with_data(instance, data)?;
            let status_ptr = allocate_status_ptr(instance)?;
            set_input_model_data.call(*opaque, name_ptr, data_ptr, data_size as u32, status_ptr)?;
            release_byte_array(instance, name_ptr)?;
            release_byte_array(instance, data_ptr)?;
            release_status_ptr(instance, status_ptr)?;
        }
    }
    Ok(())
}

fn validate_plugin(instance: &Instance) -> Result<()> {
    let e = &instance.exports;
    e.get_memory("memory")?;
    e.get_native_function::<u32, OpaquePtr>(MALLOC_FN)?;
    e.get_native_function::<OpaquePtr, ()>(FREE_FN)?;
    e.get_native_function::<(), OpaquePtr>("nanoemApplicationPluginMotionIOCreate")?;
    e.get_native_function::<OpaquePtr, ByteArray>("nanoemApplicationPluginMotionIOGetName")?;
    e.get_native_function::<OpaquePtr, ByteArray>("nanoemApplicationPluginMotionIOGetVersion")?;
    e.get_native_function::<(OpaquePtr, i32), ()>("nanoemApplicationPluginMotionIOSetLanguage")?;
    e.get_native_function::<OpaquePtr, i32>("nanoemApplicationPluginMotionIOCountAllFunctions")?;
    e.get_native_function::<(OpaquePtr, i32), ByteArray>(
        "nanoemApplicationPluginMotionIOGetFunctionName",
    )?;
    e.get_native_function::<(OpaquePtr, i32, StatusPtr), ()>(
        "nanoemApplicationPluginMotionIOSetFunction",
    )?;
    e.get_native_function::<(OpaquePtr, ByteArray, u32, StatusPtr), ()>(
        "nanoemApplicationPluginMotionIOSetInputMotionData",
    )?;
    e.get_native_function::<(OpaquePtr, StatusPtr), ()>("nanoemApplicationPluginMotionIOExecute")?;
    e.get_native_function::<(OpaquePtr, ByteArray, u32, StatusPtr), ()>(
        "nanoemApplicationPluginMotionIOGetOutputMotionData",
    )?;
    e.get_native_function::<(OpaquePtr, SizePtr), ()>(
        "nanoemApplicationPluginMotionIOGetOutputMotionDataSize",
    )?;
    e.get_native_function::<OpaquePtr, ByteArray>(
        "nanoemApplicationPluginMotionIOGetFailureReason",
    )?;
    e.get_native_function::<OpaquePtr, ()>("nanoemApplicationPluginMotionIODestroy")?;
    Ok(())
}

impl MotionIOPlugin {
    pub fn new(bytes: &[u8], store: &Store, env: &mut WasiEnv) -> Result<Self> {
        let module = Module::new(store, bytes)?;
        let imports = env.import_object(&module)?;
        let instance = Instance::new(&module, &imports)?;
        validate_plugin(&instance)?;
        Ok(Self {
            instance,
            opaque: None,
        })
    }
    pub fn initialize(&self) -> Result<()> {
        inner_initialize_function(&self.instance, "nanoemApplicationPluginMotionIOInitialize")
    }
    pub fn create(&mut self) -> Result<()> {
        self.opaque = Some(inner_create_opaque(
            &self.instance,
            "nanoemApplicationPluginMotionIOCreate",
        )?);
        Ok(())
    }
    pub fn set_language(&self, value: i32) -> Result<()> {
        inner_set_language(
            &self.instance,
            &self.opaque,
            value,
            "nanoemApplicationPluginMotionIOSetLanguage",
        )
    }
    pub fn name(&self) -> Result<String> {
        inner_get_string(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginMotionIOGetName",
        )
    }
    #[allow(unused)]
    pub fn description(&self) -> Result<String> {
        inner_get_string(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginMotionIOGetDescription",
        )
    }
    #[allow(unused)]
    pub fn version(&self) -> Result<String> {
        inner_get_string(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginMotionIOGetVersion",
        )
    }
    pub fn count_all_functions(&self) -> Result<i32> {
        inner_count_all_functions(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginMotionIOCountAllFunctions",
        )
    }
    pub fn function_name(&self, index: i32) -> Result<String> {
        inner_get_function_name(
            &self.instance,
            &self.opaque,
            index,
            "nanoemApplicationPluginMotionIOGetFunctionName",
        )
    }
    pub fn set_function(&self, index: i32) -> Result<()> {
        inner_set_function(
            &self.instance,
            &self.opaque,
            index,
            "nanoemApplicationPluginMotionIOSetFunction",
        )
    }
    pub fn set_all_selected_accessory_keyframes(&self, value: &[u32]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            value,
            "nanoemApplicationPluginMotionIOSetAllSelectedAccessoryKeyframes",
        )
    }
    pub fn set_all_named_selected_bone_keyframes(&self, name: &str, value: &[u32]) -> Result<()> {
        inner_set_named_data(
            &self.instance,
            &self.opaque,
            name,
            value,
            "nanoemApplicationPluginMotionIOSetAllNamedSelectedBoneKeyframes",
        )
    }
    pub fn set_all_selected_camera_keyframes(&self, value: &[u32]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            value,
            "nanoemApplicationPluginMotionIOSetAllSelectedCameraKeyframes",
        )
    }
    pub fn set_all_selected_light_keyframes(&self, value: &[u32]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            value,
            "nanoemApplicationPluginMotionIOSetAllSelectedLightKeyframes",
        )
    }
    pub fn set_all_selected_model_keyframes(&self, value: &[u32]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            value,
            "nanoemApplicationPluginMotionIOSetAllSelectedModelKeyframes",
        )
    }
    pub fn set_all_named_selected_morph_keyframes(&self, name: &str, value: &[u32]) -> Result<()> {
        inner_set_named_data(
            &self.instance,
            &self.opaque,
            name,
            value,
            "nanoemApplicationPluginMotionIOSetAllNamedSelectedMorphKeyframes",
        )
    }
    pub fn set_all_selected_self_shadow_keyframes(&self, value: &[u32]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            value,
            "nanoemApplicationPluginMotionIOSetAllSelectedSelfShadowKeyframes",
        )
    }
    pub fn set_audio_description(&self, data: &[u8]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            data,
            "nanoemApplicationPluginMotionIOSetAudioDescription",
        )
    }
    pub fn set_camera_description(&self, data: &[u8]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            data,
            "nanoemApplicationPluginMotionIOSetCameraDescription",
        )
    }
    pub fn set_light_description(&self, data: &[u8]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            data,
            "nanoemApplicationPluginMotionIOSetLightDescription",
        )
    }
    pub fn set_audio_data(&self, data: &[u8]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            data,
            "nanoemApplicationPluginMotionIOSetInputAudioData",
        )
    }
    pub fn set_input_model_data(&self, data: &[u8]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            data,
            "nanoemApplicationPluginMotionIOSetInputActiveModelData",
        )
    }
    pub fn set_input_motion_data(&self, bytes: &[u8]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            bytes,
            "nanoemApplicationPluginMotionIOSetInputMotionData",
        )
    }
    pub fn execute(&self) -> Result<()> {
        inner_execute(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginMotionIOExecute",
        )
    }
    pub fn get_output_data(&self) -> Result<Vec<u8>> {
        inner_get_data(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginMotionIOGetOutputMotionData",
            "nanoemApplicationPluginMotionIOGetOutputMotionDataSize",
        )
    }
    pub fn load_ui_window_layout(&self) -> Result<()> {
        inner_load_ui_window(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginMotionIOLoadUIWindowLayout",
        )
    }
    pub fn get_ui_window_layout(&self) -> Result<Vec<u8>> {
        inner_get_data(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginMotionIOGetUIWindowLayoutData",
            "nanoemApplicationPluginMotionIOGetUIWindowLayoutDataSize",
        )
    }
    pub fn set_ui_component_layout(&self, id: &str, data: &[u8], reload: &mut bool) -> Result<()> {
        inner_set_ui_component_layout(
            &self.instance,
            &self.opaque,
            id,
            data,
            "nanoemApplicationPluginMotionIOSetUIComponentLayoutData",
            reload,
        )
    }
    pub fn failure_reason(&self) -> Result<String> {
        inner_get_string(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginMotionIOGetFailureReason",
        )
    }
    pub fn recovery_suggestion(&self) -> Result<String> {
        inner_get_string(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginMotionIOGetRecoverySuggestion",
        )
    }
    pub fn destroy(&self) {
        inner_destroy_opaque(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginMotionIODestroy",
        )
    }
    pub fn terminate(&self) {
        inner_terminate_function(&self.instance, "nanoemApplicationPluginMotionIOTerminate")
    }
}

pub struct MotionIOPluginController {
    plugins: Vec<MotionIOPlugin>,
    function_indices: Vec<(usize, i32, CString)>,
    plugin_index: Option<usize>,
}

impl MotionIOPluginController {
    pub fn new(plugins: Vec<MotionIOPlugin>) -> Self {
        let function_indices = vec![];
        Self {
            plugins,
            function_indices,
            plugin_index: None,
        }
    }
    pub fn from_path(path: &Path, store: &Store, env: &mut WasiEnv) -> Result<Self> {
        let mut plugins = vec![];
        for entry in WalkDir::new(path.parent().unwrap()) {
            let entry = entry?;
            let filename = entry.file_name().to_str();
            if filename.map(|s| s.ends_with(".wasm")).unwrap_or(false) {
                let bytes = std::fs::read(entry.path())?;
                match MotionIOPlugin::new(&bytes, store, env) {
                    Ok(plugin) => plugins.push(plugin),
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
    pub fn initialize(&self) -> Result<()> {
        self.plugins
            .iter()
            .try_for_each(|plugin| plugin.initialize())
    }
    pub fn create(&mut self) -> Result<()> {
        self.plugins
            .iter_mut()
            .try_for_each(|plugin| plugin.create())?;
        for (offset, plugin) in self.plugins.iter().enumerate() {
            for index in 0..plugin.count_all_functions()? {
                let name = CString::new(
                    &format!(
                        "{}: {} ({})",
                        plugin.name()?,
                        plugin.function_name(index)?,
                        plugin.version()?
                    )[..],
                )?;
                self.function_indices.push((offset, index, name));
            }
        }
        Ok(())
    }
    pub fn set_language(&self, value: i32) -> Result<()> {
        self.plugins
            .iter()
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
            let result = self.plugins[*plugin_index].set_function(*function_index);
            self.plugin_index = Some(*plugin_index);
            result
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
    pub fn set_audio_description(&self, data: &[u8]) -> Result<()> {
        self.current_plugin()?.set_audio_description(data)
    }
    pub fn set_camera_description(&self, data: &[u8]) -> Result<()> {
        self.current_plugin()?.set_camera_description(data)
    }
    pub fn set_light_description(&self, data: &[u8]) -> Result<()> {
        self.current_plugin()?.set_light_description(data)
    }
    pub fn set_audio_data(&self, data: &[u8]) -> Result<()> {
        self.current_plugin()?.set_audio_data(data)
    }
    pub fn set_input_model_data(&self, bytes: &[u8]) -> Result<()> {
        self.current_plugin()?.set_input_model_data(bytes)
    }
    pub fn set_input_motion_data(&self, bytes: &[u8]) -> Result<()> {
        self.current_plugin()?.set_input_motion_data(bytes)
    }
    pub fn execute(&self) -> Result<()> {
        self.current_plugin()?.execute()
    }
    pub fn get_output_data(&self) -> Result<Vec<u8>> {
        self.current_plugin()?.get_output_data()
    }
    pub fn load_ui_window_layout(&self) -> Result<()> {
        self.current_plugin()?.load_ui_window_layout()
    }
    pub fn get_ui_window_layout(&self) -> Result<Vec<u8>> {
        self.current_plugin()?.get_ui_window_layout()
    }
    pub fn set_ui_component_layout(&self, id: &str, data: &[u8], reload: &mut bool) -> Result<()> {
        self.current_plugin()?
            .set_ui_component_layout(id, data, reload)
    }
    pub fn failure_reason(&self) -> Result<String> {
        self.current_plugin()?.failure_reason()
    }
    pub fn recovery_suggestion(&self) -> Result<String> {
        self.current_plugin()?.recovery_suggestion()
    }
    pub fn destroy(&self) {
        self.plugins.iter().for_each(|plugin| plugin.destroy())
    }
    pub fn terminate(&self) {
        self.plugins.iter().for_each(|plugin| plugin.terminate())
    }
    #[allow(unused)]
    pub(super) fn all_plugins_mut(&mut self) -> &mut [MotionIOPlugin] {
        &mut self.plugins
    }
    fn current_plugin(&self) -> Result<&MotionIOPlugin> {
        if let Some(plugin_index) = self.plugin_index {
            Ok(&self.plugins[plugin_index])
        } else {
            Err(anyhow::anyhow!("plugin is not set"))
        }
    }
}
