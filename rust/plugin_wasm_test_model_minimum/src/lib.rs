/*
  Copyright (c) 2015-2021 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

use serde_derive::{Deserialize, Serialize};
use serde_json::{json, Value};
use std::os::raw::c_void;
use std::{collections::HashMap, ptr::null_mut};

#[allow(non_camel_case_types)]
pub type nanoem_application_plugin_status_t = i32;

#[allow(non_camel_case_types)]
pub struct nanoem_application_plugin_model_io_t {}

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
pub unsafe extern "C" fn nanoemApplicationPluginModelIOCreate(
) -> *mut nanoem_application_plugin_model_io_t {
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
pub unsafe extern "C" fn nanoemApplicationPluginModelIOSetLanguage(
    _plugin: *mut nanoem_application_plugin_model_io_t,
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
pub unsafe extern "C" fn nanoemApplicationPluginModelIOGetName(
    _plugin: *const nanoem_application_plugin_model_io_t,
) -> *const i8 {
    println!(
        "{}",
        serde_json::to_string(&Output {
            function: plugin_function_name(function!()),
            ..Default::default()
        })
        .unwrap()
    );
    b"plugin_wasm_test_model_minimum\0" as *const u8 as *const i8
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginModelIOGetVersion(
    _plugin: *const nanoem_application_plugin_model_io_t,
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
pub unsafe extern "C" fn nanoemApplicationPluginModelIOCountAllFunctions(
    _plugin: *const nanoem_application_plugin_model_io_t,
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
pub unsafe extern "C" fn nanoemApplicationPluginModelIOGetFunctionName(
    _plugin: *const nanoem_application_plugin_model_io_t,
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
pub unsafe extern "C" fn nanoemApplicationPluginModelIOSetFunction(
    _plugin: *mut nanoem_application_plugin_model_io_t,
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
pub unsafe extern "C" fn nanoemApplicationPluginModelIOSetInputModelData(
    _plugin: *mut nanoem_application_plugin_model_io_t,
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
pub unsafe extern "C" fn nanoemApplicationPluginModelIOExecute(
    _plugin: *mut nanoem_application_plugin_model_io_t,
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
pub unsafe extern "C" fn nanoemApplicationPluginModelIOGetOutputModelDataSize(
    _plugin: *mut nanoem_application_plugin_model_io_t,
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
pub unsafe extern "C" fn nanoemApplicationPluginModelIOGetOutputModelData(
    _plugin: *mut nanoem_application_plugin_model_io_t,
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
pub unsafe extern "C" fn nanoemApplicationPluginModelIOGetFailureReason(
    _plugin: *const nanoem_application_plugin_model_io_t,
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
pub unsafe extern "C" fn nanoemApplicationPluginModelIODestroy(
    _plugin: *mut nanoem_application_plugin_model_io_t,
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
