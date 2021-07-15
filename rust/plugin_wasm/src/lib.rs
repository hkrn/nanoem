/*
  Copyright (c) 2015-2021 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

use core::slice;
use std::mem::{size_of, size_of_val};

use anyhow::Result;
use wasmer::{Array, Function, Instance, Memory, WasmPtr};

pub(crate) const PLUGIN_MODEL_IO_ABI_VERSION: u32 = 2 << 16;
pub(crate) const PLUGIN_MOTION_IO_ABI_VERSION: u32 = 2 << 16;

pub(crate) const PLUGIN_NAME: &[u8] = b"plugin_wasm\0";
pub(crate) const PLUGIN_DESCRIPTION: &[u8] = b"nanoem WASM Plugin Loader\0";
pub(crate) const PLUGIN_VERSION: &[u8] = b"1.0.0\0";

#[allow(dead_code, non_camel_case_types)]
#[derive(Copy, Clone, Debug, Hash, PartialEq, PartialOrd)]
#[repr(i32)]
pub enum nanoem_application_plugin_status_t {
    ERROR_REFER_REASON = -3,
    ERROR_UNKNOWN_OPTION = -2,
    ERROR_NULL_OBJECT = -1,
    SUCCESS = 0,
}

impl nanoem_application_plugin_status_t {
    pub(crate) unsafe fn assign(self, value: *mut nanoem_application_plugin_status_t) {
        if !value.is_null() {
            *value = self;
        }
    }
}

pub(crate) type ByteArray = WasmPtr<u8, Array>;
pub(crate) type OpaquePtr = WasmPtr<()>;
pub(crate) type SizePtr = WasmPtr<u32>;
pub(crate) type StatusPtr = WasmPtr<i32>;

fn inner_set_data_internal(
    instance: &Instance,
    opaque: &OpaquePtr,
    data: &[u8],
    name: &str,
) -> Result<()> {
    let func = resolve_func(instance, name);
    let set_input_model_data = func.native::<(OpaquePtr, ByteArray, u32, StatusPtr), ()>()?;
    let data_size = data.len() as u32;
    let data_ptr = allocate_byte_array_with_data(instance, data)?;
    let status_ptr = allocate_status_ptr(instance)?;
    set_input_model_data.call(*opaque, data_ptr, data_size as u32, status_ptr)?;
    release_byte_array(instance, data_ptr)?;
    release_status_ptr(instance, status_ptr)?;
    Ok(())
}

pub(crate) fn resolve_func<'a>(instance: &'a Instance, name: &'a str) -> &'a Function {
    instance.exports.get_function(name).unwrap()
}

pub(crate) fn inner_memory(instance: &Instance) -> &Memory {
    instance.exports.get_memory("memory").unwrap()
}

pub(crate) fn allocate_byte_array(instance: &Instance, data_size: u32) -> Result<ByteArray> {
    let malloc_func = instance.exports.get_function("malloc")?;
    let malloc_func = malloc_func.native::<u32, ByteArray>()?;
    let data_ptr = malloc_func.call(data_size)?;
    Ok(data_ptr)
}

pub(crate) fn allocate_byte_array_with_data(instance: &Instance, data: &[u8]) -> Result<ByteArray> {
    let data_size = data.len() as u32;
    let data_ptr = allocate_byte_array(instance, data_size)?;
    let cell = data_ptr
        .deref(inner_memory(instance), 0, data_size)
        .unwrap();
    for (offset, byte) in data.iter().enumerate() {
        cell[offset].set(*byte);
    }
    Ok(data_ptr)
}

pub(crate) fn allocate_status_ptr(instance: &Instance) -> Result<StatusPtr> {
    let memory = inner_memory(instance);
    let malloc_func = instance.exports.get_function("malloc")?;
    let malloc_func = malloc_func.native::<u32, StatusPtr>()?;
    let data_ptr = malloc_func.call(size_of::<u32>() as u32)?;
    data_ptr.deref(memory).unwrap().set(0);
    Ok(data_ptr)
}

pub(crate) fn allocate_size_ptr(instance: &Instance) -> Result<SizePtr> {
    let memory = inner_memory(instance);
    let malloc_func = instance.exports.get_function("malloc")?;
    let malloc_func = malloc_func.native::<u32, SizePtr>()?;
    let size_ptr = malloc_func.call(size_of::<u32>() as u32)?;
    size_ptr.deref(memory).unwrap().set(0);
    Ok(size_ptr)
}

pub(crate) fn release_byte_array(instance: &Instance, ptr: ByteArray) -> Result<()> {
    let free_func = instance.exports.get_function("free").unwrap();
    let free_func = free_func.native::<ByteArray, ()>()?;
    free_func.call(ptr)?;
    Ok(())
}

pub(crate) fn release_size_ptr(instance: &Instance, ptr: SizePtr) -> Result<()> {
    let free_func = instance.exports.get_function("free").unwrap();
    let free_func = free_func.native::<SizePtr, ()>()?;
    free_func.call(ptr)?;
    Ok(())
}

pub(crate) fn release_status_ptr(instance: &Instance, ptr: StatusPtr) -> Result<()> {
    let free_func = instance.exports.get_function("free").unwrap();
    let free_func = free_func.native::<StatusPtr, ()>()?;
    let status = ptr.deref(inner_memory(instance)).unwrap().get();
    free_func.call(ptr)?;
    if status != 0 {
        return Err(anyhow::anyhow!("code={}", status));
    }
    Ok(())
}

pub(crate) fn inner_initialize_function(instance: &Instance, name: &str) -> Result<()> {
    let func = resolve_func(instance, name);
    let initialize = func.native::<(), ()>()?;
    initialize.call()?;
    Ok(())
}

pub(crate) fn inner_create_opaque(instance: &Instance, name: &str) -> Result<OpaquePtr> {
    let func = resolve_func(instance, name);
    let create = func.native::<(), OpaquePtr>()?;
    Ok(create.call()?)
}

pub(crate) fn inner_destroy_opaque(instance: &Instance, opaque: &Option<OpaquePtr>, name: &str) {
    if let Some(opaque) = opaque {
        let func = resolve_func(instance, name);
        if let Ok(destroy) = func.native::<OpaquePtr, ()>() {
            destroy.call(*opaque).unwrap();
        }
    }
}

pub(crate) fn inner_terminate_function(instance: &Instance, name: &str) {
    let func = resolve_func(instance, name);
    if let Ok(terminate) = func.native::<(), ()>() {
        terminate.call().unwrap();
    }
}

pub(crate) fn inner_get_string(
    instance: &Instance,
    opaque: &Option<OpaquePtr>,
    name: &str,
) -> Result<String> {
    if let Some(opaque) = opaque {
        let func = resolve_func(instance, name);
        let get_string = func.native::<OpaquePtr, ByteArray>()?;
        let memory = inner_memory(instance);
        Ok(get_string
            .call(*opaque)?
            .get_utf8_string_with_nul(memory)
            .unwrap_or_default())
    } else {
        Ok(Default::default())
    }
}

pub(crate) fn inner_get_data(
    instance: &Instance,
    opaque: &Option<OpaquePtr>,
    body_func_name: &str,
    size_func_name: &str,
) -> Result<Vec<u8>> {
    if let Some(opaque) = opaque {
        let func = resolve_func(instance, size_func_name);
        let get_data_size = func.native::<(OpaquePtr, SizePtr), ()>()?;
        let func = resolve_func(instance, body_func_name);
        let get_data_body = func.native::<(OpaquePtr, ByteArray, u32, StatusPtr), ()>()?;
        let data_size_ptr = allocate_size_ptr(instance)?;
        get_data_size.call(*opaque, data_size_ptr)?;
        let data_size = data_size_ptr.deref(inner_memory(instance)).unwrap().get();
        let bytes = allocate_byte_array(instance, data_size)?;
        let status_ptr = allocate_status_ptr(instance)?;
        get_data_body.call(*opaque, bytes, data_size, status_ptr)?;
        let cell = bytes.deref(inner_memory(instance), 0, data_size).unwrap();
        let mut output_data = Vec::with_capacity(data_size as usize);
        for byte in cell {
            output_data.push(byte.get());
        }
        release_byte_array(instance, bytes)?;
        release_size_ptr(instance, data_size_ptr)?;
        release_status_ptr(instance, status_ptr)?;
        Ok(output_data)
    } else {
        Ok(vec![])
    }
}

pub(crate) fn inner_set_data<T>(
    instance: &Instance,
    opaque: &Option<OpaquePtr>,
    data: &[T],
    name: &str,
) -> Result<()> {
    if let Some(opaque) = opaque {
        let len = data.len() * size_of_val(&data[0]);
        let data = unsafe { slice::from_raw_parts(data.as_ptr() as *const u8, len) };
        inner_set_data_internal(instance, opaque, data, name)?;
    }
    Ok(())
}

pub(crate) fn inner_set_language(
    instance: &Instance,
    opaque: &Option<OpaquePtr>,
    value: i32,
    name: &str,
) -> Result<()> {
    if let Some(opaque) = opaque {
        let func = resolve_func(instance, name);
        let set_language = func.native::<(OpaquePtr, i32), ()>()?;
        set_language.call(*opaque, value)?;
    }
    Ok(())
}

pub(crate) fn inner_count_all_functions(
    instance: &Instance,
    opaque: &Option<OpaquePtr>,
    name: &str,
) -> Result<i32> {
    if let Some(opaque) = opaque {
        let func = resolve_func(instance, name);
        let count_all_functions = func.native::<OpaquePtr, i32>()?;
        Ok(count_all_functions.call(*opaque)?)
    } else {
        Ok(0)
    }
}

pub(crate) fn inner_get_function_name(
    instance: &Instance,
    opaque: &Option<OpaquePtr>,
    index: i32,
    name: &str,
) -> Result<String> {
    if let Some(opaque) = opaque {
        let func = resolve_func(instance, name);
        let get_function_name = func.native::<(OpaquePtr, i32), ByteArray>()?;
        let memory = inner_memory(instance);
        let name = get_function_name
            .call(*opaque, index)?
            .get_utf8_string_with_nul(memory)
            .unwrap_or_default();
        Ok(name)
    } else {
        Ok(Default::default())
    }
}

pub(crate) fn inner_set_function(
    instance: &Instance,
    opaque: &Option<OpaquePtr>,
    index: i32,
    name: &str,
) -> Result<()> {
    if let Some(opaque) = opaque {
        let func = resolve_func(instance, name);
        let status_ptr = allocate_status_ptr(instance)?;
        let set_function = func.native::<(OpaquePtr, i32, StatusPtr), ()>()?;
        set_function.call(*opaque, index, status_ptr)?;
        release_status_ptr(instance, status_ptr)?;
    }
    Ok(())
}

pub(crate) fn inner_execute(
    instance: &Instance,
    opaque: &Option<OpaquePtr>,
    name: &str,
) -> Result<()> {
    if let Some(opaque) = opaque {
        let status_ptr = allocate_status_ptr(instance)?;
        let func = resolve_func(instance, name);
        let execute = func.native::<(OpaquePtr, StatusPtr), ()>()?;
        execute.call(*opaque, status_ptr)?;
        release_status_ptr(instance, status_ptr)?;
    }
    Ok(())
}

pub(crate) fn inner_load_ui_window(
    instance: &Instance,
    opaque: &Option<OpaquePtr>,
    name: &str,
) -> Result<()> {
    if let Some(opaque) = opaque {
        let func = resolve_func(instance, name);
        let load_ui_window_layout = func.native::<(OpaquePtr, StatusPtr), ()>()?;
        let status_ptr = allocate_status_ptr(instance)?;
        load_ui_window_layout.call(*opaque, status_ptr)?;
        release_status_ptr(instance, status_ptr)?;
    }
    Ok(())
}

pub(crate) fn inner_set_ui_component_layout(
    instance: &Instance,
    opaque: &Option<OpaquePtr>,
    id: &str,
    data: &[u8],
    name: &str,
) -> Result<()> {
    if let Some(opaque) = opaque {
        let func = resolve_func(instance, name);
        let set_ui_component_layout =
            func.native::<(OpaquePtr, ByteArray, ByteArray, u32, SizePtr, StatusPtr), ()>()?;
        let id_ptr = allocate_byte_array_with_data(instance, id.as_bytes())?;
        let data_ptr = allocate_byte_array_with_data(instance, data)?;
        let reload_layout_ptr = allocate_size_ptr(instance)?;
        let status_ptr = allocate_status_ptr(instance)?;
        set_ui_component_layout.call(
            *opaque,
            id_ptr,
            data_ptr,
            data.len() as u32,
            reload_layout_ptr,
            status_ptr,
        )?;
        release_byte_array(instance, id_ptr)?;
        release_byte_array(instance, data_ptr)?;
        release_size_ptr(instance, reload_layout_ptr)?;
        release_status_ptr(instance, status_ptr)?;
    }
    Ok(())
}

mod model;
mod motion;
