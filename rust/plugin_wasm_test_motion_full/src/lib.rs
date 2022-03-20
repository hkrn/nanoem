/*
  Copyright (c) 2015-2021 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

use serde_derive::{Deserialize, Serialize};
use serde_json::{json, Value};
use std::os::raw::{c_void, c_char};
use std::{collections::HashMap, ffi::CStr, ptr::null_mut};

#[allow(non_camel_case_types)]
pub type nanoem_application_plugin_status_t = i32;

#[allow(non_camel_case_types)]
pub struct nanoem_application_plugin_motion_io_t {}

macro_rules! function {
    () => {{
        fn f() {}
        fn type_name_of<T>(_: T) -> &'static str {
            std::any::type_name::<T>()
        }
        let name = type_name_of(f);
        &name[..name.len() - 3]
    }};
}

#[derive(Default, Serialize, Deserialize)]
struct Output {
    function: String,
    arguments: Option<HashMap<String, Value>>,
}

fn plugin_function_name(name: &str) -> String {
    let name = name.to_owned();
    let s: Vec<_> = name.split("::").collect();
    s[1].to_owned()
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOGetABIVersion() -> u32 {
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            ..Default::default()
        })
        .unwrap()
    );
    0
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOInitialize() {
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            ..Default::default()
        })
        .unwrap()
    )
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOCreate(
) -> *mut nanoem_application_plugin_motion_io_t {
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            ..Default::default()
        })
        .unwrap()
    );
    null_mut()
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOSetLanguage(
    _plugin: *mut nanoem_application_plugin_motion_io_t,
    value: i32,
) {
    let mut arguments = HashMap::new();
    arguments.insert("value".to_owned(), json!(value));
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            arguments: Some(arguments),
        })
        .unwrap()
    )
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOGetName(
    _plugin: *const nanoem_application_plugin_motion_io_t,
) -> *const i8 {
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            ..Default::default()
        })
        .unwrap()
    );
    b"plugin_wasm_test_motion_full\0" as *const u8 as *const i8
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOGetDescription(
    _plugin: *const nanoem_application_plugin_motion_io_t,
) -> *const i8 {
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            ..Default::default()
        })
        .unwrap()
    );
    b"This is plugin_wasm_test_motion_full\0" as *const u8 as *const i8
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOGetVersion(
    _plugin: *const nanoem_application_plugin_motion_io_t,
) -> *const i8 {
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            ..Default::default()
        })
        .unwrap()
    );
    b"1.2.3\0" as *const u8 as *const i8
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOCountAllFunctions(
    _plugin: *const nanoem_application_plugin_motion_io_t,
) -> i32 {
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            ..Default::default()
        })
        .unwrap()
    );
    1
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOGetFunctionName(
    _plugin: *const nanoem_application_plugin_motion_io_t,
    index: i32,
) -> *const i8 {
    let mut arguments = HashMap::new();
    arguments.insert("index".to_owned(), json!(index));
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            arguments: Some(arguments),
        })
        .unwrap()
    );
    b"function0\0" as *const u8 as *const i8
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOSetFunction(
    _plugin: *mut nanoem_application_plugin_motion_io_t,
    index: i32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let mut arguments = HashMap::new();
    arguments.insert("index".to_owned(), json!(index));
    arguments.insert("status".to_owned(), json!(*status_ptr));
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            arguments: Some(arguments),
        })
        .unwrap()
    )
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOSetAllNamedSelectedBoneKeyframes(
    _plugin: *mut nanoem_application_plugin_motion_io_t,
    name: *const i8,
    frame_indices: *const u32,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let mut arguments = HashMap::new();
    let name = CStr::from_ptr(name as *const c_char);
    let slice = std::slice::from_raw_parts(frame_indices, length as usize);
    arguments.insert("name".to_owned(), json!(name.to_str().unwrap_or_default()));
    arguments.insert("frameIndices".to_owned(), json!(slice));
    arguments.insert("length".to_owned(), json!(length));
    arguments.insert("status".to_owned(), json!(*status_ptr));
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            arguments: Some(arguments),
        })
        .unwrap()
    )
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOSetAllNamedSelectedMorphKeyframes(
    _plugin: *mut nanoem_application_plugin_motion_io_t,
    name: *const i8,
    frame_indices: *const u32,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let mut arguments = HashMap::new();
    let name = CStr::from_ptr(name as *const c_char);
    let slice = std::slice::from_raw_parts(frame_indices, length as usize);
    arguments.insert("name".to_owned(), json!(name.to_str().unwrap_or_default()));
    arguments.insert("frameIndices".to_owned(), json!(slice));
    arguments.insert("length".to_owned(), json!(length));
    arguments.insert("status".to_owned(), json!(*status_ptr));
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            arguments: Some(arguments),
        })
        .unwrap()
    )
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOSetAllSelectedAccessoryKeyframes(
    _plugin: *mut nanoem_application_plugin_motion_io_t,
    frame_indices: *const u32,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let mut arguments = HashMap::new();
    let slice = std::slice::from_raw_parts(frame_indices, length as usize);
    arguments.insert("frameIndices".to_owned(), json!(slice));
    arguments.insert("length".to_owned(), json!(length));
    arguments.insert("status".to_owned(), json!(*status_ptr));
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            arguments: Some(arguments),
        })
        .unwrap()
    )
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOSetAllSelectedCameraKeyframes(
    _plugin: *mut nanoem_application_plugin_motion_io_t,
    frame_indices: *const u32,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let mut arguments = HashMap::new();
    let slice = std::slice::from_raw_parts(frame_indices, length as usize);
    arguments.insert("frameIndices".to_owned(), json!(slice));
    arguments.insert("length".to_owned(), json!(length));
    arguments.insert("status".to_owned(), json!(*status_ptr));
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            arguments: Some(arguments),
        })
        .unwrap()
    )
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOSetAllSelectedLightKeyframes(
    _plugin: *mut nanoem_application_plugin_motion_io_t,
    frame_indices: *const u32,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let mut arguments = HashMap::new();
    let slice = std::slice::from_raw_parts(frame_indices, length as usize);
    arguments.insert("frameIndices".to_owned(), json!(slice));
    arguments.insert("length".to_owned(), json!(length));
    arguments.insert("status".to_owned(), json!(*status_ptr));
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            arguments: Some(arguments),
        })
        .unwrap()
    )
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOSetAllSelectedModelKeyframes(
    _plugin: *mut nanoem_application_plugin_motion_io_t,
    frame_indices: *const u32,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let mut arguments = HashMap::new();
    let slice = std::slice::from_raw_parts(frame_indices, length as usize);
    arguments.insert("frameIndices".to_owned(), json!(slice));
    arguments.insert("length".to_owned(), json!(length));
    arguments.insert("status".to_owned(), json!(*status_ptr));
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            arguments: Some(arguments),
        })
        .unwrap()
    )
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOSetAllSelectedSelfShadowKeyframes(
    _plugin: *mut nanoem_application_plugin_motion_io_t,
    frame_indices: *const u32,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let mut arguments = HashMap::new();
    let slice = std::slice::from_raw_parts(frame_indices, length as usize);
    arguments.insert("frameIndices".to_owned(), json!(slice));
    arguments.insert("length".to_owned(), json!(length));
    arguments.insert("status".to_owned(), json!(*status_ptr));
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            arguments: Some(arguments),
        })
        .unwrap()
    )
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOSetAudioDescription(
    _plugin: *mut nanoem_application_plugin_motion_io_t,
    data: *const u8,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let mut arguments = HashMap::new();
    let slice = std::slice::from_raw_parts(data, length as usize);
    arguments.insert("data".to_owned(), json!(slice));
    arguments.insert("length".to_owned(), json!(length));
    arguments.insert("status".to_owned(), json!(*status_ptr));
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            arguments: Some(arguments),
        })
        .unwrap()
    )
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOSetCameraDescription(
    _plugin: *mut nanoem_application_plugin_motion_io_t,
    data: *const u8,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let mut arguments = HashMap::new();
    let slice = std::slice::from_raw_parts(data, length as usize);
    arguments.insert("data".to_owned(), json!(slice));
    arguments.insert("length".to_owned(), json!(length));
    arguments.insert("status".to_owned(), json!(*status_ptr));
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            arguments: Some(arguments),
        })
        .unwrap()
    )
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOSetLightDescription(
    _plugin: *mut nanoem_application_plugin_motion_io_t,
    data: *const u8,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let mut arguments = HashMap::new();
    let slice = std::slice::from_raw_parts(data, length as usize);
    arguments.insert("data".to_owned(), json!(slice));
    arguments.insert("length".to_owned(), json!(length));
    arguments.insert("status".to_owned(), json!(*status_ptr));
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            arguments: Some(arguments),
        })
        .unwrap()
    )
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOSetInputAudioData(
    _plugin: *mut nanoem_application_plugin_motion_io_t,
    data: *const u8,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let mut arguments = HashMap::new();
    let slice = std::slice::from_raw_parts(data, length as usize);
    arguments.insert("data".to_owned(), json!(slice));
    arguments.insert("length".to_owned(), json!(length));
    arguments.insert("status".to_owned(), json!(*status_ptr));
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            arguments: Some(arguments),
        })
        .unwrap()
    )
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOSetInputMotionData(
    _plugin: *mut nanoem_application_plugin_motion_io_t,
    data: *const u8,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let mut arguments = HashMap::new();
    let slice = std::slice::from_raw_parts(data, length as usize);
    arguments.insert("data".to_owned(), json!(slice));
    arguments.insert("length".to_owned(), json!(length));
    arguments.insert("status".to_owned(), json!(*status_ptr));
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            arguments: Some(arguments),
        })
        .unwrap()
    )
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOSetInputActiveModelData(
    _plugin: *mut nanoem_application_plugin_motion_io_t,
    data: *const u8,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let mut arguments = HashMap::new();
    let slice = std::slice::from_raw_parts(data, length as usize);
    arguments.insert("data".to_owned(), json!(slice));
    arguments.insert("length".to_owned(), json!(length));
    arguments.insert("status".to_owned(), json!(*status_ptr));
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            arguments: Some(arguments),
        })
        .unwrap()
    )
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOExecute(
    _plugin: *mut nanoem_application_plugin_motion_io_t,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let mut arguments = HashMap::new();
    arguments.insert("status".to_owned(), json!(*status_ptr));
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            arguments: Some(arguments),
        })
        .unwrap()
    )
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOGetOutputMotionDataSize(
    _plugin: *mut nanoem_application_plugin_motion_io_t,
    length: *mut u32,
) {
    let mut arguments = HashMap::new();
    arguments.insert("length".to_owned(), json!(*length));
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            arguments: Some(arguments),
        })
        .unwrap()
    )
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOGetOutputMotionData(
    _plugin: *mut nanoem_application_plugin_motion_io_t,
    data: *mut u8,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let mut arguments = HashMap::new();
    let slice = std::slice::from_raw_parts(data, length as usize);
    arguments.insert("data".to_owned(), json!(slice));
    arguments.insert("length".to_owned(), json!(length));
    arguments.insert("status".to_owned(), json!(*status_ptr));
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            arguments: Some(arguments),
        })
        .unwrap()
    )
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOLoadUIWindowLayout(
    _plugin: *mut nanoem_application_plugin_motion_io_t,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let mut arguments = HashMap::new();
    arguments.insert("status".to_owned(), json!(*status_ptr));
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            arguments: Some(arguments),
        })
        .unwrap()
    )
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOGetUIWindowLayoutDataSize(
    _plugin: *mut nanoem_application_plugin_motion_io_t,
    length: *mut u32,
) {
    let mut arguments = HashMap::new();
    arguments.insert("length".to_owned(), json!(*length));
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            arguments: Some(arguments),
        })
        .unwrap()
    )
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOGetUIWindowLayoutData(
    _plugin: *mut nanoem_application_plugin_motion_io_t,
    data: *mut u8,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let mut arguments = HashMap::new();
    let slice = std::slice::from_raw_parts(data, length as usize);
    arguments.insert("data".to_owned(), json!(slice));
    arguments.insert("length".to_owned(), json!(length));
    arguments.insert("status".to_owned(), json!(*status_ptr));
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            arguments: Some(arguments),
        })
        .unwrap()
    )
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOSetUIComponentLayoutData(
    _plugin: *mut nanoem_application_plugin_motion_io_t,
    id: *const i8,
    data: *const u8,
    length: u32,
    _reload_layout: *mut i32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let mut arguments = HashMap::new();
    let id_str = CStr::from_ptr(id as *const c_char);
    let data_slice = std::slice::from_raw_parts(data, length as usize);
    arguments.insert("id".to_owned(), json!(id_str.to_str().unwrap_or_default()));
    arguments.insert("data".to_owned(), json!(data_slice));
    arguments.insert("length".to_owned(), json!(length));
    arguments.insert("status".to_owned(), json!(*status_ptr));
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            arguments: Some(arguments),
        })
        .unwrap()
    )
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOGetFailureReason(
    _plugin: *const nanoem_application_plugin_motion_io_t,
) -> *const i8 {
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            ..Default::default()
        })
        .unwrap()
    );
    b"Failure Reason\0" as *const u8 as *const i8
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOGetRecoverySuggestion(
    _plugin: *const nanoem_application_plugin_motion_io_t,
) -> *const i8 {
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            ..Default::default()
        })
        .unwrap()
    );
    b"Recovery Suggestion\0" as *const u8 as *const i8
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIODestroy(
    _plugin: *mut nanoem_application_plugin_motion_io_t,
) {
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            ..Default::default()
        })
        .unwrap()
    )
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOTerminate() {
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            ..Default::default()
        })
        .unwrap()
    )
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginAllocateMemoryWASM(size: usize) -> *mut c_void {
    libc::malloc(size)
}
/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginReleaseMemoryWASM(ptr: *mut c_void) {
    if !ptr.is_null() {
        libc::free(ptr);
    }
}
