/*
  Copyright (c) 2015-2020 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#[allow(non_camel_case_types)]
#[derive(Default)]
pub struct nanoem_wasm_plugin_motion_io_t {
    name: std::ffi::CString,
    accessory_keyframes: Vec<u32>,
    bone_keyframes: std::collections::HashMap<String, Vec<u32>>,
    camera_keyframes: Vec<u32>,
    light_keyframes: Vec<u32>,
    model_keyframes: Vec<u32>,
    morph_keyframes: std::collections::HashMap<String, Vec<u32>>,
    self_shadow_keyframes: Vec<u32>,
    input_model_data: Vec<u8>,
    input_motion_data: Vec<u8>,
    output_motion_data: Vec<u8>,
    language: u32,
}

impl Drop for nanoem_wasm_plugin_motion_io_t {
    fn drop(&mut self) {
        println!("accessory_keyframes = {:?}", self.accessory_keyframes);
        println!("bone_keyframes = {:?}", self.bone_keyframes);
        println!("camera_keyframes = {:?}", self.camera_keyframes);
        println!("light_keyframes = {:?}", self.light_keyframes);
        println!("model_keyframes = {:?}", self.model_keyframes);
        println!("morph_keyframes = {:?}", self.morph_keyframes);
        println!("self_shadow_keyframes = {:?}", self.self_shadow_keyframes);
    }
}

impl nanoem_wasm_plugin_motion_io_t {
    pub fn new() -> Self {
        Self {
            name: std::ffi::CString::new("Passthrough Motion Plugin").unwrap(),
            accessory_keyframes: vec![],
            bone_keyframes: std::collections::HashMap::new(),
            camera_keyframes: vec![],
            light_keyframes: vec![],
            model_keyframes: vec![],
            morph_keyframes: std::collections::HashMap::new(),
            self_shadow_keyframes: vec![],
            input_model_data: vec![],
            input_motion_data: vec![],
            output_motion_data: vec![],
            language: 0,
        }
    }
    unsafe fn get(instance: *const Self) -> Option<&'static Self> {
        if !instance.is_null() {
            Some(&(*instance))
        } else {
            None
        }
    }
    unsafe fn get_mut(instance: *mut Self) -> Option<&'static mut Self> {
        if !instance.is_null() {
            Some(&mut (*instance))
        } else {
            None
        }
    }
    pub fn name(&self) -> *const i8 {
        self.name.as_ptr()
    }
    pub fn set_language(&mut self, value: u32) {
        self.language = value
    }
    pub fn set_all_selected_accessory_keyframes(&mut self, data: &[u32]) {
        self.accessory_keyframes.resize(data.len(), 0);
        self.accessory_keyframes.copy_from_slice(data);
    }
    pub fn set_all_selected_bone_keyframes(&mut self, name: &str, data: &[u32]) {
        let mut keyframes = vec![];
        keyframes.resize(data.len(), 0);
        keyframes.copy_from_slice(data);
        self.bone_keyframes.insert(name.to_owned(), keyframes);
    }
    pub fn set_all_selected_camera_keyframes(&mut self, data: &[u32]) {
        self.camera_keyframes.resize(data.len(), 0);
        self.camera_keyframes.copy_from_slice(data);
    }
    pub fn set_all_selected_light_keyframes(&mut self, data: &[u32]) {
        self.light_keyframes.resize(data.len(), 0);
        self.light_keyframes.copy_from_slice(data);
    }
    pub fn set_all_selected_model_keyframes(&mut self, data: &[u32]) {
        self.model_keyframes.resize(data.len(), 0);
        self.model_keyframes.copy_from_slice(data);
    }
    pub fn set_all_named_selected_morph_keyframes(&mut self, name: &str, data: &[u32]) {
        let mut keyframes = vec![];
        keyframes.resize(data.len(), 0);
        keyframes.copy_from_slice(data);
        self.morph_keyframes.insert(name.to_owned(), keyframes);
    }
    pub fn set_all_selected_self_shadow_keyframes(&mut self, data: &[u32]) {
        self.self_shadow_keyframes.resize(data.len(), 0);
        self.self_shadow_keyframes.copy_from_slice(data);
    }
    pub fn set_input_active_model_data(&mut self, data: &[u8]) {
        self.input_model_data.resize(data.len(), 0u8);
        self.input_model_data.copy_from_slice(data);
    }
    pub fn set_input_motion_data(&mut self, data: &[u8]) {
        self.input_motion_data.resize(data.len(), 0u8);
        self.input_motion_data.copy_from_slice(data);
    }
    pub fn output_motion_data(&self) -> *const u8 {
        self.output_motion_data.as_ptr()
    }
    pub fn output_motion_data_size(&self) -> usize {
        self.output_motion_data.len()
    }
    pub fn execute(&mut self) -> bool {
        self.output_motion_data.clone_from(&self.input_motion_data);
        true
    }
}

#[no_mangle]
pub extern "C" fn nanoemWASMPluginMotionIOCreate() -> *mut nanoem_wasm_plugin_motion_io_t {
    let instance = Box::new(nanoem_wasm_plugin_motion_io_t::new());
    unsafe { std::mem::transmute(instance) }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn nanoemWASMPluginMotionIOSetLanguage(
    instance: *mut nanoem_wasm_plugin_motion_io_t,
    value: u32,
) {
    if let Some(instance) = nanoem_wasm_plugin_motion_io_t::get_mut(instance) {
        instance.set_language(value)
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn nanoemWASMPluginMotionIOGetName(
    instance: *const nanoem_wasm_plugin_motion_io_t,
) -> *const i8 {
    match nanoem_wasm_plugin_motion_io_t::get(instance) {
        Some(instance) => instance.name(),
        None => std::ptr::null(),
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn nanoemWASMPluginMotionIOSetAllSelectedAccessoryKeyframes(
    instance: *mut nanoem_wasm_plugin_motion_io_t,
    data: *const u32,
    size: usize,
) {
    if let Some(instance) = nanoem_wasm_plugin_motion_io_t::get_mut(instance) {
        let slice = std::slice::from_raw_parts(data, size);
        instance.set_all_selected_accessory_keyframes(slice);
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn nanoemWASMPluginMotionIOSetAllNamedSelectedBoneKeyframes(
    instance: *mut nanoem_wasm_plugin_motion_io_t,
    name: *const i8,
    data: *const u32,
    size: usize,
) {
    if let Some(instance) = nanoem_wasm_plugin_motion_io_t::get_mut(instance) {
        let slice = std::slice::from_raw_parts(data, size);
        let name = std::ffi::CStr::from_ptr(name).to_string_lossy();
        instance.set_all_selected_bone_keyframes(&name, slice);
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn nanoemWASMPluginMotionIOSetAllSelectedCameraKeyframes(
    instance: *mut nanoem_wasm_plugin_motion_io_t,
    data: *const u32,
    size: usize,
) {
    if let Some(instance) = nanoem_wasm_plugin_motion_io_t::get_mut(instance) {
        let slice = std::slice::from_raw_parts(data, size);
        instance.set_all_selected_camera_keyframes(slice);
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn nanoemWASMPluginMotionIOSetAllSelectedLightKeyframes(
    instance: *mut nanoem_wasm_plugin_motion_io_t,
    data: *const u32,
    size: usize,
) {
    if let Some(instance) = nanoem_wasm_plugin_motion_io_t::get_mut(instance) {
        let slice = std::slice::from_raw_parts(data, size);
        instance.set_all_selected_light_keyframes(slice);
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn nanoemWASMPluginMotionIOSetAllSelectedModelKeyframes(
    instance: *mut nanoem_wasm_plugin_motion_io_t,
    data: *const u32,
    size: usize,
) {
    if let Some(instance) = nanoem_wasm_plugin_motion_io_t::get_mut(instance) {
        let slice = std::slice::from_raw_parts(data, size);
        instance.set_all_selected_model_keyframes(slice);
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn nanoemWASMPluginMotionIOSetAllNamedSelectedMorphKeyframes(
    instance: *mut nanoem_wasm_plugin_motion_io_t,
    name: *const i8,
    data: *const u32,
    size: usize,
) {
    if let Some(instance) = nanoem_wasm_plugin_motion_io_t::get_mut(instance) {
        let slice = std::slice::from_raw_parts(data, size);
        let name = std::ffi::CStr::from_ptr(name).to_string_lossy();
        instance.set_all_named_selected_morph_keyframes(&name, slice);
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn nanoemWASMPluginMotionIOSetAllSelectedSelfShadowKeyframes(
    instance: *mut nanoem_wasm_plugin_motion_io_t,
    data: *const u32,
    size: usize,
) {
    if let Some(instance) = nanoem_wasm_plugin_motion_io_t::get_mut(instance) {
        let slice = std::slice::from_raw_parts(data, size);
        instance.set_all_selected_self_shadow_keyframes(slice);
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn nanoemWASMPluginMotionIOSetInputActiveModelData(
    instance: *mut nanoem_wasm_plugin_motion_io_t,
    data: *const u8,
    size: usize,
) {
    if let Some(instance) = nanoem_wasm_plugin_motion_io_t::get_mut(instance) {
        let slice = std::slice::from_raw_parts(data, size);
        instance.set_input_active_model_data(slice);
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn nanoemWASMPluginMotionIOSetInputMotionData(
    instance: *mut nanoem_wasm_plugin_motion_io_t,
    data: *const u8,
    size: usize,
) {
    if let Some(instance) = nanoem_wasm_plugin_motion_io_t::get_mut(instance) {
        let slice = std::slice::from_raw_parts(data, size);
        instance.set_input_motion_data(slice);
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn nanoemWASMPluginMotionIOExecute(
    instance: *mut nanoem_wasm_plugin_motion_io_t,
) -> bool {
    match nanoem_wasm_plugin_motion_io_t::get_mut(instance) {
        Some(instance) => instance.execute(),
        None => false,
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn nanoemWASMPluginMotionIOGetOutputMotionDataSize(
    instance: *const nanoem_wasm_plugin_motion_io_t,
) -> usize {
    match nanoem_wasm_plugin_motion_io_t::get(instance) {
        Some(instance) => instance.output_motion_data_size(),
        None => 0,
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn nanoemWASMPluginMotionIOGetOutputMotionData(
    instance: *const nanoem_wasm_plugin_motion_io_t,
) -> *const u8 {
    match nanoem_wasm_plugin_motion_io_t::get(instance) {
        Some(instance) => instance.output_motion_data(),
        None => std::ptr::null(),
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn nanoemWASMPluginMotionIODestroy(
    instance: *mut nanoem_wasm_plugin_motion_io_t,
) {
    if !instance.is_null() {
        let _: Box<nanoem_wasm_plugin_motion_io_t> = std::mem::transmute(instance);
    }
}
