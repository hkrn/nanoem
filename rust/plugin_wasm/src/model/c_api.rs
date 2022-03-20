/*
  Copyright (c) 2015-2021 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

use crate::initialize_env_logger;

use super::super::nanoem_application_plugin_status_t;
use super::super::PLUGIN_MODEL_IO_ABI_VERSION;
use super::core::nanoem_application_plugin_model_io_t;

use std::ffi::CStr;
use std::os::raw::c_char;
use std::ptr::{null, null_mut};

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginModelIOGetABIVersion() -> u32 {
    PLUGIN_MODEL_IO_ABI_VERSION
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginModelIOInitialize() {
    initialize_env_logger();
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginModelIOCreate(
) -> *mut nanoem_application_plugin_model_io_t {
    nanoemApplicationPluginModelIOCreateWithLocation(null())
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginModelIOCreateWithLocation(
    path: *const i8,
) -> *mut nanoem_application_plugin_model_io_t {
    let path = CStr::from_ptr(path as *const c_char);
    if let Ok(mut instance) = nanoem_application_plugin_model_io_t::new(path) {
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
pub unsafe extern "C" fn nanoemApplicationPluginModelIOSetLanguage(
    plugin: *mut nanoem_application_plugin_model_io_t,
    value: i32,
) {
    if let Some(instance) = nanoem_application_plugin_model_io_t::get(plugin) {
        instance.set_language(value).unwrap_or_default();
    }
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginModelIOGetName(
    plugin: *const nanoem_application_plugin_model_io_t,
) -> *const i8 {
    match nanoem_application_plugin_model_io_t::get(plugin) {
        Some(instance) => instance.name(),
        None => null(),
    }
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginModelIOGetDescription(
    plugin: *const nanoem_application_plugin_model_io_t,
) -> *const i8 {
    match nanoem_application_plugin_model_io_t::get(plugin) {
        Some(instance) => instance.description(),
        None => null(),
    }
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginModelIOGetVersion(
    plugin: *const nanoem_application_plugin_model_io_t,
) -> *const i8 {
    match nanoem_application_plugin_model_io_t::get(plugin) {
        Some(instance) => instance.version(),
        None => null(),
    }
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginModelIOCountAllFunctions(
    plugin: *const nanoem_application_plugin_model_io_t,
) -> i32 {
    match nanoem_application_plugin_model_io_t::get(plugin) {
        Some(instance) => instance.count_all_functions(),
        None => 0,
    }
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginModelIOGetFunctionName(
    plugin: *const nanoem_application_plugin_model_io_t,
    index: i32,
) -> *const i8 {
    match nanoem_application_plugin_model_io_t::get(plugin) {
        Some(instance) => instance.function_name(index),
        None => null(),
    }
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginModelIOSetFunction(
    plugin: *mut nanoem_application_plugin_model_io_t,
    index: i32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_model_io_t::get_mut(plugin) {
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
pub unsafe extern "C" fn nanoemApplicationPluginModelIOSetAllSelectedVertexObjectIndices(
    plugin: *mut nanoem_application_plugin_model_io_t,
    data: *const i32,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_model_io_t::get_mut(plugin) {
        Some(instance) => {
            let slice = if !data.is_null() && length > 0 {
                std::slice::from_raw_parts(data, length as usize)
            } else {
                &[]
            };
            match instance.set_all_selected_vertex_indices(slice) {
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
pub unsafe extern "C" fn nanoemApplicationPluginModelIOSetAllSelectedMaterialObjectIndices(
    plugin: *mut nanoem_application_plugin_model_io_t,
    data: *const i32,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_model_io_t::get_mut(plugin) {
        Some(instance) => {
            let slice = if !data.is_null() && length > 0 {
                std::slice::from_raw_parts(data, length as usize)
            } else {
                &[]
            };
            match instance.set_all_selected_material_indices(slice) {
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
pub unsafe extern "C" fn nanoemApplicationPluginModelIOSetAllSelectedBoneObjectIndices(
    plugin: *mut nanoem_application_plugin_model_io_t,
    data: *const i32,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_model_io_t::get_mut(plugin) {
        Some(instance) => {
            let slice = if !data.is_null() && length > 0 {
                std::slice::from_raw_parts(data, length as usize)
            } else {
                &[]
            };
            match instance.set_all_selected_bone_indices(slice) {
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
pub unsafe extern "C" fn nanoemApplicationPluginModelIOSetAllSelectedConstraintObjectIndices(
    plugin: *mut nanoem_application_plugin_model_io_t,
    data: *const i32,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_model_io_t::get_mut(plugin) {
        Some(instance) => {
            let slice = if !data.is_null() && length > 0 {
                std::slice::from_raw_parts(data, length as usize)
            } else {
                &[]
            };
            match instance.set_all_selected_bone_indices(slice) {
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
pub unsafe extern "C" fn nanoemApplicationPluginModelIOSetAllSelectedMorphObjectIndices(
    plugin: *mut nanoem_application_plugin_model_io_t,
    data: *const i32,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_model_io_t::get_mut(plugin) {
        Some(instance) => {
            let slice = if !data.is_null() && length > 0 {
                std::slice::from_raw_parts(data, length as usize)
            } else {
                &[]
            };
            match instance.set_all_selected_morph_indices(slice) {
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
pub unsafe extern "C" fn nanoemApplicationPluginModelIOSetAllSelectedLabelObjectIndices(
    plugin: *mut nanoem_application_plugin_model_io_t,
    data: *const i32,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_model_io_t::get_mut(plugin) {
        Some(instance) => {
            let slice = if !data.is_null() && length > 0 {
                std::slice::from_raw_parts(data, length as usize)
            } else {
                &[]
            };
            match instance.set_all_selected_label_indices(slice) {
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
pub unsafe extern "C" fn nanoemApplicationPluginModelIOSetAllSelectedRigidBodyObjectIndices(
    plugin: *mut nanoem_application_plugin_model_io_t,
    data: *const i32,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_model_io_t::get_mut(plugin) {
        Some(instance) => {
            let slice = if !data.is_null() && length > 0 {
                std::slice::from_raw_parts(data, length as usize)
            } else {
                &[]
            };
            match instance.set_all_selected_rigid_body_indices(slice) {
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
pub unsafe extern "C" fn nanoemApplicationPluginModelIOSetAllSelectedJointObjectIndices(
    plugin: *mut nanoem_application_plugin_model_io_t,
    data: *const i32,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_model_io_t::get_mut(plugin) {
        Some(instance) => {
            let slice = if !data.is_null() && length > 0 {
                std::slice::from_raw_parts(data, length as usize)
            } else {
                &[]
            };
            match instance.set_all_selected_joint_indices(slice) {
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
pub unsafe extern "C" fn nanoemApplicationPluginModelIOSetAllSelectedSoftBodyObjectIndices(
    plugin: *mut nanoem_application_plugin_model_io_t,
    data: *const i32,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_model_io_t::get_mut(plugin) {
        Some(instance) => {
            let slice = if !data.is_null() && length > 0 {
                std::slice::from_raw_parts(data, length as usize)
            } else {
                &[]
            };
            match instance.set_all_selected_soft_body_indices(slice) {
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
pub unsafe extern "C" fn nanoemApplicationPluginModelIOSetAudioDescription(
    plugin: *mut nanoem_application_plugin_model_io_t,
    data: *const u8,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_model_io_t::get_mut(plugin) {
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
pub unsafe extern "C" fn nanoemApplicationPluginModelIOSetCameraDescription(
    plugin: *mut nanoem_application_plugin_model_io_t,
    data: *const u8,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_model_io_t::get_mut(plugin) {
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
pub unsafe extern "C" fn nanoemApplicationPluginModelIOSetLightDescription(
    plugin: *mut nanoem_application_plugin_model_io_t,
    data: *const u8,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_model_io_t::get_mut(plugin) {
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
pub unsafe extern "C" fn nanoemApplicationPluginModelIOSetInputAudioData(
    plugin: *mut nanoem_application_plugin_model_io_t,
    data: *const u8,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_model_io_t::get_mut(plugin) {
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
pub unsafe extern "C" fn nanoemApplicationPluginModelIOSetInputModelData(
    plugin: *mut nanoem_application_plugin_model_io_t,
    data: *const u8,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_model_io_t::get_mut(plugin) {
        Some(instance) => {
            if !data.is_null() && length > 0 {
                let slice = std::slice::from_raw_parts(data, length as usize);
                match instance.set_input_data(slice) {
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
pub unsafe extern "C" fn nanoemApplicationPluginModelIOExecute(
    plugin: *mut nanoem_application_plugin_model_io_t,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_model_io_t::get_mut(plugin) {
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
pub unsafe extern "C" fn nanoemApplicationPluginModelIOGetOutputModelDataSize(
    plugin: *mut nanoem_application_plugin_model_io_t,
    length: *mut u32,
) {
    if let Some(instance) = nanoem_application_plugin_model_io_t::get(plugin) {
        if !length.is_null() {
            *length = instance.output_slice().len() as u32
        }
    }
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginModelIOGetOutputModelData(
    plugin: *mut nanoem_application_plugin_model_io_t,
    data: *mut u8,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_model_io_t::get(plugin) {
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
pub unsafe extern "C" fn nanoemApplicationPluginModelIOLoadUIWindowLayout(
    plugin: *mut nanoem_application_plugin_model_io_t,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_model_io_t::get_mut(plugin) {
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
pub unsafe extern "C" fn nanoemApplicationPluginModelIOGetUIWindowLayoutDataSize(
    plugin: *mut nanoem_application_plugin_model_io_t,
    length: *mut u32,
) {
    if let Some(instance) = nanoem_application_plugin_model_io_t::get(plugin) {
        if !length.is_null() {
            *length = instance.window_layout_data_slice().len() as u32
        }
    }
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginModelIOGetUIWindowLayoutData(
    plugin: *mut nanoem_application_plugin_model_io_t,
    data: *mut u8,
    length: u32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_model_io_t::get(plugin) {
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
pub unsafe extern "C" fn nanoemApplicationPluginModelIOSetUIComponentLayoutData(
    plugin: *mut nanoem_application_plugin_model_io_t,
    id: *const i8,
    data: *const u8,
    length: u32,
    reload_layout: *mut i32,
    status_ptr: *mut nanoem_application_plugin_status_t,
) {
    let status = match nanoem_application_plugin_model_io_t::get_mut(plugin) {
        Some(instance) => {
            let id = CStr::from_ptr(id as *const c_char);
            let data = std::slice::from_raw_parts(data, length as usize);
            let mut reload = false;
            match instance.set_component_layout(id, data, &mut reload) {
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
pub unsafe extern "C" fn nanoemApplicationPluginModelIOGetFailureReason(
    plugin: *const nanoem_application_plugin_model_io_t,
) -> *const i8 {
    match nanoem_application_plugin_model_io_t::get(plugin) {
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
pub unsafe extern "C" fn nanoemApplicationPluginModelIOGetRecoverySuggestion(
    plugin: *const nanoem_application_plugin_model_io_t,
) -> *const i8 {
    match nanoem_application_plugin_model_io_t::get(plugin) {
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
pub unsafe extern "C" fn nanoemApplicationPluginModelIODestroy(
    plugin: *mut nanoem_application_plugin_model_io_t,
) {
    if !plugin.is_null() {
        let _: Box<nanoem_application_plugin_model_io_t> = std::mem::transmute(plugin);
    }
}

/// # Safety
///
/// This function should be called from nanoem via plugin loader
#[no_mangle]
pub unsafe extern "C" fn nanoemApplicationPluginModelIOTerminate() {}
