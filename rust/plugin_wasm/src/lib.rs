/*
  Copyright (c) 2015-2023 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

use core::slice;
use std::mem::size_of;

use anyhow::Result;
use wasmtime::{AsContextMut, Instance, Memory, TypedFunc};
use zerocopy::AsBytes;

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

pub(crate) type ByteArray = i32; //WasmPtr<u8>;
pub(crate) type OpaquePtr = i32; //WasmPtr<u8>;
pub(crate) type SizePtr = i32; // WasmPtr<u32>;
pub(crate) type StatusPtr = i32; // WasmPtr<i32>;
type Store = wasmtime::Store<wasmtime_wasi::WasiCtx>;

fn notify_export_function_error(name: &str) {
    tracing::warn!(
        name = name,
        "Cannot be called due to not found or signature unmatched"
    );
}

fn inner_get_data_internal(
    instance: &Instance,
    opaque: &OpaquePtr,
    get_data_size: TypedFunc<(OpaquePtr, SizePtr), ()>,
    get_data_body: TypedFunc<(OpaquePtr, ByteArray, u32, StatusPtr), ()>,
    output_data: &mut Vec<u8>,
    mut store: impl AsContextMut,
) -> Result<()> {
    let data_size_ptr = allocate_size_ptr(instance, store.as_context_mut())?;
    get_data_size.call(store.as_context_mut(), (*opaque, data_size_ptr))?;
    let mut data_size = 0;
    inner_memory(instance, store.as_context_mut())?.read(
        store.as_context_mut(),
        data_size_ptr as usize,
        data_size.as_bytes_mut(),
    )?;
    let bytes = allocate_byte_array(instance, data_size, store.as_context_mut())?;
    let status_ptr = allocate_status_ptr(instance, store.as_context_mut())?;
    get_data_body.call(
        store.as_context_mut(),
        (*opaque, bytes, data_size, status_ptr),
    )?;
    output_data.resize(data_size as usize, 0);
    inner_memory(instance, store.as_context_mut())?.read(
        store.as_context_mut(),
        bytes as usize,
        output_data,
    )?;
    release_byte_array(instance, bytes, store.as_context_mut())?;
    release_size_ptr(instance, data_size_ptr, store.as_context_mut())?;
    release_status_ptr(instance, status_ptr, store.as_context_mut())?;
    Ok(())
}

fn inner_set_data_internal(
    instance: &Instance,
    opaque: &OpaquePtr,
    data: &[u8],
    component_size: usize,
    name: &str,
    mut store: impl AsContextMut,
) -> Result<()> {
    if let Ok(set_input_model_data) = instance
        .get_typed_func::<(OpaquePtr, ByteArray, u32, StatusPtr), ()>(store.as_context_mut(), name)
    {
        let data_ptr = allocate_byte_array_with_data(instance, data, store.as_context_mut())?;
        let status_ptr = allocate_status_ptr(instance, store.as_context_mut())?;
        set_input_model_data.call(
            store.as_context_mut(),
            (
                *opaque,
                data_ptr,
                if component_size > 1 {
                    (data.len() / component_size) as u32
                } else {
                    data.len() as u32
                },
                status_ptr,
            ),
        )?;
        release_byte_array(instance, data_ptr, store.as_context_mut())?;
        release_status_ptr(instance, status_ptr, store.as_context_mut())?;
    } else {
        notify_export_function_error(name);
    }
    Ok(())
}

pub(crate) fn initialize_env_logger() {
    tracing_subscriber::fmt::try_init().unwrap_or_default();
}

pub(crate) fn inner_memory(instance: &Instance, mut store: impl AsContextMut) -> Result<Memory> {
    instance
        .get_memory(store.as_context_mut(), "memory")
        .ok_or(anyhow::anyhow!("Cannot retrieve memory"))
}

pub(crate) fn allocate_byte_array(
    instance: &Instance,
    data_size: u32,
    mut store: impl AsContextMut,
) -> Result<ByteArray> {
    let malloc_func =
        instance.get_typed_func::<u32, ByteArray>(store.as_context_mut(), MALLOC_FN)?;
    let data_ptr = malloc_func.call(store.as_context_mut(), data_size)?;
    tracing::debug!(size = data_size, "Allocated byte array");
    Ok(data_ptr)
}

pub(crate) fn allocate_byte_array_with_data(
    instance: &Instance,
    data: &[u8],
    mut store: impl AsContextMut,
) -> Result<ByteArray> {
    let data_size = data.len() as u32;
    let data_ptr = allocate_byte_array(instance, data_size, store.as_context_mut())?;
    inner_memory(instance, store.as_context_mut())?.write(
        store.as_context_mut(),
        data_ptr as usize,
        data,
    )?;
    tracing::debug!(size = data_size, "Allocated byte array with data");
    Ok(data_ptr)
}

pub(crate) fn allocate_status_ptr(
    instance: &Instance,
    mut store: impl AsContextMut,
) -> Result<StatusPtr> {
    let malloc_func =
        instance.get_typed_func::<u32, StatusPtr>(store.as_context_mut(), MALLOC_FN)?;
    let data_ptr = malloc_func.call(store.as_context_mut(), size_of::<u32>() as u32)?;
    inner_memory(instance, store.as_context_mut())?.write(
        store.as_context_mut(),
        data_ptr as usize,
        0u32.as_bytes(),
    )?;
    Ok(data_ptr)
}

pub(crate) fn allocate_size_ptr(
    instance: &Instance,
    mut store: impl AsContextMut,
) -> Result<SizePtr> {
    let malloc_func = instance.get_typed_func::<u32, SizePtr>(store.as_context_mut(), MALLOC_FN)?;
    let size_ptr = malloc_func.call(store.as_context_mut(), size_of::<u32>() as u32)?;
    inner_memory(instance, store.as_context_mut())?.write(
        store.as_context_mut(),
        size_ptr as usize,
        0u32.as_bytes(),
    )?;
    Ok(size_ptr)
}

pub(crate) fn read_utf8_string(
    memory: Memory,
    ptr: ByteArray,
    mut store: impl AsContextMut,
) -> Result<String> {
    if let Some(size) = memory
        .data(store.as_context_mut())
        .get(ptr as usize..)
        .ok_or(anyhow::anyhow!(
            "Cannot retrieve range from memory: {:x}",
            ptr
        ))?
        .iter()
        .position(|i| *i == 0)
    {
        let mut value = Vec::new();
        value.resize(size, 0);
        memory.read(store.as_context_mut(), ptr as usize, value.as_mut())?;
        Ok(String::from_utf8(value)?)
    } else {
        Err(anyhow::anyhow!("Cannot find null terminator: {:x}", ptr))
    }
}

pub(crate) fn release_byte_array(
    instance: &Instance,
    ptr: ByteArray,
    mut store: impl AsContextMut,
) -> Result<()> {
    let free_func = instance.get_typed_func::<ByteArray, ()>(store.as_context_mut(), FREE_FN)?;
    free_func.call(store.as_context_mut(), ptr)?;
    tracing::debug!("Released byte array");
    Ok(())
}

pub(crate) fn release_size_ptr(
    instance: &Instance,
    ptr: SizePtr,
    mut store: impl AsContextMut,
) -> Result<()> {
    let free_func = instance.get_typed_func::<SizePtr, ()>(store.as_context_mut(), FREE_FN)?;
    free_func.call(store.as_context_mut(), ptr)?;
    Ok(())
}

pub(crate) fn release_status_ptr(
    instance: &Instance,
    ptr: StatusPtr,
    mut store: impl AsContextMut,
) -> Result<()> {
    let free_func = instance.get_typed_func::<StatusPtr, ()>(store.as_context_mut(), FREE_FN)?;
    free_func.call(store.as_context_mut(), ptr)?;
    Ok(())
}

pub(crate) fn inner_initialize_function(
    instance: &Instance,
    name: &str,
    mut store: impl AsContextMut,
) -> Result<()> {
    if let Ok(initialize) = instance.get_typed_func::<(), ()>(store.as_context_mut(), name) {
        initialize.call(store.as_context_mut(), ())?;
        tracing::debug!(name = name, "Called initialization");
    } else {
        notify_export_function_error(name);
    }
    Ok(())
}

pub(crate) fn inner_create_opaque(
    instance: &Instance,
    name: &str,
    mut store: impl AsContextMut,
) -> Result<OpaquePtr> {
    let create = instance.get_typed_func::<(), OpaquePtr>(store.as_context_mut(), name)?;
    let opaque = create.call(store.as_context_mut(), ())?;
    tracing::debug!(name = name, opaque = ?opaque, "Called creating opaque");
    Ok(opaque)
}

pub(crate) fn inner_destroy_opaque(
    instance: &Instance,
    opaque: &Option<OpaquePtr>,
    name: &str,
    mut store: impl AsContextMut,
) {
    if let Some(opaque) = opaque {
        if let Ok(destroy) = instance.get_typed_func::<OpaquePtr, ()>(store.as_context_mut(), name)
        {
            match destroy.call(store.as_context_mut(), *opaque) {
                Ok(_) => tracing::debug!(name = name, opaque = ?opaque, "Called destroying opaque"),
                Err(err) => {
                    tracing::warn!(name = name, opaque = ?opaque, error = %err, "Catched runtime error")
                }
            }
        }
    }
}

pub(crate) fn inner_terminate_function(
    instance: &Instance,
    name: &str,
    mut store: impl AsContextMut,
) {
    if let Ok(terminate) = instance.get_typed_func::<(), ()>(store.as_context_mut(), name) {
        terminate
            .call(store.as_context_mut(), ())
            .unwrap_or_default();
        tracing::debug!(name = name, "Called termination");
    } else {
        notify_export_function_error(name);
    }
}

pub(crate) fn inner_get_string(
    instance: &Instance,
    opaque: &Option<OpaquePtr>,
    name: &str,
    mut store: impl AsContextMut,
) -> Result<String> {
    if let Some(opaque) = opaque {
        if let Ok(get_string) =
            instance.get_typed_func::<OpaquePtr, ByteArray>(store.as_context_mut(), name)
        {
            let ptr = get_string.call(store.as_context_mut(), *opaque)?;
            let value = read_utf8_string(
                inner_memory(instance, store.as_context_mut())?,
                ptr,
                store.as_context_mut(),
            )?;
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
    mut store: impl AsContextMut,
) -> Result<Vec<u8>> {
    if let Some(opaque) = opaque {
        let get_data_size = instance.get_typed_func(store.as_context_mut(), size_func_name);
        let get_data_body = instance.get_typed_func(store.as_context_mut(), body_func_name);
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
                store.as_context_mut(),
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
        let len = std::mem::size_of_val(data);
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
    mut store: impl AsContextMut,
) -> Result<()> {
    if let Some(opaque) = opaque {
        let set_language =
            instance.get_typed_func::<(OpaquePtr, i32), ()>(store.as_context_mut(), name)?;
        set_language.call(store.as_context_mut(), (*opaque, value))?;
        tracing::debug!(name = name, opaque = ?opaque, value = value, "Called setting language");
    }
    Ok(())
}

pub(crate) fn inner_count_all_functions(
    instance: &Instance,
    opaque: &Option<OpaquePtr>,
    name: &str,
    mut store: impl AsContextMut,
) -> Result<i32> {
    if let Some(opaque) = opaque {
        let count_all_functions =
            instance.get_typed_func::<OpaquePtr, i32>(store.as_context_mut(), name)?;
        let count = count_all_functions.call(store.as_context_mut(), *opaque)?;
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
    mut store: impl AsContextMut,
) -> Result<String> {
    if let Some(opaque) = opaque {
        let get_function_name =
            instance.get_typed_func::<(OpaquePtr, i32), ByteArray>(store.as_context_mut(), name)?;
        let ptr = get_function_name.call(store.as_context_mut(), (*opaque, index))?;
        let function_name = read_utf8_string(
            inner_memory(instance, store.as_context_mut())?,
            ptr,
            store.as_context_mut(),
        )?;
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
    mut store: impl AsContextMut,
) -> Result<i32> {
    let result = if let Some(opaque) = opaque {
        let status_ptr = allocate_status_ptr(instance, store.as_context_mut())?;
        let set_function = instance
            .get_typed_func::<(OpaquePtr, i32, StatusPtr), ()>(store.as_context_mut(), name)?;
        set_function.call(store.as_context_mut(), (*opaque, index, status_ptr))?;
        let mut result = 0;
        inner_memory(instance, store.as_context_mut())?.read(
            store.as_context_mut(),
            status_ptr as usize,
            result.as_bytes_mut(),
        )?;
        tracing::debug!(name = name, opaque = ?opaque, index = index, "Called setting function");
        release_status_ptr(instance, status_ptr, store.as_context_mut())?;
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
    mut store: impl AsContextMut,
) -> Result<i32> {
    let result = if let Some(opaque) = opaque {
        let status_ptr = allocate_status_ptr(instance, store.as_context_mut())?;
        let execute =
            instance.get_typed_func::<(OpaquePtr, StatusPtr), ()>(store.as_context_mut(), name)?;
        execute.call(store.as_context_mut(), (*opaque, status_ptr))?;
        let mut result = 0;
        inner_memory(instance, store.as_context_mut())?.read(
            store.as_context_mut(),
            status_ptr as usize,
            result.as_bytes_mut(),
        )?;
        tracing::debug!(name = name, opaque = ?opaque, "Called executing function");
        release_status_ptr(instance, status_ptr, store.as_context_mut())?;
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
    mut store: impl AsContextMut,
) -> Result<i32> {
    let result = if let Some(opaque) = opaque {
        if let Ok(load_ui_window_layout) =
            instance.get_typed_func::<(OpaquePtr, StatusPtr), ()>(store.as_context_mut(), name)
        {
            let status_ptr = allocate_status_ptr(instance, store.as_context_mut())?;
            load_ui_window_layout.call(store.as_context_mut(), (*opaque, status_ptr))?;
            let mut result = 0;
            inner_memory(instance, store.as_context_mut())?.read(
                store.as_context_mut(),
                status_ptr as usize,
                result.as_bytes_mut(),
            )?;
            tracing::debug!(name = name, opaque = ?opaque, "Called loading UI window");
            release_status_ptr(instance, status_ptr, store.as_context_mut())?;
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
    mut store: impl AsContextMut,
) -> Result<i32> {
    let result = if let Some(opaque) = opaque {
        if let Ok(set_ui_component_layout) =
            instance
                .get_typed_func::<(OpaquePtr, ByteArray, ByteArray, u32, SizePtr, StatusPtr), ()>(
                    store.as_context_mut(),
                    name,
                )
        {
            let id_ptr =
                allocate_byte_array_with_data(instance, id.as_bytes(), store.as_context_mut())?;
            let data_ptr = allocate_byte_array_with_data(instance, data, store.as_context_mut())?;
            let reload_layout_ptr = allocate_size_ptr(instance, store.as_context_mut())?;
            let status_ptr = allocate_status_ptr(instance, store.as_context_mut())?;
            set_ui_component_layout.call(
                store.as_context_mut(),
                (
                    *opaque,
                    id_ptr,
                    data_ptr,
                    data.len() as u32,
                    reload_layout_ptr,
                    status_ptr,
                ),
            )?;
            *reload = false;
            let mut result = 0;
            inner_memory(instance, store.as_context_mut())?.read(
                store.as_context_mut(),
                status_ptr as usize,
                result.as_bytes_mut(),
            )?;
            tracing::debug!(name = name, opaque = ?opaque, "Called setting UI component layout");
            release_byte_array(instance, id_ptr, store.as_context_mut())?;
            release_byte_array(instance, data_ptr, store.as_context_mut())?;
            release_size_ptr(instance, reload_layout_ptr, store.as_context_mut())?;
            release_status_ptr(instance, status_ptr, store.as_context_mut())?;
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
