/*
  Copyright (c) 2015-2020 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#[allow(non_camel_case_types)]
#[derive(Default)]
pub struct nanoem_wasm_plugin_model_io_t {
    name: std::ffi::CString,
    vertex_indices: Vec<i32>,
    material_indices: Vec<i32>,
    bone_indices: Vec<i32>,
    morph_indices: Vec<i32>,
    label_indices: Vec<i32>,
    rigid_body_indices: Vec<i32>,
    joint_indices: Vec<i32>,
    input_model_data: Vec<u8>,
    output_model_data: Vec<u8>,
    language: u32,
}

impl Drop for nanoem_wasm_plugin_model_io_t {
    fn drop(&mut self) {
        println!("vertex_indices: {:?}", self.vertex_indices);
        println!("material_indices: {:?}", self.material_indices);
        println!("bone_indices: {:?}", self.bone_indices);
        println!("morph_indices: {:?}", self.morph_indices);
        println!("label_indices: {:?}", self.label_indices);
        println!("rigid_body_indices: {:?}", self.rigid_body_indices);
        println!("joint_indices: {:?}", self.joint_indices);
    }
}

impl nanoem_wasm_plugin_model_io_t {
    pub fn new() -> Self {
        Self {
            name: std::ffi::CString::new("Passthrough Model Plugin").unwrap(),
            vertex_indices: vec![],
            material_indices: vec![],
            bone_indices: vec![],
            morph_indices: vec![],
            label_indices: vec![],
            rigid_body_indices: vec![],
            joint_indices: vec![],
            input_model_data: vec![],
            output_model_data: vec![],
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
    pub fn set_all_selected_vertex_indices(&mut self, data: &[i32]) {
        self.vertex_indices.resize(data.len(), 0);
        self.vertex_indices.copy_from_slice(data);
    }
    pub fn set_all_selected_material_indices(&mut self, data: &[i32]) {
        self.material_indices.resize(data.len(), 0);
        self.material_indices.copy_from_slice(data);
    }
    pub fn set_all_selected_bone_indices(&mut self, data: &[i32]) {
        self.bone_indices.resize(data.len(), 0);
        self.bone_indices.copy_from_slice(data);
    }
    pub fn set_all_selected_morph_indices(&mut self, data: &[i32]) {
        self.morph_indices.resize(data.len(), 0);
        self.morph_indices.copy_from_slice(data);
    }
    pub fn set_all_selected_label_indices(&mut self, data: &[i32]) {
        self.label_indices.resize(data.len(), 0);
        self.label_indices.copy_from_slice(data);
    }
    pub fn set_all_selected_rigid_body_indices(&mut self, data: &[i32]) {
        self.rigid_body_indices.resize(data.len(), 0);
        self.rigid_body_indices.copy_from_slice(data);
    }
    pub fn set_all_selected_joint_indices(&mut self, data: &[i32]) {
        self.joint_indices.resize(data.len(), 0);
        self.joint_indices.copy_from_slice(data);
    }
    pub fn set_input_model_data(&mut self, data: &[u8]) {
        self.input_model_data.resize(data.len(), 0);
        self.input_model_data.copy_from_slice(data);
    }
    pub fn output_model_data(&self) -> *const u8 {
        self.output_model_data.as_ptr()
    }
    pub fn output_model_data_size(&self) -> usize {
        self.output_model_data.len()
    }
    pub fn execute(&mut self) -> bool {
        self.output_model_data.clone_from(&self.input_model_data);
        true
    }
    pub fn failure_reason(&self) -> *const i8 {
        std::ptr::null()
    }
    pub fn recovery_suggestion(&self) -> *const i8 {
        std::ptr::null()
    }
}

#[no_mangle]
pub extern "C" fn nanoemWASMPluginModelIOCreate() -> *mut nanoem_wasm_plugin_model_io_t {
    let instance = Box::new(nanoem_wasm_plugin_model_io_t::new());
    unsafe { std::mem::transmute(instance) }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn nanoemWASMPluginModelIOSetLanguage(
    instance: *mut nanoem_wasm_plugin_model_io_t,
    value: u32,
) {
    if let Some(instance) = nanoem_wasm_plugin_model_io_t::get_mut(instance) {
        instance.set_language(value)
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn nanoemWASMPluginModelIOGetName(
    instance: *const nanoem_wasm_plugin_model_io_t,
) -> *const i8 {
    match nanoem_wasm_plugin_model_io_t::get(instance) {
        Some(instance) => instance.name(),
        None => std::ptr::null(),
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn nanoemWASMPluginModelIOSetAllSelectedVertexIndices(
    instance: *mut nanoem_wasm_plugin_model_io_t,
    data: *const i32,
    size: usize,
) {
    if let Some(instance) = nanoem_wasm_plugin_model_io_t::get_mut(instance) {
        let slice = std::slice::from_raw_parts(data, size);
        instance.set_all_selected_vertex_indices(slice);
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn nanoemWASMPluginModelIOSetAllSelectedMaterialIndices(
    instance: *mut nanoem_wasm_plugin_model_io_t,
    data: *const i32,
    size: usize,
) {
    if let Some(instance) = nanoem_wasm_plugin_model_io_t::get_mut(instance) {
        let slice = std::slice::from_raw_parts(data, size);
        instance.set_all_selected_material_indices(slice);
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn nanoemWASMPluginModelIOSetAllSelectedBoneIndices(
    instance: *mut nanoem_wasm_plugin_model_io_t,
    data: *const i32,
    size: usize,
) {
    if let Some(instance) = nanoem_wasm_plugin_model_io_t::get_mut(instance) {
        let slice = std::slice::from_raw_parts(data, size);
        instance.set_all_selected_bone_indices(slice);
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn nanoemWASMPluginModelIOSetAllSelectedMorphIndices(
    instance: *mut nanoem_wasm_plugin_model_io_t,
    data: *const i32,
    size: usize,
) {
    if let Some(instance) = nanoem_wasm_plugin_model_io_t::get_mut(instance) {
        let slice = std::slice::from_raw_parts(data, size);
        instance.set_all_selected_morph_indices(slice);
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn nanoemWASMPluginModelIOSetAllSelectedLabelIndices(
    instance: *mut nanoem_wasm_plugin_model_io_t,
    data: *const i32,
    size: usize,
) {
    if let Some(instance) = nanoem_wasm_plugin_model_io_t::get_mut(instance) {
        let slice = std::slice::from_raw_parts(data, size);
        instance.set_all_selected_label_indices(slice);
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn nanoemWASMPluginModelIOSetAllSelectedRigidBodyIndices(
    instance: *mut nanoem_wasm_plugin_model_io_t,
    data: *const i32,
    size: usize,
) {
    if let Some(instance) = nanoem_wasm_plugin_model_io_t::get_mut(instance) {
        let slice = std::slice::from_raw_parts(data, size);
        instance.set_all_selected_rigid_body_indices(slice);
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn nanoemWASMPluginModelIOSetAllSelectedJointIndices(
    instance: *mut nanoem_wasm_plugin_model_io_t,
    data: *const i32,
    size: usize,
) {
    if let Some(instance) = nanoem_wasm_plugin_model_io_t::get_mut(instance) {
        let slice = std::slice::from_raw_parts(data, size);
        instance.set_all_selected_joint_indices(slice);
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn nanoemWASMPluginModelIOSetInputModelData(
    instance: *mut nanoem_wasm_plugin_model_io_t,
    data: *const u8,
    size: usize,
) {
    if let Some(instance) = nanoem_wasm_plugin_model_io_t::get_mut(instance) {
        let slice = std::slice::from_raw_parts(data, size);
        instance.set_input_model_data(slice);
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn nanoemWASMPluginModelIOExecute(
    instance: *mut nanoem_wasm_plugin_model_io_t,
) -> bool {
    match nanoem_wasm_plugin_model_io_t::get_mut(instance) {
        Some(instance) => instance.execute(),
        None => false,
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn nanoemWASMPluginModelIOGetOutputModelDataSize(
    instance: *const nanoem_wasm_plugin_model_io_t,
) -> usize {
    match nanoem_wasm_plugin_model_io_t::get(instance) {
        Some(instance) => instance.output_model_data_size(),
        None => 0,
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn nanoemWASMPluginModelIOGetOutputModelData(
    instance: *const nanoem_wasm_plugin_model_io_t,
) -> *const u8 {
    match nanoem_wasm_plugin_model_io_t::get(instance) {
        Some(instance) => instance.output_model_data(),
        None => std::ptr::null(),
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn nanoemWASMPluginModelIOGetFailureReason(
    instance: *const nanoem_wasm_plugin_model_io_t,
) -> *const i8 {
    match nanoem_wasm_plugin_model_io_t::get(instance) {
        Some(instance) => instance.failure_reason(),
        None => std::ptr::null(),
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn nanoemWASMPluginModelIOGetRecoverySuggestion(
    instance: *const nanoem_wasm_plugin_model_io_t,
) -> *const i8 {
    match nanoem_wasm_plugin_model_io_t::get(instance) {
        Some(instance) => instance.recovery_suggestion(),
        None => std::ptr::null(),
    }
}

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn nanoemWASMPluginModelIODestroy(
    instance: *mut nanoem_wasm_plugin_model_io_t,
) {
    if !instance.is_null() {
        let _: Box<nanoem_wasm_plugin_model_io_t> = std::mem::transmute(instance);
    }
}
