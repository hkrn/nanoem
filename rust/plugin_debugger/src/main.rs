extern crate libloading;

use libloading::{Library, Symbol};
use std::ffi::{c_void, CStr, CString};
use std::os::raw::c_char;

unsafe fn run_as_model_plugin(
    library: &Library,
    path: &std::path::Path,
) -> Result<(), Box<dyn std::error::Error>> {
    let initialize: Symbol<unsafe extern "C" fn()> =
        library.get(b"nanoemApplicationPluginModelIOInitialize")?;
    let create: Symbol<unsafe extern "C" fn(*const c_char) -> *mut c_void> =
        library.get(b"nanoemApplicationPluginModelIOCreateWithLocation")?;
    let count_all_functions: Symbol<unsafe extern "C" fn(*const c_void) -> u32> =
        library.get(b"nanoemApplicationPluginModelIOCountAllFunctions")?;
    let function_name: Symbol<unsafe extern "C" fn(*const c_void, u32) -> *const c_char> =
        library.get(b"nanoemApplicationPluginModelIOGetFunctionName")?;
    let set_function: Symbol<unsafe extern "C" fn(*const c_void, i32, *mut u32)> =
        library.get(b"nanoemApplicationPluginModelIOSetFunction")?;
    let set_input_model_data: Symbol<
        unsafe extern "C" fn(*const c_void, *const c_void, usize, *mut u32),
    > = library.get(b"nanoemApplicationPluginModelIOSetInputModelData")?;
    let set_all_selected_vertex_object_indices: Symbol<
        unsafe extern "C" fn(*const c_void, *const c_void, usize, *mut u32),
    > = library.get(b"nanoemApplicationPluginModelIOSetAllSelectedVertexObjectIndices")?;
    let set_all_selected_material_object_indices: Symbol<
        unsafe extern "C" fn(*const c_void, *const c_void, usize, *mut u32),
    > = library.get(b"nanoemApplicationPluginModelIOSetAllSelectedMaterialObjectIndices")?;
    let set_all_selected_bone_object_indices: Symbol<
        unsafe extern "C" fn(*const c_void, *const c_void, usize, *mut u32),
    > = library.get(b"nanoemApplicationPluginModelIOSetAllSelectedBoneObjectIndices")?;
    let set_all_selected_constraint_object_indices: Symbol<
        unsafe extern "C" fn(*const c_void, *const c_void, usize, *mut u32),
    > = library.get(b"nanoemApplicationPluginModelIOSetAllSelectedConstraintObjectIndices")?;
    let set_all_selected_morph_object_indices: Symbol<
        unsafe extern "C" fn(*const c_void, *const c_void, usize, *mut u32),
    > = library.get(b"nanoemApplicationPluginModelIOSetAllSelectedMorphObjectIndices")?;
    let set_all_selected_label_object_indices: Symbol<
        unsafe extern "C" fn(*const c_void, *const c_void, usize, *mut u32),
    > = library.get(b"nanoemApplicationPluginModelIOSetAllSelectedLabelObjectIndices")?;
    let set_all_selected_rigid_body_object_indices: Symbol<
        unsafe extern "C" fn(*const c_void, *const c_void, usize, *mut u32),
    > = library.get(b"nanoemApplicationPluginModelIOSetAllSelectedRigidBodyObjectIndices")?;
    let set_all_selected_joint_object_indices: Symbol<
        unsafe extern "C" fn(*const c_void, *const c_void, usize, *mut u32),
    > = library.get(b"nanoemApplicationPluginModelIOSetAllSelectedJointObjectIndices")?;
    let execute: Symbol<unsafe extern "C" fn(*const c_void, *mut u32)> =
        library.get(b"nanoemApplicationPluginModelIOExecute")?;
    let get_output_model_data_size: Symbol<unsafe extern "C" fn(*const c_void, *mut usize)> =
        library.get(b"nanoemApplicationPluginModelIOGetOutputModelDataSize")?;
    let get_output_model_data: Symbol<
        unsafe extern "C" fn(*const c_void, *mut c_void, usize, *mut u32),
    > = library.get(b"nanoemApplicationPluginModelIOGetOutputModelData")?;
    let destroy: Symbol<unsafe extern "C" fn(*mut c_void)> =
        library.get(b"nanoemApplicationPluginModelIODestroy")?;
    let terminate: Symbol<unsafe extern "C" fn()> =
        library.get(b"nanoemApplicationPluginModelIOTerminate")?;
    initialize();
    let path = CString::new(path.to_str().unwrap())?;
    let instance = create(path.as_ptr());
    for i in 0..count_all_functions(instance) {
        let name = function_name(instance, i);
        if !name.is_null() {
            let name = CStr::from_ptr(name);
            println!("[{}] = {}", i, name.to_owned().to_str()?);
            set_function(instance, i as i32, std::ptr::null_mut());
            let mut input_data = vec![];
            input_data.resize(4, 0u8);
            set_input_model_data(
                instance,
                input_data.as_ptr() as *const _,
                input_data.len(),
                std::ptr::null_mut(),
            );
            let indices = vec![0xff00i32];
            set_all_selected_bone_object_indices(
                instance,
                indices.as_ptr() as *const _,
                indices.len(),
                std::ptr::null_mut(),
            );
            let indices = vec![0xff01i32];
            set_all_selected_constraint_object_indices(
                instance,
                indices.as_ptr() as *const _,
                indices.len(),
                std::ptr::null_mut(),
            );
            let indices = vec![0xff02i32];
            set_all_selected_joint_object_indices(
                instance,
                indices.as_ptr() as *const _,
                indices.len(),
                std::ptr::null_mut(),
            );
            let indices = vec![0xff03i32];
            set_all_selected_label_object_indices(
                instance,
                indices.as_ptr() as *const _,
                indices.len(),
                std::ptr::null_mut(),
            );
            let indices = vec![0xff04i32];
            set_all_selected_material_object_indices(
                instance,
                indices.as_ptr() as *const _,
                indices.len(),
                std::ptr::null_mut(),
            );
            let indices = vec![0xff05i32];
            set_all_selected_morph_object_indices(
                instance,
                indices.as_ptr() as *const _,
                indices.len(),
                std::ptr::null_mut(),
            );
            let indices = vec![0xff06i32];
            set_all_selected_rigid_body_object_indices(
                instance,
                indices.as_ptr() as *const _,
                indices.len(),
                std::ptr::null_mut(),
            );
            let indices = vec![0xff07i32];
            set_all_selected_vertex_object_indices(
                instance,
                indices.as_ptr() as *const _,
                indices.len(),
                std::ptr::null_mut(),
            );
            execute(instance, std::ptr::null_mut());
            let mut output_size = 0usize;
            get_output_model_data_size(instance, &mut output_size as *mut usize);
            let mut output_data = vec![];
            output_data.resize(output_size, 0u8);
            get_output_model_data(
                instance,
                output_data.as_mut_ptr() as *mut _,
                output_size,
                std::ptr::null_mut(),
            );
        }
    }
    destroy(instance);
    terminate();
    Ok(())
}

unsafe fn run_as_motion_plugin(
    library: &Library,
    path: &std::path::Path,
) -> Result<(), Box<dyn std::error::Error>> {
    let initialize: Symbol<unsafe extern "C" fn()> =
        library.get(b"nanoemApplicationPluginMotionIOInitialize")?;
    let create: Symbol<unsafe extern "C" fn(*const c_char) -> *mut c_void> =
        library.get(b"nanoemApplicationPluginMotionIOCreateWithLocation")?;
    let count_all_functions: Symbol<unsafe extern "C" fn(*const c_void) -> u32> =
        library.get(b"nanoemApplicationPluginMotionIOCountAllFunctions")?;
    let function_name: Symbol<unsafe extern "C" fn(*const c_void, u32) -> *const c_char> =
        library.get(b"nanoemApplicationPluginMotionIOGetFunctionName")?;
    let set_function: Symbol<unsafe extern "C" fn(*const c_void, i32, *mut u32)> =
        library.get(b"nanoemApplicationPluginMotionIOSetFunction")?;
    let set_accessory_keyframes: Symbol<
        unsafe extern "C" fn(*const c_void, *const c_void, usize, *mut u32),
    > = library.get(b"nanoemApplicationPluginMotionIOSetAllSelectedAccessoryKeyframes")?;
    let set_bone_keyframes: Symbol<
        unsafe extern "C" fn(*const c_void, *const c_char, *const c_void, usize, *mut u32),
    > = library.get(b"nanoemApplicationPluginMotionIOSetAllNamedSelectedBoneKeyframes")?;
    let set_camera_keyframes: Symbol<
        unsafe extern "C" fn(*const c_void, *const c_void, usize, *mut u32),
    > = library.get(b"nanoemApplicationPluginMotionIOSetAllSelectedCameraKeyframes")?;
    let set_light_keyframes: Symbol<
        unsafe extern "C" fn(*const c_void, *const c_void, usize, *mut u32),
    > = library.get(b"nanoemApplicationPluginMotionIOSetAllSelectedLightKeyframes")?;
    let set_model_keyframes: Symbol<
        unsafe extern "C" fn(*const c_void, *const c_void, usize, *mut u32),
    > = library.get(b"nanoemApplicationPluginMotionIOSetAllSelectedModelKeyframes")?;
    let set_morph_keyframes: Symbol<
        unsafe extern "C" fn(*const c_void, *const c_char, *const c_void, usize, *mut u32),
    > = library.get(b"nanoemApplicationPluginMotionIOSetAllNamedSelectedMorphKeyframes")?;
    let set_self_shadow_keyframes: Symbol<
        unsafe extern "C" fn(*const c_void, *const c_void, usize, *mut u32),
    > = library.get(b"nanoemApplicationPluginMotionIOSetAllSelectedSelfShadowKeyframes")?;
    let set_input_active_model_data: Symbol<
        unsafe extern "C" fn(*const c_void, *const c_void, usize, *mut u32),
    > = library.get(b"nanoemApplicationPluginMotionIOSetInputActiveModelData")?;
    let set_input_motion_data: Symbol<
        unsafe extern "C" fn(*const c_void, *const c_void, usize, *mut u32),
    > = library.get(b"nanoemApplicationPluginMotionIOSetInputMotionData")?;
    let execute: Symbol<unsafe extern "C" fn(*const c_void, *mut u32)> =
        library.get(b"nanoemApplicationPluginMotionIOExecute")?;
    let get_output_motion_data_size: Symbol<unsafe extern "C" fn(*const c_void, *mut usize)> =
        library.get(b"nanoemApplicationPluginMotionIOGetOutputMotionDataSize")?;
    let get_output_motion_data: Symbol<
        unsafe extern "C" fn(*const c_void, *mut c_void, usize, *mut u32),
    > = library.get(b"nanoemApplicationPluginMotionIOGetOutputMotionData")?;
    let destroy: Symbol<unsafe extern "C" fn(*mut c_void)> =
        library.get(b"nanoemApplicationPluginMotionIODestroy")?;
    let terminate: Symbol<unsafe extern "C" fn()> =
        library.get(b"nanoemApplicationPluginMotionIOTerminate")?;
    initialize();
    let path = CString::new(path.to_str().unwrap())?;
    let instance = create(path.as_ptr());
    for i in 0..count_all_functions(instance) {
        let name = function_name(instance, i);
        if !name.is_null() {
            let name = CStr::from_ptr(name);
            println!("[{}] = {}", i, name.to_owned().to_str()?);
            set_function(instance, i as i32, std::ptr::null_mut());
            let mut input_data = vec![];
            input_data.resize(4, 0u8);
            let indices = vec![0xfe00i32];
            set_accessory_keyframes(
                instance,
                indices.as_ptr() as *const _,
                indices.len(),
                std::ptr::null_mut(),
            );
            let indices = vec![0xfe01i32];
            for i in 0..3 {
                let name = CString::new(format!("bones/ボーン/{}", i)).unwrap();
                set_bone_keyframes(
                    instance,
                    name.as_ptr(),
                    indices.as_ptr() as *const _,
                    indices.len(),
                    std::ptr::null_mut(),
                );
            }
            let indices = vec![0xfe02i32];
            set_camera_keyframes(
                instance,
                indices.as_ptr() as *const _,
                indices.len(),
                std::ptr::null_mut(),
            );
            let indices = vec![0xfe03i32];
            set_light_keyframes(
                instance,
                indices.as_ptr() as *const _,
                indices.len(),
                std::ptr::null_mut(),
            );
            let indices = vec![0xfe04i32];
            set_model_keyframes(
                instance,
                indices.as_ptr() as *const _,
                indices.len(),
                std::ptr::null_mut(),
            );
            let indices = vec![0xfe05i32];
            for i in 0..3 {
                let name = CString::new(format!("morphs/モーフ/{}", i)).unwrap();
                set_morph_keyframes(
                    instance,
                    name.as_ptr(),
                    indices.as_ptr() as *const _,
                    indices.len(),
                    std::ptr::null_mut(),
                );
            }
            let indices = vec![0xfe06i32];
            set_self_shadow_keyframes(
                instance,
                indices.as_ptr() as *const _,
                indices.len(),
                std::ptr::null_mut(),
            );
            set_input_active_model_data(
                instance,
                input_data.as_ptr() as *const _,
                input_data.len(),
                std::ptr::null_mut(),
            );
            set_input_motion_data(
                instance,
                input_data.as_ptr() as *const _,
                input_data.len(),
                std::ptr::null_mut(),
            );
            execute(instance, std::ptr::null_mut());
            let mut output_size = 0usize;
            get_output_motion_data_size(instance, &mut output_size as *mut usize);
            let mut output_data = vec![];
            output_data.resize(output_size, 0u8);
            get_output_motion_data(
                instance,
                output_data.as_mut_ptr() as *mut _,
                output_size,
                std::ptr::null_mut(),
            );
        }
    }
    destroy(instance);
    terminate();
    Ok(())
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let mut path = std::env::current_dir()?;
    path.push("target/debug/libplugin_wasm.dylib");
    let library = Library::new(&path)?;
    unsafe {
        run_as_model_plugin(&library, &path)?;
        run_as_motion_plugin(&library, &path)?;
    }
    Ok(())
}
