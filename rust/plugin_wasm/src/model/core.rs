/*
  Copyright (c) 2015-2020 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

use super::super::nanoem_application_plugin_status_t;

use std::ffi::{CStr, CString};
use std::io::Read;
use std::ptr::null;
use std::sync::Arc;
use std::sync::Mutex;
use wasmer_runtime::{Array, Func, Instance, WasmPtr};

pub enum ModelPluginError {
    // InvalidArgument,
    FromError(String),
}
type Result<T> = std::result::Result<T, ModelPluginError>;

impl ModelPluginError {
    pub fn to_string(&self, _language: i32) -> String {
        match self {
            // ModelPluginError::InvalidArgument => "invalid argument".to_string(),
            ModelPluginError::FromError(value) => value.to_string(),
        }
    }
}

impl<E: std::error::Error> From<E> for ModelPluginError {
    fn from(value: E) -> Self {
        ModelPluginError::FromError(value.to_string())
    }
}

struct WasmPlugin {
    instance: Instance,
    opaque: WasmPtr<()>,
    name: CString,
    path: std::path::PathBuf,
}

#[allow(non_camel_case_types)]
pub struct nanoem_application_plugin_model_io_t {
    name: CString,
    description: CString,
    version: CString,
    failure_reason: Option<CString>,
    recovery_suggestion: Option<CString>,
    output_model_data: Option<Vec<u8>>,
    window_layout_data: Option<Vec<u8>>,
    instances: Arc<Mutex<Vec<WasmPlugin>>>,
    watcher: hotwatch::Hotwatch,
    function_index: i32,
    language: i32,
}

impl Drop for nanoem_application_plugin_model_io_t {
    fn drop(&mut self) {
        let instances = self.instances.lock().unwrap();
        for WasmPlugin {
            instance, opaque, ..
        } in instances.iter()
        {
            if let Ok(func) = instance
                .exports
                .get::<Func<WasmPtr<()>, ()>>("nanoemWASMPluginModelIODestroy")
            {
                func.call(*opaque).unwrap_or_default();
            }
        }
    }
}

#[allow(non_camel_case_types)]
impl nanoem_application_plugin_model_io_t {
    pub fn new() -> std::result::Result<Self, ModelPluginError> {
        let name = CString::new("WASM Model I/O Plugin")?;
        let description = CString::new("WASM Model I/O Plugin on WASM runtime (wasm)")?;
        let version = CString::new("1.0.0")?;
        let watcher = hotwatch::Hotwatch::new()?;
        let instances = Arc::<Mutex<_>>::default();
        Ok(Self {
            name,
            description,
            version,
            failure_reason: None,
            recovery_suggestion: None,
            output_model_data: None,
            window_layout_data: None,
            watcher,
            instances,
            function_index: -1,
            language: 0,
        })
    }
    pub(crate) unsafe fn get(plugin: *const Self) -> Option<&'static Self> {
        if !plugin.is_null() {
            Some(&(*plugin))
        } else {
            None
        }
    }
    pub(crate) unsafe fn get_mut(plugin: *mut Self) -> Option<&'static mut Self> {
        if !plugin.is_null() {
            Some(&mut (*plugin))
        } else {
            None
        }
    }
    pub fn load_all_assemblies(&mut self, path: &str) -> Result<()> {
        if let Some(path) = std::path::Path::new(path).parent() {
            for entry in path.read_dir()? {
                if let Ok(entry) = entry {
                    let path = entry.path();
                    if path.extension().unwrap_or_default() == "wasm" {
                        self.load_assembly(&path)?;
                    }
                }
            }
        }
        Ok(())
    }
    fn load_assembly(&mut self, path: &std::path::PathBuf) -> Result<()> {
        let mut file = std::fs::File::open(path)?;
        let mut content = vec![];
        file.read_to_end(&mut content)?;
        let arc = Arc::clone(&self.instances);
        self.watcher.watch(path, move |event: hotwatch::Event| {
            if let hotwatch::Event::Write(path) = event {
                if let Ok(mut file) = std::fs::File::open(&path) {
                    let mut content = vec![];
                    if file.read_to_end(&mut content).is_ok() {
                        if let Ok(mut instances) = arc.try_lock() {
                            instances.retain(|item| item.path != path);
                            Self::add_wasm_instance(&path, &content[..], &mut instances)
                                .unwrap_or_default();
                        }
                    }
                }
            }
        })?;
        let mut instances = self.instances.try_lock()?;
        Self::add_wasm_instance(path, &content[..], &mut instances)?;
        Ok(())
    }
    pub fn name(&self) -> *const i8 {
        self.name.as_ptr()
    }
    pub fn description(&self) -> *const i8 {
        self.description.as_ptr()
    }
    pub fn version(&self) -> *const i8 {
        self.version.as_ptr()
    }
    pub fn count_all_functions(&self) -> i32 {
        let instances = self.instances.lock().unwrap();
        instances.len() as i32
    }
    pub fn function_name(&self, value: i32) -> *const i8 {
        if value >= 0 && value < self.count_all_functions() {
            let instances = self.instances.lock().unwrap();
            let WasmPlugin { name, .. } = &instances[value as usize];
            return name.as_ptr();
        }
        null()
    }
    pub fn set_language(&mut self, value: i32) -> Result<()> {
        type FnSetLanguage<'a> = Func<'a, (WasmPtr<()>, i32), ()>;
        let instances = self.instances.try_lock()?;
        for WasmPlugin {
            instance, opaque, ..
        } in instances.iter()
        {
            if let Ok(func) = instance
                .exports
                .get::<FnSetLanguage>("nanoemWASMPluginModelIOSetLanguage")
            {
                func.call(*opaque, value)?;
            }
        }
        Ok(())
    }
    pub fn set_function(&mut self, value: i32) -> Result<()> {
        self.failure_reason = None;
        self.recovery_suggestion = None;
        self.window_layout_data = None;
        self.output_model_data = None;
        if value >= 0 && value < self.count_all_functions() {
            self.function_index = value
        }
        Ok(())
    }
    pub fn set_all_selected_vertex_indices(&mut self, data: &[i32]) -> Result<()> {
        self.call_optional_method_with_indices(
            "nanoemWASMPluginModelIOSetAllSelectedVertexIndices",
            &data[..],
        )
    }
    pub fn set_all_selected_material_indices(&mut self, data: &[i32]) -> Result<()> {
        self.call_optional_method_with_indices(
            "nanoemWASMPluginModelIOSetAllSelectedMaterialIndices",
            &data[..],
        )
    }
    pub fn set_all_selected_bone_indices(&mut self, data: &[i32]) -> Result<()> {
        self.call_optional_method_with_indices(
            "nanoemWASMPluginModelIOSetAllSelectedBoneIndices",
            &data[..],
        )
    }
    pub fn set_all_selected_morph_indices(&mut self, data: &[i32]) -> Result<()> {
        self.call_optional_method_with_indices(
            "nanoemWASMPluginModelIOSetAllSelectedMorphIndices",
            &data[..],
        )
    }
    pub fn set_all_selected_label_indices(&mut self, data: &[i32]) -> Result<()> {
        self.call_optional_method_with_indices(
            "nanoemWASMPluginModelIOSetAllSelectedLabelIndices",
            &data[..],
        )
    }
    pub fn set_all_selected_rigid_body_indices(&mut self, data: &[i32]) -> Result<()> {
        self.call_optional_method_with_indices(
            "nanoemWASMPluginModelIOSetAllSelectedRigidBodyIndices",
            &data[..],
        )
    }
    pub fn set_all_selected_joint_indices(&mut self, data: &[i32]) -> Result<()> {
        self.call_optional_method_with_indices(
            "nanoemWASMPluginModelIOSetAllSelectedJointIndices",
            &data[..],
        )
    }
    pub fn set_audio_description(&mut self, data: &[u8]) -> Result<()> {
        self.call_optional_method_with_byte_array(
            "nanoemWASMPluginModelIOSetAudioDescription",
            data,
        )
    }
    pub fn set_camera_description(&mut self, data: &[u8]) -> Result<()> {
        self.call_optional_method_with_byte_array(
            "nanoemWASMPluginModelIOSetCameraDescription",
            data,
        )
    }
    pub fn set_light_description(&mut self, data: &[u8]) -> Result<()> {
        self.call_optional_method_with_byte_array(
            "nanoemWASMPluginModelIOSetLightDescription",
            data,
        )
    }
    pub fn set_audio_data(&mut self, data: &[u8]) -> Result<()> {
        self.call_optional_method_with_byte_array("nanoemWASMPluginModelIOSetAudioData", data)
    }
    pub fn set_input_data(&mut self, data: &[u8]) -> Result<()> {
        self.call_method_with_byte_array("nanoemWASMPluginModelIOSetInputModelData", data)
    }
    pub fn execute(&mut self) -> Result<()> {
        debug_assert!(self.function_index >= 0);
        type FnExecute<'a> = Func<'a, WasmPtr<()>, i32>;
        type FnGetOutputModelDataSize<'a> = Func<'a, WasmPtr<()>, u32>;
        type FnGetOutputModelData<'a> = Func<'a, WasmPtr<()>, WasmPtr<u8, Array>>;
        let mut instances = self.instances.try_lock()?;
        let WasmPlugin {
            instance, opaque, ..
        } = &mut instances[self.function_index as usize];
        let func: FnExecute = instance.exports.get("nanoemWASMPluginModelIOExecute")?;
        if func.call(*opaque)? != 0 {
            let func: FnGetOutputModelDataSize = instance
                .exports
                .get("nanoemWASMPluginModelIOGetOutputModelDataSize")?;
            let size = func.call(*opaque)? as usize;
            let mut bytes = vec![];
            bytes.resize(size, 0u8);
            let func: FnGetOutputModelData = instance
                .exports
                .get("nanoemWASMPluginModelIOGetOutputModelData")?;
            let data = func.call(*opaque)?;
            if let Some(cells) = data.deref(instance.context_mut().memory(0), 0, size as u32) {
                for (i, b) in bytes.iter_mut().zip(cells) {
                    *i = b.get();
                }
                self.output_model_data = Some(bytes);
            }
            Ok(())
        } else {
            type FnGetFailureReason<'a> = Func<'a, WasmPtr<()>, WasmPtr<u8, Array>>;
            type FnGetRecoverySuggestion<'a> = Func<'a, WasmPtr<()>, WasmPtr<u8, Array>>;
            let func: FnGetFailureReason = instance
                .exports
                .get("nanoemWASMPluginModelIOGetFailureReason")?;
            let ptr = func.call(*opaque)?;
            let reason =
                if let Some(s) = ptr.get_utf8_string_with_nul(instance.context_mut().memory(0)) {
                    s.to_string()
                } else {
                    String::from("Unknown Error")
                };
            if let Ok(func) = instance
                .exports
                .get::<FnGetRecoverySuggestion>("nanoemWASMPluginModelIOGetRecoverySuggestion")
            {
                let ptr = func.call(*opaque)?;
                if let Some(s) = ptr.get_utf8_string_with_nul(instance.context_mut().memory(0)) {
                    self.recovery_suggestion = Some(CString::new(s)?);
                }
            }
            Err(ModelPluginError::FromError(reason))
        }
    }
    pub fn output_slice(&self) -> &[u8] {
        match &self.output_model_data {
            Some(data) => data.as_slice(),
            None => &[],
        }
    }
    pub fn load_window_layout(&mut self) -> Result<()> {
        type FnLoadUIWindowLayout<'a> = Func<'a, WasmPtr<()>, i32>;
        type FnGetUIWindowLayoutDataSize<'a> = Func<'a, WasmPtr<()>, u32>;
        type FnGetUIWindowLayoutData<'a> = Func<'a, WasmPtr<()>, WasmPtr<u8, Array>>;
        let mut instances = self.instances.try_lock()?;
        let WasmPlugin {
            instance, opaque, ..
        } = &mut instances[self.function_index as usize];
        if let Ok(func) = instance
            .exports
            .get::<FnLoadUIWindowLayout>("nanoemWASMPluginModelIOLoadUIWindowLayout")
        {
            if func.call(*opaque)? != 0 {
                let func = instance.exports.get::<FnGetUIWindowLayoutDataSize>(
                    "nanoemWASMPluginModelIOGetUIWindowLayoutDataSize",
                )?;
                let size = func.call(*opaque)? as usize;
                let mut bytes = vec![];
                bytes.resize(size, 0u8);
                let func = instance.exports.get::<FnGetUIWindowLayoutData>(
                    "nanoemWASMPluginModelIOGetUIWindowLayoutData",
                )?;
                let data = func.call(*opaque)?;
                if let Some(cells) = data.deref(instance.context_mut().memory(0), 0, size as u32) {
                    for (i, b) in bytes.iter_mut().zip(cells) {
                        *i = b.get();
                    }
                    self.window_layout_data = Some(bytes);
                }
            }
        }
        Ok(())
    }
    pub fn set_component_layout(&mut self, id: &CStr, data: &[u8]) -> Result<()> {
        self.call_optional_method_with_named_byte_array(
            "nanoemWASMPluginModelIOSetUIComponentLayoutData",
            id.to_str()?,
            data,
            std::mem::size_of::<u8>(),
        )
    }
    pub fn window_layout_data_slice(&self) -> &[u8] {
        match &self.window_layout_data {
            Some(data) => data.as_slice(),
            None => &[],
        }
    }
    pub fn failure_reason(&self) -> &Option<CString> {
        &self.failure_reason
    }
    pub fn recovery_suggestion(&self) -> &Option<CString> {
        &self.recovery_suggestion
    }
    pub fn assign_failure_reason(
        &mut self,
        value: ModelPluginError,
    ) -> nanoem_application_plugin_status_t {
        match CString::new(value.to_string(self.language)) {
            Ok(reason) => {
                self.failure_reason = Some(reason);
                nanoem_application_plugin_status_t::ERROR_REFER_REASON
            }
            Err(_) => nanoem_application_plugin_status_t::ERROR_NULL_OBJECT,
        }
    }
    fn add_wasm_instance(
        path: &std::path::Path,
        content: &[u8],
        instances: &mut Vec<WasmPlugin>,
    ) -> Result<()> {
        type FnCreate<'a> = Func<'a, (), WasmPtr<()>>;
        type FnGetName<'a> = Func<'a, WasmPtr<()>, WasmPtr<u8, Array>>;
        let module = wasmer_runtime::compile(content)?;
        let imports = if let Some(wasi_version) = wasmer_wasi::get_wasi_version(&module, false) {
            wasmer_wasi::generate_import_object_for_version(
                wasi_version,
                vec![],
                vec![],
                vec![],
                vec![],
            )
        } else {
            wasmer_runtime::imports! {}
        };
        let mut instance = module.instantiate(&imports)?;
        let func: FnCreate = instance.exports.get("nanoemWASMPluginModelIOCreate")?;
        let opaque = func.call()?;
        let func: FnGetName = instance.exports.get("nanoemWASMPluginModelIOGetName")?;
        let ptr = func.call(opaque)?;
        if let Some(s) = ptr.get_utf8_string_with_nul(instance.context_mut().memory(0)) {
            let name = CString::new(s)?;
            let path = path.to_path_buf();
            instances.push(WasmPlugin {
                instance,
                opaque,
                name,
                path,
            });
        }
        Ok(())
    }
    fn call_method_with_byte_array(&mut self, method: &str, data: &[u8]) -> Result<()> {
        self.call_method_with_vec(method, data, std::mem::size_of::<u8>())
    }
    fn call_method_with_vec(&mut self, method: &str, data: &[u8], stride: usize) -> Result<()> {
        debug_assert!(self.function_index >= 0);
        type FnSetByteArray<'a> = Func<'a, (WasmPtr<()>, WasmPtr<u8, Array>, i32), ()>;
        let mut instances = self.instances.try_lock()?;
        let WasmPlugin {
            instance, opaque, ..
        } = &mut instances[self.function_index as usize];
        let memory = instance.context_mut().memory(0);
        for (byte, cell) in data
            .iter()
            .cloned()
            .zip(memory.view()[0..data.len()].iter())
        {
            cell.set(byte);
        }
        let func: FnSetByteArray = instance.exports.get(method)?;
        func.call(*opaque, WasmPtr::new(0), (data.len() / stride) as _)?;
        Ok(())
    }
    fn call_optional_method_with_indices(&mut self, method: &str, data: &[i32]) -> Result<()> {
        let ptr = data.as_ptr() as *const _ as *const u8;
        let size = data.len() * std::mem::size_of::<i32>();
        let data = unsafe { std::slice::from_raw_parts(ptr, size) };
        self.call_optional_method_with_vec(method, data, std::mem::size_of::<i32>())
    }
    fn call_optional_method_with_byte_array(&mut self, method: &str, data: &[u8]) -> Result<()> {
        self.call_optional_method_with_vec(method, data, std::mem::size_of::<u8>())
    }
    fn call_optional_method_with_vec(
        &mut self,
        method: &str,
        data: &[u8],
        stride: usize,
    ) -> Result<()> {
        debug_assert!(self.function_index >= 0);
        type FnSetByteArray<'a> = Func<'a, (WasmPtr<()>, WasmPtr<u8, Array>, i32), ()>;
        let function_index = self.function_index as usize;
        let mut instances = self.instances.try_lock()?;
        let memory = instances[function_index].instance.context_mut().memory(0);
        for (byte, cell) in data
            .iter()
            .cloned()
            .zip(memory.view()[0..data.len()].iter())
        {
            cell.set(byte);
        }
        let WasmPlugin {
            instance, opaque, ..
        } = &instances[function_index];
        if let Ok(func) = instance.exports.get::<FnSetByteArray>(method) {
            func.call(*opaque, WasmPtr::new(0), (data.len() / stride) as _)?;
        }
        Ok(())
    }
    fn call_optional_method_with_named_byte_array(
        &mut self,
        method: &str,
        name: &str,
        data: &[u8],
        stride: usize,
    ) -> Result<()> {
        debug_assert!(self.function_index >= 0);
        type FnSetNamedByteArray<'a> =
            Func<'a, (WasmPtr<()>, WasmPtr<u8, Array>, WasmPtr<u8, Array>, i32)>;
        let function_index = self.function_index as usize;
        let mut instances = self.instances.try_lock()?;
        let memory = instances[function_index].instance.context_mut().memory(0);
        let memory_view: wasmer_runtime::memory::MemoryView<u8> = memory.view();
        let data_size = data.len();
        for (byte, cell) in data.iter().cloned().zip(memory_view[0..data_size].iter()) {
            cell.set(byte);
        }
        let bytes = name.as_bytes();
        for (byte, cell) in bytes
            .iter()
            .cloned()
            .zip(memory_view[data_size..data_size + bytes.len()].iter())
        {
            cell.set(byte);
        }
        memory_view[data_size + bytes.len() + 1].set(0);
        let WasmPlugin {
            instance, opaque, ..
        } = &instances[function_index];
        if let Ok(func) = instance.exports.get::<FnSetNamedByteArray>(method) {
            func.call(
                *opaque,
                WasmPtr::new(data_size as u32),
                WasmPtr::new(0),
                (data.len() / stride) as _,
            )?;
        }
        Ok(())
    }
}
