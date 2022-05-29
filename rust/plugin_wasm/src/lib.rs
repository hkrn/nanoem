/*
  Copyright (c) 2015-2021 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

use core::slice;
use std::mem::size_of;

use anyhow::Result;
use tracing::{debug, warn};
use wasmer::{Array, Instance, Memory, NativeFunc, WasmPtr};

pub(crate) const PLUGIN_MODEL_IO_ABI_VERSION: u32 = 2 << 16;
pub(crate) const PLUGIN_MOTION_IO_ABI_VERSION: u32 = 2 << 16;

pub(crate) const PLUGIN_NAME: &[u8] = b"plugin_wasm\0";
pub(crate) const PLUGIN_DESCRIPTION: &[u8] = b"nanoem WASM Plugin Loader\0";
pub(crate) const PLUGIN_VERSION: &[u8] = b"1.0.0\0";

const MALLOC_FN: &str = "nanoemApplicationPluginAllocateMemoryWASM";
const FREE_FN: &str = "nanoemApplicationPluginReleaseMemoryWASM";

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

fn notify_export_function_error(name: &str) {
    debug!(
        "{} cannot be called due to not found or signature unmatched",
        name
    );
}

fn inner_get_data_internal(
    instance: &Instance,
    opaque: &OpaquePtr,
    get_data_size: NativeFunc<(OpaquePtr, SizePtr), ()>,
    get_data_body: NativeFunc<(OpaquePtr, ByteArray, u32, StatusPtr), ()>,
    output_data: &mut Vec<u8>,
) -> Result<()> {
    let data_size_ptr = allocate_size_ptr(instance)?;
    get_data_size.call(*opaque, data_size_ptr)?;
    let data_size = data_size_ptr.deref(inner_memory(instance)).unwrap().get();
    let bytes = allocate_byte_array(instance, data_size)?;
    let status_ptr = allocate_status_ptr(instance)?;
    get_data_body.call(*opaque, bytes, data_size, status_ptr)?;
    let cell = bytes.deref(inner_memory(instance), 0, data_size).unwrap();
    output_data.resize(data_size as usize, 0);
    for byte in cell {
        output_data.push(byte.get());
    }
    release_byte_array(instance, bytes)?;
    release_size_ptr(instance, data_size_ptr)?;
    release_status_ptr(instance, status_ptr)?;
    Ok(())
}

fn inner_set_data_internal(
    instance: &Instance,
    opaque: &OpaquePtr,
    data: &[u8],
    component_size: usize,
    name: &str,
) -> Result<()> {
    if let Ok(set_input_model_data) = instance
        .exports
        .get_native_function::<(OpaquePtr, ByteArray, u32, StatusPtr), ()>(name)
    {
        let data_ptr = allocate_byte_array_with_data(instance, data)?;
        let status_ptr = allocate_status_ptr(instance)?;
        set_input_model_data.call(
            *opaque,
            data_ptr,
            if component_size > 1 {
                (data.len() / component_size) as u32
            } else {
                data.len() as u32
            },
            status_ptr,
        )?;
        release_byte_array(instance, data_ptr)?;
        release_status_ptr(instance, status_ptr)?;
    } else {
        notify_export_function_error(name);
    }
    Ok(())
}

pub(crate) fn initialize_env_logger() {
    tracing_subscriber::fmt::try_init().unwrap_or_default();
}

pub(crate) fn inner_memory(instance: &Instance) -> &Memory {
    instance.exports.get_memory("memory").unwrap()
}

pub(crate) fn allocate_byte_array(instance: &Instance, data_size: u32) -> Result<ByteArray> {
    let malloc_func = instance
        .exports
        .get_native_function::<u32, ByteArray>(MALLOC_FN)?;
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
    let malloc_func = instance
        .exports
        .get_native_function::<u32, StatusPtr>(MALLOC_FN)?;
    let data_ptr = malloc_func.call(size_of::<u32>() as u32)?;
    data_ptr.deref(memory).unwrap().set(0);
    Ok(data_ptr)
}

pub(crate) fn allocate_size_ptr(instance: &Instance) -> Result<SizePtr> {
    let memory = inner_memory(instance);
    let malloc_func = instance
        .exports
        .get_native_function::<u32, SizePtr>(MALLOC_FN)?;
    let size_ptr = malloc_func.call(size_of::<u32>() as u32)?;
    size_ptr.deref(memory).unwrap().set(0);
    Ok(size_ptr)
}

pub(crate) fn release_byte_array(instance: &Instance, ptr: ByteArray) -> Result<()> {
    let free_func = instance
        .exports
        .get_native_function::<ByteArray, ()>(FREE_FN)
        .unwrap();
    free_func.call(ptr)?;
    Ok(())
}

pub(crate) fn release_size_ptr(instance: &Instance, ptr: SizePtr) -> Result<()> {
    let free_func = instance
        .exports
        .get_native_function::<SizePtr, ()>(FREE_FN)
        .unwrap();
    free_func.call(ptr)?;
    Ok(())
}

pub(crate) fn release_status_ptr(instance: &Instance, ptr: StatusPtr) -> Result<()> {
    let free_func = instance
        .exports
        .get_native_function::<StatusPtr, ()>(FREE_FN)
        .unwrap();
    let status = ptr.deref(inner_memory(instance)).unwrap().get();
    free_func.call(ptr)?;
    if status != 0 {
        return Err(anyhow::anyhow!("code={}", status));
    }
    Ok(())
}

pub(crate) fn inner_initialize_function(instance: &Instance, name: &str) -> Result<()> {
    if let Ok(initialize) = instance.exports.get_native_function::<(), ()>(name) {
        initialize.call()?;
    } else {
        notify_export_function_error(name);
    }
    Ok(())
}

pub(crate) fn inner_create_opaque(instance: &Instance, name: &str) -> Result<OpaquePtr> {
    let create = instance
        .exports
        .get_native_function::<(), OpaquePtr>(name)?;
    Ok(create.call()?)
}

pub(crate) fn inner_destroy_opaque(instance: &Instance, opaque: &Option<OpaquePtr>, name: &str) {
    if let Some(opaque) = opaque {
        if let Ok(destroy) = instance.exports.get_native_function::<OpaquePtr, ()>(name) {
            if let Err(err) = destroy.call(*opaque) {
                warn!("runtime error at calling {}: {}", name, err);
            }
        }
    }
}

pub(crate) fn inner_terminate_function(instance: &Instance, name: &str) {
    if let Ok(terminate) = instance.exports.get_native_function::<(), ()>(name) {
        terminate.call().unwrap();
    } else {
        notify_export_function_error(name);
    }
}

pub(crate) fn inner_get_string(
    instance: &Instance,
    opaque: &Option<OpaquePtr>,
    name: &str,
) -> Result<String> {
    if let Some(opaque) = opaque {
        if let Ok(get_string) = instance
            .exports
            .get_native_function::<OpaquePtr, ByteArray>(name)
        {
            let memory = inner_memory(instance);
            Ok(get_string
                .call(*opaque)?
                .get_utf8_string_with_nul(memory)
                .unwrap_or_default())
        } else {
            Ok(Default::default())
        }
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
        let get_data_size = instance.exports.get_native_function(size_func_name);
        let get_data_body = instance.exports.get_native_function(body_func_name);
        let mut output_data = Vec::new();
        if get_data_size.is_ok() && get_data_body.is_ok() {
            let get_data_size = get_data_size?;
            let get_data_body = get_data_body?;
            inner_get_data_internal(
                instance,
                opaque,
                get_data_size,
                get_data_body,
                &mut output_data,
            )?;
        }
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
        let component_size = size_of::<T>();
        let len = data.len() * component_size;
        let data = unsafe { slice::from_raw_parts(data.as_ptr() as *const u8, len) };
        inner_set_data_internal(instance, opaque, data, component_size, name)?;
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
        let set_language = instance
            .exports
            .get_native_function::<(OpaquePtr, i32), ()>(name)?;
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
        let count_all_functions = instance
            .exports
            .get_native_function::<OpaquePtr, i32>(name)?;
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
        let get_function_name = instance
            .exports
            .get_native_function::<(OpaquePtr, i32), ByteArray>(name)?;
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
        let status_ptr = allocate_status_ptr(instance)?;
        let set_function = instance
            .exports
            .get_native_function::<(OpaquePtr, i32, StatusPtr), ()>(name)?;
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
        let execute = instance
            .exports
            .get_native_function::<(OpaquePtr, StatusPtr), ()>(name)?;
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
        if let Ok(load_ui_window_layout) = instance
            .exports
            .get_native_function::<(OpaquePtr, StatusPtr), ()>(name)
        {
            let status_ptr = allocate_status_ptr(instance)?;
            load_ui_window_layout.call(*opaque, status_ptr)?;
            release_status_ptr(instance, status_ptr)?;
        }
    }
    Ok(())
}

pub(crate) fn inner_set_ui_component_layout(
    instance: &Instance,
    opaque: &Option<OpaquePtr>,
    id: &str,
    data: &[u8],
    name: &str,
    reload: &mut bool,
) -> Result<()> {
    if let Some(opaque) = opaque {
        if let Ok(set_ui_component_layout) = instance.exports.get_native_function::<(
            OpaquePtr,
            ByteArray,
            ByteArray,
            u32,
            SizePtr,
            StatusPtr,
        ), ()>(name)
        {
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
            *reload = false;
            release_byte_array(instance, id_ptr)?;
            release_byte_array(instance, data_ptr)?;
            release_size_ptr(instance, reload_layout_ptr)?;
            release_status_ptr(instance, status_ptr)?;
        } else {
            notify_export_function_error(name);
        }
    }
    Ok(())
}

mod model;
mod motion;
