/*
  Copyright (c) 2015-2021 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

use crate::initialize_env_logger;

use super::super::nanoem_application_plugin_status_t;
use super::super::PLUGIN_MOTION_IO_ABI_VERSION;
use super::core::nanoem_application_plugin_motion_io_t;

use std::ffi::CStr;
use std::os::raw::c_char;
use std::ptr::{null, null_mut};

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOGetABIVersion() -> u32 {
    PLUGIN_MOTION_IO_ABI_VERSION
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOInitialize() {
    initialize_env_logger();
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOCreate(
) -> *mut nanoem_application_plugin_motion_io_t {
    nanoemApplicationPluginMotionIOCreateWithLocation(null())
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOCreateWithLocation(
    path: *const i8,
) -> *mut nanoem_application_plugin_motion_io_t {
    let path = CStr::from_ptr(path as *const c_char);
    if let Ok(mut instance) = nanoem_application_plugin_motion_io_t::new(path) {
        let result = instance.create();
        if result.is_ok() {
            let plugin = Box::new(instance);
            return std::mem::transmute(plugin);
        }
    }
    null_mut()
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOSetLanguage(
    plugin: *mut nanoem_application_plugin_motion_io_t,
    value: i32,
) {
    if let Some(instance) = nanoem_application_plugin_motion_io_t::get(plugin) {
        instance.set_language(value).unwrap_or_default();
    }
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOGetName(
    plugin: *const nanoem_application_plugin_motion_io_t,
) -> *const i8 {
    match nanoem_application_plugin_motion_io_t::get(plugin) {
        Some(instance) => instance.name(),
        None => null(),
    }
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOGetDescription(
    plugin: *const nanoem_application_plugin_motion_io_t,
) -> *const i8 {
    match nanoem_application_plugin_motion_io_t::get(plugin) {
        Some(instance) => instance.description(),
        None => null(),
    }
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOGetVersion(
    plugin: *const nanoem_application_plugin_motion_io_t,
) -> *const i8 {
    match nanoem_application_plugin_motion_io_t::get(plugin) {
        Some(instance) => instance.version(),
        None => null(),
    }
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOCountAllFunctions(
    plugin: *const nanoem_application_plugin_motion_io_t,
) -> i32 {
    match nanoem_application_plugin_motion_io_t::get(plugin) {
        Some(instance) => instance.count_all_functions(),
        None => 0,
    }
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOGetFunctionName(
    plugin: *const nanoem_application_plugin_motion_io_t,
    index: i32,
) -> *const i8 {
    match nanoem_application_plugin_motion_io_t::get(plugin) {
        Some(instance) => instance.function_name(index),
        None => null(),
    }
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
    let status = match nanoem_application_plugin_motion_io_t::get_mut(plugin) {
        Some(instance) => match instance.set_function(index) {
            Ok(_) => nanoem_application_plugin_status_t::SUCCESS,
            Err(value) => instance.assign_failure_reason(value),
        },
        None => nanoem_application_plugin_status_t::ERROR_NULL_OBJECT,
    };
    status.assign(status_ptr)
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOSetAllNamedSelectedBoneKeyframes(
    plugin: *mut nanoem_application_plugin_motion_io_t,
    name: *const i8,
    frame_indices: *const u32,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_motion_io_t::get_mut(plugin) {
        Some(instance) => {
            let slice = if !frame_indices.is_null() && length > 0 {
                std::slice::from_raw_parts(frame_indices, length as usize)
            } else {
                &[]
            };
            match instance.set_all_named_selected_bone_keyframes(CStr::from_ptr(name as *const c_char), slice) {
                Ok(_) => nanoem_application_plugin_status_t::SUCCESS,
                Err(value) => instance.assign_failure_reason(value),
            }
        }
        None => nanoem_application_plugin_status_t::ERROR_NULL_OBJECT,
    };
    status.assign(status_ptr)
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOSetAllNamedSelectedMorphKeyframes(
    plugin: *mut nanoem_application_plugin_motion_io_t,
    name: *const i8,
    frame_indices: *const u32,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_motion_io_t::get_mut(plugin) {
        Some(instance) => {
            let slice = if !frame_indices.is_null() && length > 0 {
                std::slice::from_raw_parts(frame_indices, length as usize)
            } else {
                &[]
            };
            match instance.set_all_named_selected_morph_keyframes(CStr::from_ptr(name as *const c_char), slice) {
                Ok(_) => nanoem_application_plugin_status_t::SUCCESS,
                Err(value) => instance.assign_failure_reason(value),
            }
        }
        None => nanoem_application_plugin_status_t::ERROR_NULL_OBJECT,
    };
    status.assign(status_ptr)
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOSetAllSelectedAccessoryKeyframes(
    plugin: *mut nanoem_application_plugin_motion_io_t,
    frame_indices: *const u32,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_motion_io_t::get_mut(plugin) {
        Some(instance) => {
            let slice = if !frame_indices.is_null() && length > 0 {
                std::slice::from_raw_parts(frame_indices, length as usize)
            } else {
                &[]
            };
            match instance.set_all_selected_accessory_keyframes(slice) {
                Ok(_) => nanoem_application_plugin_status_t::SUCCESS,
                Err(value) => instance.assign_failure_reason(value),
            }
        }
        None => nanoem_application_plugin_status_t::ERROR_NULL_OBJECT,
    };
    status.assign(status_ptr)
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOSetAllSelectedCameraKeyframes(
    plugin: *mut nanoem_application_plugin_motion_io_t,
    frame_indices: *const u32,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_motion_io_t::get_mut(plugin) {
        Some(instance) => {
            let slice = if !frame_indices.is_null() && length > 0 {
                std::slice::from_raw_parts(frame_indices, length as usize)
            } else {
                &[]
            };
            match instance.set_all_selected_camera_keyframes(slice) {
                Ok(_) => nanoem_application_plugin_status_t::SUCCESS,
                Err(value) => instance.assign_failure_reason(value),
            }
        }
        None => nanoem_application_plugin_status_t::ERROR_NULL_OBJECT,
    };
    status.assign(status_ptr)
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOSetAllSelectedLightKeyframes(
    plugin: *mut nanoem_application_plugin_motion_io_t,
    frame_indices: *const u32,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_motion_io_t::get_mut(plugin) {
        Some(instance) => {
            let slice = if !frame_indices.is_null() && length > 0 {
                std::slice::from_raw_parts(frame_indices, length as usize)
            } else {
                &[]
            };
            match instance.set_all_selected_light_keyframes(slice) {
                Ok(_) => nanoem_application_plugin_status_t::SUCCESS,
                Err(value) => instance.assign_failure_reason(value),
            }
        }
        None => nanoem_application_plugin_status_t::ERROR_NULL_OBJECT,
    };
    status.assign(status_ptr)
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOSetAllSelectedModelKeyframes(
    plugin: *mut nanoem_application_plugin_motion_io_t,
    frame_indices: *const u32,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_motion_io_t::get_mut(plugin) {
        Some(instance) => {
            let slice = if !frame_indices.is_null() && length > 0 {
                std::slice::from_raw_parts(frame_indices, length as usize)
            } else {
                &[]
            };
            match instance.set_all_selected_model_keyframes(slice) {
                Ok(_) => nanoem_application_plugin_status_t::SUCCESS,
                Err(value) => instance.assign_failure_reason(value),
            }
        }
        None => nanoem_application_plugin_status_t::ERROR_NULL_OBJECT,
    };
    status.assign(status_ptr)
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOSetAllSelectedSelfShadowKeyframes(
    plugin: *mut nanoem_application_plugin_motion_io_t,
    frame_indices: *const u32,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_motion_io_t::get_mut(plugin) {
        Some(instance) => {
            let slice = if !frame_indices.is_null() && length > 0 {
                std::slice::from_raw_parts(frame_indices, length as usize)
            } else {
                &[]
            };
            match instance.set_all_selected_self_shadow_keyframes(slice) {
                Ok(_) => nanoem_application_plugin_status_t::SUCCESS,
                Err(value) => instance.assign_failure_reason(value),
            }
        }
        None => nanoem_application_plugin_status_t::ERROR_NULL_OBJECT,
    };
    status.assign(status_ptr)
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOSetAudioDescription(
    plugin: *mut nanoem_application_plugin_motion_io_t,
    data: *const u8,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_motion_io_t::get_mut(plugin) {
        Some(instance) => {
            if !data.is_null() && length > 0 {
                let slice = std::slice::from_raw_parts(data, length as usize);
                match instance.set_audio_description(slice) {
                    Ok(_) => nanoem_application_plugin_status_t::SUCCESS,
                    Err(value) => instance.assign_failure_reason(value),
                }
            } else {
                nanoem_application_plugin_status_t::ERROR_NULL_OBJECT
            }
        }
        None => nanoem_application_plugin_status_t::ERROR_NULL_OBJECT,
    };
    status.assign(status_ptr)
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOSetCameraDescription(
    plugin: *mut nanoem_application_plugin_motion_io_t,
    data: *const u8,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_motion_io_t::get_mut(plugin) {
        Some(instance) => {
            if !data.is_null() && length > 0 {
                let slice = std::slice::from_raw_parts(data, length as usize);
                match instance.set_camera_description(slice) {
                    Ok(_) => nanoem_application_plugin_status_t::SUCCESS,
                    Err(value) => instance.assign_failure_reason(value),
                }
            } else {
                nanoem_application_plugin_status_t::ERROR_NULL_OBJECT
            }
        }
        None => nanoem_application_plugin_status_t::ERROR_NULL_OBJECT,
    };
    status.assign(status_ptr)
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOSetLightDescription(
    plugin: *mut nanoem_application_plugin_motion_io_t,
    data: *const u8,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_motion_io_t::get_mut(plugin) {
        Some(instance) => {
            if !data.is_null() && length > 0 {
                let slice = std::slice::from_raw_parts(data, length as usize);
                match instance.set_light_description(slice) {
                    Ok(_) => nanoem_application_plugin_status_t::SUCCESS,
                    Err(value) => instance.assign_failure_reason(value),
                }
            } else {
                nanoem_application_plugin_status_t::ERROR_NULL_OBJECT
            }
        }
        None => nanoem_application_plugin_status_t::ERROR_NULL_OBJECT,
    };
    status.assign(status_ptr)
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOSetInputAudioData(
    plugin: *mut nanoem_application_plugin_motion_io_t,
    data: *const u8,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_motion_io_t::get_mut(plugin) {
        Some(instance) => {
            if !data.is_null() && length > 0 {
                let slice = std::slice::from_raw_parts(data, length as usize);
                match instance.set_audio_data(slice) {
                    Ok(_) => nanoem_application_plugin_status_t::SUCCESS,
                    Err(value) => instance.assign_failure_reason(value),
                }
            } else {
                nanoem_application_plugin_status_t::ERROR_NULL_OBJECT
            }
        }
        None => nanoem_application_plugin_status_t::ERROR_NULL_OBJECT,
    };
    status.assign(status_ptr)
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOSetInputMotionData(
    plugin: *mut nanoem_application_plugin_motion_io_t,
    data: *const u8,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_motion_io_t::get_mut(plugin) {
        Some(instance) => {
            if !data.is_null() && length > 0 {
                let slice = std::slice::from_raw_parts(data, length as usize);
                match instance.set_input_motion_data(slice) {
                    Ok(_) => nanoem_application_plugin_status_t::SUCCESS,
                    Err(value) => instance.assign_failure_reason(value),
                }
            } else {
                nanoem_application_plugin_status_t::ERROR_NULL_OBJECT
            }
        }
        None => nanoem_application_plugin_status_t::ERROR_NULL_OBJECT,
    };
    status.assign(status_ptr)
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOSetInputActiveModelData(
    plugin: *mut nanoem_application_plugin_motion_io_t,
    data: *const u8,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_motion_io_t::get_mut(plugin) {
        Some(instance) => {
            if !data.is_null() && length > 0 {
                let slice = std::slice::from_raw_parts(data, length as usize);
                match instance.set_input_model_data(slice) {
                    Ok(_) => nanoem_application_plugin_status_t::SUCCESS,
                    Err(value) => instance.assign_failure_reason(value),
                }
            } else {
                nanoem_application_plugin_status_t::ERROR_NULL_OBJECT
            }
        }
        None => nanoem_application_plugin_status_t::ERROR_NULL_OBJECT,
    };
    status.assign(status_ptr)
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOExecute(
    plugin: *mut nanoem_application_plugin_motion_io_t,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_motion_io_t::get_mut(plugin) {
        Some(instance) => match instance.execute() {
            Ok(_) => nanoem_application_plugin_status_t::SUCCESS,
            Err(value) => instance.assign_failure_reason(value),
        },
        None => nanoem_application_plugin_status_t::ERROR_NULL_OBJECT,
    };
    status.assign(status_ptr)
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOGetOutputMotionDataSize(
    plugin: *mut nanoem_application_plugin_motion_io_t,
    length: *mut u32,
) {
    if let Some(instance) = nanoem_application_plugin_motion_io_t::get(plugin) {
        if !length.is_null() {
            *length = instance.output_slice().len() as u32
        }
    }
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOGetOutputMotionData(
    plugin: *mut nanoem_application_plugin_motion_io_t,
    data: *mut u8,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_motion_io_t::get(plugin) {
        Some(instance) => {
            if !data.is_null() && length > 0 {
                let slice = instance.output_slice();
                std::ptr::copy(
                    slice.as_ptr(),
                    data,
                    std::cmp::min(length as usize, slice.len()),
                );
                nanoem_application_plugin_status_t::SUCCESS
            } else {
                nanoem_application_plugin_status_t::ERROR_NULL_OBJECT
            }
        }
        None => nanoem_application_plugin_status_t::ERROR_NULL_OBJECT,
    };
    status.assign(status_ptr)
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOLoadUIWindowLayout(
    plugin: *mut nanoem_application_plugin_motion_io_t,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_motion_io_t::get_mut(plugin) {
        Some(instance) => match instance.load_window_layout() {
            Ok(_) => nanoem_application_plugin_status_t::SUCCESS,
            Err(value) => instance.assign_failure_reason(value),
        },
        None => nanoem_application_plugin_status_t::ERROR_NULL_OBJECT,
    };
    status.assign(status_ptr)
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOGetUIWindowLayoutDataSize(
    plugin: *mut nanoem_application_plugin_motion_io_t,
    length: *mut u32,
) {
    if let Some(instance) = nanoem_application_plugin_motion_io_t::get(plugin) {
        if !length.is_null() {
            *length = instance.window_layout_data_slice().len() as u32
        }
    }
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOGetUIWindowLayoutData(
    plugin: *mut nanoem_application_plugin_motion_io_t,
    data: *mut u8,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_motion_io_t::get(plugin) {
        Some(instance) => {
            if !data.is_null() && length > 0 {
                let slice = instance.window_layout_data_slice();
                std::ptr::copy(
                    slice.as_ptr(),
                    data,
                    std::cmp::min(length as usize, slice.len()),
                );
                nanoem_application_plugin_status_t::SUCCESS
            } else {
                nanoem_application_plugin_status_t::ERROR_NULL_OBJECT
            }
        }
        None => nanoem_application_plugin_status_t::ERROR_NULL_OBJECT,
    };
    status.assign(status_ptr)
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOSetUIComponentLayoutData(
    plugin: *mut nanoem_application_plugin_motion_io_t,
    id: *const i8,
    data: *const u8,
    length: u32,
    reload_layout: *mut i32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_motion_io_t::get_mut(plugin) {
        Some(instance) => {
            let id = CStr::from_ptr(id as *const c_char);
            let slice = std::slice::from_raw_parts(data, length as usize);
            let mut reload = false;
            match instance.set_component_layout(id, slice, &mut reload) {
                Ok(_) => {
                    if !reload_layout.is_null() {
                        *reload_layout = reload as i32;
                    }
                    nanoem_application_plugin_status_t::SUCCESS
                }
                Err(value) => instance.assign_failure_reason(value),
            }
        }
        None => nanoem_application_plugin_status_t::ERROR_NULL_OBJECT,
    };
    status.assign(status_ptr)
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOGetFailureReason(
    plugin: *const nanoem_application_plugin_motion_io_t,
) -> *const i8 {
    match nanoem_application_plugin_motion_io_t::get(plugin) {
        Some(instance) => match instance.failure_reason() {
            Some(reason) => reason.as_ptr() as *const i8,
            None => null(),
        },
        None => null(),
    }
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOGetRecoverySuggestion(
    plugin: *const nanoem_application_plugin_motion_io_t,
) -> *const i8 {
    match nanoem_application_plugin_motion_io_t::get(plugin) {
        Some(instance) => match instance.recovery_suggestion() {
            Some(reason) => reason.as_ptr() as *const i8,
            None => null(),
        },
        None => null(),
    }
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIODestroy(
    plugin: *mut nanoem_application_plugin_motion_io_t,
) {
    if !plugin.is_null() {
        let _: Box<nanoem_application_plugin_motion_io_t> = std::mem::transmute(plugin);
    }
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginMotionIOTerminate() {}
