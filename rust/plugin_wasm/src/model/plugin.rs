/*
  Copyright (c) 2015-2023 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

use std::path::{Path, PathBuf};

use anyhow::Result;
use wasmtime::{AsContextMut, Instance, Linker, Module};
use wasmtime_wasi::WasiCtx;

use crate::{
    inner_count_all_functions, inner_create_opaque, inner_destroy_opaque, inner_execute,
    inner_get_data, inner_get_function_name, inner_get_string, inner_initialize_function,
    inner_load_ui_window, inner_set_data, inner_set_function, inner_set_language,
    inner_set_ui_component_layout, inner_terminate_function, ByteArray, OpaquePtr, SizePtr,
    StatusPtr, Store, FREE_FN, MALLOC_FN,
};

fn validate_plugin(instance: &Instance, mut store: impl AsContextMut) -> Result<()> {
    instance
        .get_memory(store.as_context_mut(), "memory")
        .unwrap();
    instance.get_typed_func::<u32, OpaquePtr>(store.as_context_mut(), MALLOC_FN)?;
    instance.get_typed_func::<OpaquePtr, ()>(store.as_context_mut(), FREE_FN)?;
    instance.get_typed_func::<(), OpaquePtr>(
        store.as_context_mut(),
        "nanoemApplicationPluginModelIOCreate",
    )?;
    instance.get_typed_func::<OpaquePtr, ByteArray>(
        store.as_context_mut(),
        "nanoemApplicationPluginModelIOGetName",
    )?;
    instance.get_typed_func::<OpaquePtr, ByteArray>(
        store.as_context_mut(),
        "nanoemApplicationPluginModelIOGetVersion",
    )?;
    instance.get_typed_func::<(OpaquePtr, i32), ()>(
        store.as_context_mut(),
        "nanoemApplicationPluginModelIOSetLanguage",
    )?;
    instance.get_typed_func::<OpaquePtr, i32>(
        store.as_context_mut(),
        "nanoemApplicationPluginModelIOCountAllFunctions",
    )?;
    instance.get_typed_func::<(OpaquePtr, i32), ByteArray>(
        store.as_context_mut(),
        "nanoemApplicationPluginModelIOGetFunctionName",
    )?;
    instance.get_typed_func::<(OpaquePtr, i32, StatusPtr), ()>(
        store.as_context_mut(),
        "nanoemApplicationPluginModelIOSetFunction",
    )?;
    instance.get_typed_func::<(OpaquePtr, ByteArray, u32, StatusPtr), ()>(
        store.as_context_mut(),
        "nanoemApplicationPluginModelIOSetInputModelData",
    )?;
    instance.get_typed_func::<(OpaquePtr, StatusPtr), ()>(
        store.as_context_mut(),
        "nanoemApplicationPluginModelIOExecute",
    )?;
    instance.get_typed_func::<(OpaquePtr, ByteArray, u32, StatusPtr), ()>(
        store.as_context_mut(),
        "nanoemApplicationPluginModelIOGetOutputModelData",
    )?;
    instance.get_typed_func::<(OpaquePtr, SizePtr), ()>(
        store.as_context_mut(),
        "nanoemApplicationPluginModelIOGetOutputModelDataSize",
    )?;
    instance.get_typed_func::<OpaquePtr, ByteArray>(
        store.as_context_mut(),
        "nanoemApplicationPluginModelIOGetFailureReason",
    )?;
    instance.get_typed_func::<OpaquePtr, ()>(
        store.as_context_mut(),
        "nanoemApplicationPluginModelIODestroy",
    )?;
    Ok(())
}

pub struct ModelIOPlugin {
    instance: Instance,
    store: Store,
    path: PathBuf,
    opaque: Option<OpaquePtr>,
}

impl ModelIOPlugin {
    pub fn new(
        linker: &Linker<WasiCtx>,
        path: &Path,
        bytes: &[u8],
        mut store: Store,
    ) -> Result<Self> {
        let module = Module::new(linker.engine(), bytes)?;
        let instance = linker.instantiate(store.as_context_mut(), &module)?;
        validate_plugin(&instance, store.as_context_mut())?;
        let path = path.to_path_buf();
        Ok(Self {
            instance,
            store,
            path,
            opaque: None,
        })
    }
    pub fn initialize(&mut self) -> Result<()> {
        inner_initialize_function(
            &self.instance,
            "nanoemApplicationPluginModelIOInitialize",
            &mut self.store,
        )
    }
    pub fn create(&mut self) -> Result<()> {
        self.opaque = Some(inner_create_opaque(
            &self.instance,
            "nanoemApplicationPluginModelIOCreate",
            &mut self.store,
        )?);
        Ok(())
    }
    pub fn set_language(&mut self, value: i32) -> Result<()> {
        inner_set_language(
            &self.instance,
            &self.opaque,
            value,
            "nanoemApplicationPluginModelIOSetLanguage",
            &mut self.store,
        )
    }
    pub fn name(&mut self) -> Result<String> {
        inner_get_string(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginModelIOGetName",
            &mut self.store,
        )
    }
    #[allow(unused)]
    pub fn description(&mut self) -> Result<String> {
        inner_get_string(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginModelIOGetDescription",
            &mut self.store,
        )
    }
    pub fn version(&mut self) -> Result<String> {
        inner_get_string(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginModelIOGetVersion",
            &mut self.store,
        )
    }
    pub fn count_all_functions(&mut self) -> Result<i32> {
        inner_count_all_functions(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginModelIOCountAllFunctions",
            &mut self.store,
        )
    }
    pub fn function_name(&mut self, index: i32) -> Result<String> {
        inner_get_function_name(
            &self.instance,
            &self.opaque,
            index,
            "nanoemApplicationPluginModelIOGetFunctionName",
            &mut self.store,
        )
    }
    pub fn set_function(&mut self, index: i32) -> Result<i32> {
        inner_set_function(
            &self.instance,
            &self.opaque,
            index,
            "nanoemApplicationPluginModelIOSetFunction",
            &mut self.store,
        )
    }
    pub fn set_all_selected_vertex_indices(&mut self, data: &[i32]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            data,
            "nanoemApplicationPluginModelIOSetAllSelectedVertexObjectIndices",
            &mut self.store,
        )
    }
    pub fn set_all_selected_material_indices(&mut self, data: &[i32]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            data,
            "nanoemApplicationPluginModelIOSetAllSelectedMaterialObjectIndices",
            &mut self.store,
        )
    }
    pub fn set_all_selected_bone_indices(&mut self, data: &[i32]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            data,
            "nanoemApplicationPluginModelIOSetAllSelectedBoneObjectIndices",
            &mut self.store,
        )
    }
    pub fn set_all_selected_morph_indices(&mut self, data: &[i32]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            data,
            "nanoemApplicationPluginModelIOSetAllSelectedMorphObjectIndices",
            &mut self.store,
        )
    }
    pub fn set_all_selected_label_indices(&mut self, data: &[i32]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            data,
            "nanoemApplicationPluginModelIOSetAllSelectedLabelObjectIndices",
            &mut self.store,
        )
    }
    pub fn set_all_selected_rigid_body_indices(&mut self, data: &[i32]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            data,
            "nanoemApplicationPluginModelIOSetAllSelectedRigidBodyObjectIndices",
            &mut self.store,
        )
    }
    pub fn set_all_selected_joint_indices(&mut self, data: &[i32]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            data,
            "nanoemApplicationPluginModelIOSetAllSelectedJointObjectIndices",
            &mut self.store,
        )
    }
    pub fn set_all_selected_soft_body_indices(&mut self, data: &[i32]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            data,
            "nanoemApplicationPluginModelIOSetAllSelectedSoftBodyObjectIndices",
            &mut self.store,
        )
    }
    pub fn set_audio_description(&mut self, data: &[u8]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            data,
            "nanoemApplicationPluginModelIOSetAudioDescription",
            &mut self.store,
        )
    }
    pub fn set_camera_description(&mut self, data: &[u8]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            data,
            "nanoemApplicationPluginModelIOSetCameraDescription",
            &mut self.store,
        )
    }
    pub fn set_light_description(&mut self, data: &[u8]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            data,
            "nanoemApplicationPluginModelIOSetLightDescription",
            &mut self.store,
        )
    }
    pub fn set_audio_data(&mut self, data: &[u8]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            data,
            "nanoemApplicationPluginModelIOSetInputAudioData",
            &mut self.store,
        )
    }
    pub fn set_input_model_data(&mut self, data: &[u8]) -> Result<()> {
        inner_set_data(
            &self.instance,
            &self.opaque,
            data,
            "nanoemApplicationPluginModelIOSetInputModelData",
            &mut self.store,
        )
    }
    pub fn execute(&mut self) -> Result<i32> {
        inner_execute(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginModelIOExecute",
            &mut self.store,
        )
    }
    pub fn get_output_data(&mut self) -> Result<Vec<u8>> {
        inner_get_data(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginModelIOGetOutputModelData",
            "nanoemApplicationPluginModelIOGetOutputModelDataSize",
            &mut self.store,
        )
    }
    pub fn load_ui_window_layout(&mut self) -> Result<i32> {
        inner_load_ui_window(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginModelIOLoadUIWindowLayout",
            &mut self.store,
        )
    }
    pub fn get_ui_window_layout(&mut self) -> Result<Vec<u8>> {
        inner_get_data(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginModelIOGetUIWindowLayoutData",
            "nanoemApplicationPluginModelIOGetUIWindowLayoutDataSize",
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
            "nanoemApplicationPluginModelIOSetUIComponentLayoutData",
            reload,
            &mut self.store,
        )
    }
    pub fn failure_reason(&mut self) -> Result<String> {
        inner_get_string(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginModelIOGetFailureReason",
            &mut self.store,
        )
    }
    pub fn recovery_suggestion(&mut self) -> Result<String> {
        inner_get_string(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginModelIOGetRecoverySuggestion",
            &mut self.store,
        )
    }
    pub fn destroy(&mut self) {
        inner_destroy_opaque(
            &self.instance,
            &self.opaque,
            "nanoemApplicationPluginModelIODestroy",
            &mut self.store,
        )
    }
    pub fn terminate(&mut self) {
        inner_terminate_function(
            &self.instance,
            "nanoemApplicationPluginModelIOTerminate",
            &mut self.store,
        )
    }
    pub fn path(&self) -> &PathBuf {
        &self.path
    }
}
