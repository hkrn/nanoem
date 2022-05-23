/*
  Copyright (c) 2015-2021 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

use std::{ffi::CString, path::Path};

use anyhow::Result;
use tracing::warn;
use walkdir::WalkDir;
use wasmer::{Instance, Module, Store};
use wasmer_wasi::WasiEnv;

use crate::{
    inner_count_all_functions, inner_create_opaque, inner_destroy_opaque, inner_execute,
    inner_get_data, inner_get_function_name, inner_get_string, inner_initialize_function,
    inner_load_ui_window, inner_set_data, inner_set_function, inner_set_language,
    inner_set_ui_component_layout, inner_terminate_function, ByteArray, OpaquePtr, SizePtr,
    StatusPtr, FREE_FN, MALLOC_FN,
};

fn validate_plugin(instance: &Instance) -> Result<()> {
    let e = &instance.exports;
    e.get_memory("memory")?;
    e.get_native_function::<u32, OpaquePtr>(MALLOC_FN)?;
    e.get_native_function::<OpaquePtr, ()>(FREE_FN)?;
    e.get_native_function::<(), OpaquePtr>("nanoemApplicationPluginModelIOCreate")?;
    e.get_native_function::<OpaquePtr, ByteArray>("nanoemApplicationPluginModelIOGetName")?;
    e.get_native_function::<OpaquePtr, ByteArray>("nanoemApplicationPluginModelIOGetVersion")?;
    e.get_native_function::<(OpaquePtr, i32), ()>("nanoemApplicationPluginModelIOSetLanguage")?;
    e.get_native_function::<OpaquePtr, i32>("nanoemApplicationPluginModelIOCountAllFunctions")?;
    e.get_native_function::<(OpaquePtr, i32), ByteArray>(
        "nanoemApplicationPluginModelIOGetFunctionName",
    )?;
    e.get_native_function::<(OpaquePtr, i32, StatusPtr), ()>(
        "nanoemApplicationPluginModelIOSetFunction",
    )?;
    e.get_native_function::<(OpaquePtr, ByteArray, u32, StatusPtr), ()>(
        "nanoemApplicationPluginModelIOSetInputModelData",
    )?;
    e.get_native_function::<(OpaquePtr, StatusPtr), ()>("nanoemApplicationPluginModelIOExecute")?;
    e.get_native_function::<(OpaquePtr, ByteArray, u32, StatusPtr), ()>(
        "nanoemApplicationPluginModelIOGetOutputModelData",
    )?;
    e.get_native_function::<(OpaquePtr, SizePtr), ()>(
        "nanoemApplicationPluginModelIOGetOutputModelDataSize",
    )?;
    e.get_native_function::<OpaquePtr, ByteArray>(
        "nanoemApplicationPluginModelIOGetFailureReason",
    )?;
    e.get_native_function::<OpaquePtr, ()>("nanoemApplicationPluginModelIODestroy")?;
    Ok(())
}

pub struct ModelIOPlugin {
    instance: Instance,
    opaque: Option<OpaquePtr>,
}

impl ModelIOPlugin {
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
        inner_initialize_function(&self.instance, "nanoemApplicationPluginModelIOInitialize")
    }
    pub fn create(&mut self) -> Result<()> {
        self.opaque = Some(inner_create_opaque(
            &self.instance,
            "nanoemApplicationPluginModelIOCreate",
        )?);
        Ok(())
    }
    pub fn set_language(&self, value: i32) -> Result<()> {
        inner_set_language(
            &self.instance,
            &self.opaque,
            value,
            "nanoemApplicationPluginModelIOSetLanguage",
        )
    }
    pub fn name(&self) -> Result<String> {
        inner_get_string(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginModelIOGetName",
        )
    }
    #[allow(unused)]
    pub fn description(&self) -> Result<String> {
        inner_get_string(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginModelIOGetDescription",
        )
    }
    pub fn version(&self) -> Result<String> {
        inner_get_string(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginModelIOGetVersion",
        )
    }
    pub fn count_all_functions(&self) -> Result<i32> {
        inner_count_all_functions(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginModelIOCountAllFunctions",
        )
    }
    pub fn function_name(&self, index: i32) -> Result<String> {
        inner_get_function_name(
            &self.instance,
            &self.opaque,
            index,
            "nanoemApplicationPluginModelIOGetFunctionName",
        )
    }
    pub fn set_function(&self, index: i32) -> Result<()> {
        inner_set_function(
            &self.instance,
            &self.opaque,
            index,
            "nanoemApplicationPluginModelIOSetFunction",
        )
    }
    pub fn set_all_selected_vertex_indices(&self, data: &[i32]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            data,
            "nanoemApplicationPluginModelIOSetAllSelectedVertexObjectIndices",
        )
    }
    pub fn set_all_selected_material_indices(&self, data: &[i32]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            data,
            "nanoemApplicationPluginModelIOSetAllSelectedMaterialObjectIndices",
        )
    }
    pub fn set_all_selected_bone_indices(&self, data: &[i32]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            data,
            "nanoemApplicationPluginModelIOSetAllSelectedBoneObjectIndices",
        )
    }
    pub fn set_all_selected_morph_indices(&self, data: &[i32]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            data,
            "nanoemApplicationPluginModelIOSetAllSelectedMorphObjectIndices",
        )
    }
    pub fn set_all_selected_label_indices(&self, data: &[i32]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            data,
            "nanoemApplicationPluginModelIOSetAllSelectedLabelObjectIndices",
        )
    }
    pub fn set_all_selected_rigid_body_indices(&self, data: &[i32]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            data,
            "nanoemApplicationPluginModelIOSetAllSelectedRigidBodyObjectIndices",
        )
    }
    pub fn set_all_selected_joint_indices(&self, data: &[i32]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            data,
            "nanoemApplicationPluginModelIOSetAllSelectedJointObjectIndices",
        )
    }
    pub fn set_all_selected_soft_body_indices(&self, data: &[i32]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            data,
            "nanoemApplicationPluginModelIOSetAllSelectedSoftBodyObjectIndices",
        )
    }
    pub fn set_audio_description(&self, data: &[u8]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            data,
            "nanoemApplicationPluginModelIOSetAudioDescription",
        )
    }
    pub fn set_camera_description(&self, data: &[u8]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            data,
            "nanoemApplicationPluginModelIOSetCameraDescription",
        )
    }
    pub fn set_light_description(&self, data: &[u8]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            data,
            "nanoemApplicationPluginModelIOSetLightDescription",
        )
    }
    pub fn set_audio_data(&self, data: &[u8]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            data,
            "nanoemApplicationPluginModelIOSetInputAudioData",
        )
    }
    pub fn set_input_model_data(&self, data: &[u8]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            data,
            "nanoemApplicationPluginModelIOSetInputModelData",
        )
    }
    pub fn execute(&self) -> Result<()> {
        inner_execute(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginModelIOExecute",
        )
    }
    pub fn get_output_data(&self) -> Result<Vec<u8>> {
        inner_get_data(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginModelIOGetOutputModelData",
            "nanoemApplicationPluginModelIOGetOutputModelDataSize",
        )
    }
    pub fn load_ui_window_layout(&self) -> Result<()> {
        inner_load_ui_window(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginModelIOLoadUIWindowLayout",
        )
    }
    pub fn get_ui_window_layout(&self) -> Result<Vec<u8>> {
        inner_get_data(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginModelIOGetUIWindowLayoutData",
            "nanoemApplicationPluginModelIOGetUIWindowLayoutDataSize",
        )
    }
    pub fn set_ui_component_layout(&self, id: &str, data: &[u8], reload: &mut bool) -> Result<()> {
        inner_set_ui_component_layout(
            &self.instance,
            &self.opaque,
            id,
            data,
            "nanoemApplicationPluginModelIOSetUIComponentLayoutData",
            reload,
        )
    }
    pub fn failure_reason(&self) -> Result<String> {
        inner_get_string(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginModelIOGetFailureReason",
        )
    }
    pub fn recovery_suggestion(&self) -> Result<String> {
        inner_get_string(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginModelIOGetRecoverySuggestion",
        )
    }
    pub fn destroy(&self) {
        inner_destroy_opaque(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginModelIODestroy",
        )
    }
    pub fn terminate(&self) {
        inner_terminate_function(&self.instance, "nanoemApplicationPluginModelIOTerminate")
    }
}

pub struct ModelIOPluginController {
    plugins: Vec<ModelIOPlugin>,
    function_indices: Vec<(usize, i32, CString)>,
    plugin_index: Option<usize>,
}

impl ModelIOPluginController {
    pub fn new(plugins: Vec<ModelIOPlugin>) -> Self {
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
                match ModelIOPlugin::new(&bytes, store, env) {
                    Ok(plugin) => plugins.push(plugin),
                    Err(err) => {
                        warn!(
                            "Cannot load model WASM plugin {}: {}",
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
    pub fn set_all_selected_vertex_indices(&self, data: &[i32]) -> Result<()> {
        self.current_plugin()?.set_all_selected_vertex_indices(data)
    }
    pub fn set_all_selected_material_indices(&self, data: &[i32]) -> Result<()> {
        self.current_plugin()?
            .set_all_selected_material_indices(data)
    }
    pub fn set_all_selected_bone_indices(&self, data: &[i32]) -> Result<()> {
        self.current_plugin()?.set_all_selected_bone_indices(data)
    }
    pub fn set_all_selected_morph_indices(&self, data: &[i32]) -> Result<()> {
        self.current_plugin()?.set_all_selected_morph_indices(data)
    }
    pub fn set_all_selected_label_indices(&self, data: &[i32]) -> Result<()> {
        self.current_plugin()?.set_all_selected_label_indices(data)
    }
    pub fn set_all_selected_rigid_body_indices(&self, data: &[i32]) -> Result<()> {
        self.current_plugin()?
            .set_all_selected_rigid_body_indices(data)
    }
    pub fn set_all_selected_joint_indices(&self, data: &[i32]) -> Result<()> {
        self.current_plugin()?.set_all_selected_joint_indices(data)
    }
    pub fn set_all_selected_soft_body_indices(&self, data: &[i32]) -> Result<()> {
        self.current_plugin()?
            .set_all_selected_soft_body_indices(data)
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
    pub(super) fn all_plugins_mut(&mut self) -> &mut [ModelIOPlugin] {
        &mut self.plugins
    }
    fn current_plugin(&self) -> Result<&ModelIOPlugin> {
        if let Some(plugin_index) = self.plugin_index {
            Ok(&self.plugins[plugin_index])
        } else {
            Err(anyhow::anyhow!("plugin is not set"))
        }
    }
}
