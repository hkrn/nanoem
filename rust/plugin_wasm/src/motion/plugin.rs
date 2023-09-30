/*
  Copyright (c) 2015-2023 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

use std::{
    path::{Path, PathBuf},
    slice,
};

use anyhow::Result;
use wasmtime::{AsContextMut, Instance, Linker, Module};
use wasmtime_wasi::WasiCtx;

use crate::{
    allocate_byte_array_with_data, allocate_status_ptr, inner_count_all_functions,
    inner_create_opaque, inner_destroy_opaque, inner_execute, inner_get_data,
    inner_get_function_name, inner_get_string, inner_initialize_function, inner_load_ui_window,
    inner_set_data, inner_set_function, inner_set_language, inner_set_ui_component_layout,
    inner_terminate_function, release_byte_array, release_status_ptr, ByteArray, OpaquePtr,
    SizePtr, StatusPtr, Store, FREE_FN, MALLOC_FN,
};

pub struct MotionIOPlugin {
    instance: Instance,
    store: Store,
    path: PathBuf,
    opaque: Option<OpaquePtr>,
}

fn inner_set_named_data(
    instance: &Instance,
    opaque: &Option<OpaquePtr>,
    name: &str,
    data: &[u32],
    func: &str,
    mut store: impl AsContextMut,
) -> Result<()> {
    if let Some(opaque) = opaque {
        if let Ok(set_input_model_data) = instance
            .get_typed_func::<(OpaquePtr, ByteArray, ByteArray, u32, StatusPtr), ()>(
                store.as_context_mut(),
                func,
            )
        {
            let data_size = data.len();
            let len = std::mem::size_of_val(data);
            let data = unsafe { slice::from_raw_parts(data.as_ptr() as *const u8, len) };
            let mut name = name.as_bytes().to_vec();
            name.push(0);
            let name_ptr =
                allocate_byte_array_with_data(instance, name.as_slice(), store.as_context_mut())?;
            let data_ptr = allocate_byte_array_with_data(instance, data, store.as_context_mut())?;
            let status_ptr = allocate_status_ptr(instance, store.as_context_mut())?;
            set_input_model_data.call(
                store.as_context_mut(),
                (*opaque, name_ptr, data_ptr, data_size as u32, status_ptr),
            )?;
            release_byte_array(instance, name_ptr, store.as_context_mut())?;
            release_byte_array(instance, data_ptr, store.as_context_mut())?;
            release_status_ptr(instance, status_ptr, store.as_context_mut())?;
        }
    }
    Ok(())
}

fn validate_plugin(instance: &Instance, mut store: impl AsContextMut) -> Result<()> {
    instance
        .get_memory(store.as_context_mut(), "memory")
        .unwrap();
    instance.get_typed_func::<u32, OpaquePtr>(store.as_context_mut(), MALLOC_FN)?;
    instance.get_typed_func::<OpaquePtr, ()>(store.as_context_mut(), FREE_FN)?;
    instance.get_typed_func::<(), OpaquePtr>(
        store.as_context_mut(),
        "nanoemApplicationPluginMotionIOCreate",
    )?;
    instance.get_typed_func::<OpaquePtr, ByteArray>(
        store.as_context_mut(),
        "nanoemApplicationPluginMotionIOGetName",
    )?;
    instance.get_typed_func::<OpaquePtr, ByteArray>(
        store.as_context_mut(),
        "nanoemApplicationPluginMotionIOGetVersion",
    )?;
    instance.get_typed_func::<(OpaquePtr, i32), ()>(
        store.as_context_mut(),
        "nanoemApplicationPluginMotionIOSetLanguage",
    )?;
    instance.get_typed_func::<OpaquePtr, i32>(
        store.as_context_mut(),
        "nanoemApplicationPluginMotionIOCountAllFunctions",
    )?;
    instance.get_typed_func::<(OpaquePtr, i32), ByteArray>(
        store.as_context_mut(),
        "nanoemApplicationPluginMotionIOGetFunctionName",
    )?;
    instance.get_typed_func::<(OpaquePtr, i32, StatusPtr), ()>(
        store.as_context_mut(),
        "nanoemApplicationPluginMotionIOSetFunction",
    )?;
    instance.get_typed_func::<(OpaquePtr, ByteArray, u32, StatusPtr), ()>(
        store.as_context_mut(),
        "nanoemApplicationPluginMotionIOSetInputMotionData",
    )?;
    instance.get_typed_func::<(OpaquePtr, StatusPtr), ()>(
        store.as_context_mut(),
        "nanoemApplicationPluginMotionIOExecute",
    )?;
    instance.get_typed_func::<(OpaquePtr, ByteArray, u32, StatusPtr), ()>(
        store.as_context_mut(),
        "nanoemApplicationPluginMotionIOGetOutputMotionData",
    )?;
    instance.get_typed_func::<(OpaquePtr, SizePtr), ()>(
        store.as_context_mut(),
        "nanoemApplicationPluginMotionIOGetOutputMotionDataSize",
    )?;
    instance.get_typed_func::<OpaquePtr, ByteArray>(
        store.as_context_mut(),
        "nanoemApplicationPluginMotionIOGetFailureReason",
    )?;
    instance.get_typed_func::<OpaquePtr, ()>(
        store.as_context_mut(),
        "nanoemApplicationPluginMotionIODestroy",
    )?;
    Ok(())
}

impl MotionIOPlugin {
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
            path,
            store,
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
    pub fn path(&self) -> &PathBuf {
        &self.path
    }
}
