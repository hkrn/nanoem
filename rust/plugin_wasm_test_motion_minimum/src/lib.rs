/*
  Copyright (c) 2015-2023 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

use serde::{Deserialize, Serialize};
use serde_json::{json, Value};
use std::collections::HashMap;
use std::ffi::c_char;
use std::os::raw::c_void;

#[allow(non_camel_case_types)]
pub type nanoem_application_plugin_status_t = i32;

#[allow(non_camel_case_types)]
#[derive(Default)]
pub struct nanoem_application_plugin_motion_io_t {
    function_index: i32,
}

impl nanoem_application_plugin_motion_io_t {
    pub(self) unsafe fn get_mut(plugin: *mut Self) -> Option<&'static mut Self> {
        if !plugin.is_null() {
            Some(&mut (*plugin))
        } else {
            None
        }
    }
}

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
    let plugin = Box::<nanoem_application_plugin_motion_io_t>::default();
    std::mem::transmute(plugin)
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
) -> *const c_char {
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            ..Default::default()
        })
        .unwrap()
    );
    b"plugin_wasm_test_motion_minimum\0" as *const u8 as *const c_char
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOGetDescription(
    _plugin: *const nanoem_application_plugin_motion_io_t,
) -> *const c_char {
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            ..Default::default()
        })
        .unwrap()
    );
    b"This is plugin_wasm_test_motion_minimum\0" as *const u8 as *const c_char
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOGetVersion(
    _plugin: *const nanoem_application_plugin_motion_io_t,
) -> *const c_char {
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            ..Default::default()
        })
        .unwrap()
    );
    b"1.2.3\0" as *const u8 as *const c_char
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
    2
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOGetFunctionName(
    _plugin: *const nanoem_application_plugin_motion_io_t,
    index: i32,
) -> *const c_char {
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
    b"function0\0" as *const u8 as *const c_char
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOSetFunction(
    plugin: *mut nanoem_application_plugin_motion_io_t,
    index: i32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let mut arguments = HashMap::new();
    arguments.insert("index".to_owned(), json!(index));
    arguments.insert("status".to_owned(), json!(*status_ptr));
    if let Some(plugin) = nanoem_application_plugin_motion_io_t::get_mut(plugin) {
        plugin.function_index = index;
    }
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
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOExecute(
    plugin: *mut nanoem_application_plugin_motion_io_t,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let mut arguments = HashMap::new();
    if let Some(plugin) = nanoem_application_plugin_motion_io_t::get_mut(plugin) {
        if plugin.function_index == 1 {
            *status_ptr = -1;
        }
    }
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
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOGetFailureReason(
    _plugin: *const nanoem_application_plugin_motion_io_t,
) -> *const c_char {
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            ..Default::default()
        })
        .unwrap()
    );
    b"Failure Reason\0" as *const u8 as *const c_char
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIODestroy(
    plugin: *mut nanoem_application_plugin_motion_io_t,
) {
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            ..Default::default()
        })
        .unwrap()
    );
    if !plugin.is_null() {
        let _: Box<nanoem_application_plugin_motion_io_t> = std::mem::transmute(plugin);
    }
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
