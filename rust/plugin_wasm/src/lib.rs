/*
  Copyright (c) 2015-2023 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

use core::slice;
use std::mem::size_of;

use anyhow::Result;
use wasmer::{Instance, Memory, Store, TypedFunction, WasmPtr};

pub(crate) const PLUGIN_MODEL_IO_ABI_VERSION: u32 = 2 << 16;
pub(crate) const PLUGIN_MOTION_IO_ABI_VERSION: u32 = 2 << 16;

pub(crate) const PLUGIN_NAME: &[u8] = b"plugin_wasm\0";
pub(crate) const PLUGIN_DESCRIPTION: &[u8] = b"nanoem WASM Plugin Loader\0";
pub(crate) const PLUGIN_VERSION: &[u8] = b"1.0.0\0";

const MALLOC_FN: &str = "nanoemApplicationPluginAllocateMemoryWASM";
const FREE_FN: &str = "nanoemApplicationPluginReleaseMemoryWASM";

#[allow(dead_code, non_camel_case_types)]
#[derive(Copy, Clone, Debug, Eq, Hash, PartialEq, PartialOrd)]
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

pub(crate) type ByteArray = WasmPtr<u8>;
pub(crate) type OpaquePtr = WasmPtr<u8>;
pub(crate) type SizePtr = WasmPtr<u32>;
pub(crate) type StatusPtr = WasmPtr<i32>;

fn notify_export_function_error(name: &str) {
    tracing::warn!(
        name = name,
        "Cannot be called due to not found or signature unmatched"
    );
}

fn inner_get_data_internal(
    instance: &Instance,
    opaque: &OpaquePtr,
    get_data_size: TypedFunction<(OpaquePtr, SizePtr), ()>,
    get_data_body: TypedFunction<(OpaquePtr, ByteArray, u32, StatusPtr), ()>,
    output_data: &mut Vec<u8>,
    store: &mut Store,
) -> Result<()> {
    let data_size_ptr = allocate_size_ptr(instance, store)?;
    get_data_size.call(store, *opaque, data_size_ptr)?;
    let data_size = data_size_ptr
        .deref(&inner_memory(instance).view(store))
        .read()?;
    let bytes = allocate_byte_array(instance, data_size, store)?;
    let status_ptr = allocate_status_ptr(instance, store)?;
    get_data_body.call(store, *opaque, bytes, data_size, status_ptr)?;
    let cell = bytes.deref(&inner_memory(instance).view(store)).as_ptr32();
    output_data.resize(data_size as usize, 0);
    let view = inner_memory(instance).view(store);
    let slice = cell.slice(&view, data_size)?;
    for byte in slice.iter() {
        output_data.push(byte.read()?);
    }
    release_byte_array(instance, bytes, store)?;
    release_size_ptr(instance, data_size_ptr, store)?;
    release_status_ptr(instance, status_ptr, store)?;
    Ok(())
}

fn inner_set_data_internal(
    instance: &Instance,
    opaque: &OpaquePtr,
    data: &[u8],
    component_size: usize,
    name: &str,
    store: &mut Store,
) -> Result<()> {
    if let Ok(set_input_model_data) = instance
        .exports
        .get_typed_function::<(OpaquePtr, ByteArray, u32, StatusPtr), ()>(store, name)
    {
        let data_ptr = allocate_byte_array_with_data(instance, data, store)?;
        let status_ptr = allocate_status_ptr(instance, store)?;
        set_input_model_data.call(
            store,
            *opaque,
            data_ptr,
            if component_size > 1 {
                (data.len() / component_size) as u32
            } else {
                data.len() as u32
            },
            status_ptr,
        )?;
        release_byte_array(instance, data_ptr, store)?;
        release_status_ptr(instance, status_ptr, store)?;
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

pub(crate) fn allocate_byte_array(
    instance: &Instance,
    data_size: u32,
    store: &mut Store,
) -> Result<ByteArray> {
    let malloc_func = instance
        .exports
        .get_typed_function::<u32, ByteArray>(store, MALLOC_FN)?;
    let data_ptr = malloc_func.call(store, data_size)?;
    tracing::debug!(size = data_size, "Allocated byte array");
    Ok(data_ptr)
}

pub(crate) fn allocate_byte_array_with_data(
    instance: &Instance,
    data: &[u8],
    store: &mut Store,
) -> Result<ByteArray> {
    let data_size = data.len() as u32;
    let data_ptr = allocate_byte_array(instance, data_size, store)?;
    let view = inner_memory(instance).view(store);
    let cell = data_ptr.deref(&view);
    let ptr = cell.as_ptr32();
    for (offset, byte) in data.iter().enumerate() {
        ptr.add_offset(offset as u32)?.write(&view, *byte)?;
    }
    tracing::debug!(size = data_size, "Allocated byte array with data");
    Ok(data_ptr)
}

pub(crate) fn allocate_status_ptr(instance: &Instance, store: &mut Store) -> Result<StatusPtr> {
    let malloc_func = instance
        .exports
        .get_typed_function::<u32, StatusPtr>(store, MALLOC_FN)?;
    let data_ptr = malloc_func.call(store, size_of::<u32>() as u32)?;
    let view = inner_memory(instance).view(store);
    data_ptr.deref(&view).write(0)?;
    Ok(data_ptr)
}

pub(crate) fn allocate_size_ptr(instance: &Instance, store: &mut Store) -> Result<SizePtr> {
    let memory = inner_memory(instance);
    let malloc_func = instance
        .exports
        .get_typed_function::<u32, SizePtr>(store, MALLOC_FN)?;
    let size_ptr = malloc_func.call(store, size_of::<u32>() as u32)?;
    let view = memory.view(store);
    size_ptr.deref(&view).write(0)?;
    Ok(size_ptr)
}

pub(crate) fn release_byte_array(
    instance: &Instance,
    ptr: ByteArray,
    store: &mut Store,
) -> Result<()> {
    let free_func = instance
        .exports
        .get_typed_function::<ByteArray, ()>(store, FREE_FN)
        .unwrap();
    free_func.call(store, ptr)?;
    tracing::debug!("Released byte array");
    Ok(())
}

pub(crate) fn release_size_ptr(instance: &Instance, ptr: SizePtr, store: &mut Store) -> Result<()> {
    let free_func = instance
        .exports
        .get_typed_function::<SizePtr, ()>(store, FREE_FN)
        .unwrap();
    free_func.call(store, ptr)?;
    Ok(())
}

pub(crate) fn release_status_ptr(
    instance: &Instance,
    ptr: StatusPtr,
    store: &mut Store,
) -> Result<()> {
    let free_func = instance
        .exports
        .get_typed_function::<StatusPtr, ()>(store, FREE_FN)
        .unwrap();
    free_func.call(store, ptr)?;
    Ok(())
}

pub(crate) fn inner_initialize_function(
    instance: &Instance,
    name: &str,
    store: &mut Store,
) -> Result<()> {
    if let Ok(initialize) = instance.exports.get_typed_function::<(), ()>(store, name) {
        initialize.call(store)?;
        tracing::debug!(name = name, "Called initialization");
    } else {
        notify_export_function_error(name);
    }
    Ok(())
}

pub(crate) fn inner_create_opaque(
    instance: &Instance,
    name: &str,
    store: &mut Store,
) -> Result<OpaquePtr> {
    let create = instance
        .exports
        .get_typed_function::<(), OpaquePtr>(store, name)?;
    let opaque = create.call(store)?;
    tracing::debug!(name = name, opaque = ?opaque, "Called creating opaque");
    Ok(opaque)
}

pub(crate) fn inner_destroy_opaque(
    instance: &Instance,
    opaque: &Option<OpaquePtr>,
    name: &str,
    store: &mut Store,
) {
    if let Some(opaque) = opaque {
        if let Ok(destroy) = instance
            .exports
            .get_typed_function::<OpaquePtr, ()>(store, name)
        {
            match destroy.call(store, *opaque) {
                Ok(_) => tracing::debug!(name = name, opaque = ?opaque, "Called destroying opaque"),
                Err(err) => {
                    tracing::warn!(name = name, opaque = ?opaque, error = %err, "Catched runtime error")
                }
            }
        }
    }
}

pub(crate) fn inner_terminate_function(instance: &Instance, name: &str, store: &mut Store) {
    if let Ok(terminate) = instance.exports.get_typed_function::<(), ()>(store, name) {
        terminate.call(store).unwrap();
        tracing::debug!(name = name, "Called termination");
    } else {
        notify_export_function_error(name);
    }
}

pub(crate) fn inner_get_string(
    instance: &Instance,
    opaque: &Option<OpaquePtr>,
    name: &str,
    store: &mut Store,
) -> Result<String> {
    if let Some(opaque) = opaque {
        if let Ok(get_string) = instance
            .exports
            .get_typed_function::<OpaquePtr, ByteArray>(store, name)
        {
            let memory = inner_memory(instance);
            let value = get_string
                .call(store, *opaque)?
                .read_utf8_string_with_nul(&memory.view(store))
                .unwrap_or_default();
            tracing::debug!(
                name = name,
                opaque = ?opaque,
                value = value,
                "Called getting string value"
            );
            Ok(value)
        } else {
            notify_export_function_error(name);
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
    store: &mut Store,
) -> Result<Vec<u8>> {
    if let Some(opaque) = opaque {
        let get_data_size = instance.exports.get_typed_function(store, size_func_name);
        let get_data_body = instance.exports.get_typed_function(store, body_func_name);
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
                store,
            )?;
            tracing::debug!(
                name = body_func_name,
                opaque = ?opaque,
                size = output_data.len(),
                "Called getting data"
            );
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
    store: &mut Store,
) -> Result<()> {
    if let Some(opaque) = opaque {
        let component_size = size_of::<T>();
        let len = data.len() * component_size;
        let data = unsafe { slice::from_raw_parts(data.as_ptr() as *const u8, len) };
        inner_set_data_internal(instance, opaque, data, component_size, name, store)?;
        tracing::debug!(name = name, opaque = ?opaque, size = len, "Called setting data");
    }
    Ok(())
}

pub(crate) fn inner_set_language(
    instance: &Instance,
    opaque: &Option<OpaquePtr>,
    value: i32,
    name: &str,
    store: &mut Store,
) -> Result<()> {
    if let Some(opaque) = opaque {
        let set_language = instance
            .exports
            .get_typed_function::<(OpaquePtr, i32), ()>(store, name)?;
        set_language.call(store, *opaque, value)?;
        tracing::debug!(name = name, opaque = ?opaque, value = value, "Called setting language");
    }
    Ok(())
}

pub(crate) fn inner_count_all_functions(
    instance: &Instance,
    opaque: &Option<OpaquePtr>,
    name: &str,
    store: &mut Store,
) -> Result<i32> {
    if let Some(opaque) = opaque {
        let count_all_functions = instance
            .exports
            .get_typed_function::<OpaquePtr, i32>(store, name)?;
        let count = count_all_functions.call(store, *opaque)?;
        tracing::debug!(name = name, opaque = ?opaque, count = count, "Called counting all functions");
        Ok(count)
    } else {
        Ok(0)
    }
}

pub(crate) fn inner_get_function_name(
    instance: &Instance,
    opaque: &Option<OpaquePtr>,
    index: i32,
    name: &str,
    store: &mut Store,
) -> Result<String> {
    if let Some(opaque) = opaque {
        let get_function_name = instance
            .exports
            .get_typed_function::<(OpaquePtr, i32), ByteArray>(store, name)?;
        let memory = inner_memory(instance);
        let function_name = get_function_name
            .call(store, *opaque, index)?
            .read_utf8_string_with_nul(&memory.view(store))
            .unwrap_or_default();
        tracing::debug!(
            name = name,
            opaque = ?opaque,
            index = index,
            function = function_name,
            "Called getting function name"
        );
        Ok(function_name)
    } else {
        Ok(Default::default())
    }
}

pub(crate) fn inner_set_function(
    instance: &Instance,
    opaque: &Option<OpaquePtr>,
    index: i32,
    name: &str,
    store: &mut Store,
) -> Result<i32> {
    let result = if let Some(opaque) = opaque {
        let status_ptr = allocate_status_ptr(instance, store)?;
        let set_function = instance
            .exports
            .get_typed_function::<(OpaquePtr, i32, StatusPtr), ()>(store, name)?;
        set_function.call(store, *opaque, index, status_ptr)?;
        let result = status_ptr.read(&inner_memory(instance).view(store))?;
        tracing::debug!(name = name, opaque = ?opaque, index = index, "Called setting function");
        release_status_ptr(instance, status_ptr, store)?;
        result
    } else {
        nanoem_application_plugin_status_t::ERROR_NULL_OBJECT as i32
    };
    Ok(result)
}

pub(crate) fn inner_execute(
    instance: &Instance,
    opaque: &Option<OpaquePtr>,
    name: &str,
    store: &mut Store,
) -> Result<i32> {
    let result = if let Some(opaque) = opaque {
        let status_ptr = allocate_status_ptr(instance, store)?;
        let execute = instance
            .exports
            .get_typed_function::<(OpaquePtr, StatusPtr), ()>(store, name)?;
        execute.call(store, *opaque, status_ptr)?;
        let result = status_ptr.read(&inner_memory(instance).view(store))?;
        tracing::debug!(name = name, opaque = ?opaque, "Called executing function");
        release_status_ptr(instance, status_ptr, store)?;
        result
    } else {
        nanoem_application_plugin_status_t::ERROR_NULL_OBJECT as i32
    };
    Ok(result)
}

pub(crate) fn inner_load_ui_window(
    instance: &Instance,
    opaque: &Option<OpaquePtr>,
    name: &str,
    store: &mut Store,
) -> Result<i32> {
    let result = if let Some(opaque) = opaque {
        if let Ok(load_ui_window_layout) = instance
            .exports
            .get_typed_function::<(OpaquePtr, StatusPtr), ()>(store, name)
        {
            let status_ptr = allocate_status_ptr(instance, store)?;
            load_ui_window_layout.call(store, *opaque, status_ptr)?;
            let result = status_ptr.read(&inner_memory(instance).view(store))?;
            tracing::debug!(name = name, opaque = ?opaque, "Called loading UI window");
            release_status_ptr(instance, status_ptr, store)?;
            result
        } else {
            nanoem_application_plugin_status_t::SUCCESS as i32
        }
    } else {
        nanoem_application_plugin_status_t::ERROR_NULL_OBJECT as i32
    };
    Ok(result)
}

pub(crate) fn inner_set_ui_component_layout(
    instance: &Instance,
    opaque: &Option<OpaquePtr>,
    id: &str,
    data: &[u8],
    name: &str,
    reload: &mut bool,
    store: &mut Store,
) -> Result<i32> {
    let result = if let Some(opaque) = opaque {
        if let Ok(set_ui_component_layout) = instance.exports.get_typed_function::<(
            OpaquePtr,
            ByteArray,
            ByteArray,
            u32,
            SizePtr,
            StatusPtr,
        ), ()>(store, name)
        {
            let id_ptr = allocate_byte_array_with_data(instance, id.as_bytes(), store)?;
            let data_ptr = allocate_byte_array_with_data(instance, data, store)?;
            let reload_layout_ptr = allocate_size_ptr(instance, store)?;
            let status_ptr = allocate_status_ptr(instance, store)?;
            set_ui_component_layout.call(
                store,
                *opaque,
                id_ptr,
                data_ptr,
                data.len() as u32,
                reload_layout_ptr,
                status_ptr,
            )?;
            *reload = false;
            let result = status_ptr.read(&inner_memory(instance).view(store))?;
            tracing::debug!(name = name, opaque = ?opaque, "Called setting UI component layout");
            release_byte_array(instance, id_ptr, store)?;
            release_byte_array(instance, data_ptr, store)?;
            release_size_ptr(instance, reload_layout_ptr, store)?;
            release_status_ptr(instance, status_ptr, store)?;
            result
        } else {
            notify_export_function_error(name);
            nanoem_application_plugin_status_t::SUCCESS as i32
        }
    } else {
        nanoem_application_plugin_status_t::ERROR_NULL_OBJECT as i32
    };
    Ok(result)
}

mod model;
mod motion;
