/*
  Copyright (c) 2015-2023 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

use anyhow::Result;

use crate::nanoem_application_plugin_status_t;

use super::plugin::ModelIOPluginController;

use std::ffi::CStr;
use std::path::Path;
use std::ptr::null;

#[allow(non_camel_case_types)]
pub struct nanoem_application_plugin_model_io_t {
    controller: ModelIOPluginController,
}

impl Drop for nanoem_application_plugin_model_io_t {
    fn drop(&mut self) {
        self.controller.destroy();
        self.controller.terminate();
    }
}

#[allow(non_camel_case_types)]
impl nanoem_application_plugin_model_io_t {
    pub fn new(path: &CStr) -> Result<Self> {
        let path = Path::new(path.to_str()?);
        let mut controller = ModelIOPluginController::from_path(path)?;
        controller.initialize()?;
        Ok(Self { controller })
    }
    pub(crate) unsafe fn get(plugin: *const Self) -> Option<&'static Self> {
        if !plugin.is_null() {
            Some(&(*plugin))
        } else {
            None
        }
    }
    pub(crate) unsafe fn get_mut(plugin: *mut Self) -> Option<&'static mut Self> {
        if !plugin.is_null() {
            Some(&mut (*plugin))
        } else {
            None
        }
    }
    pub fn create(&mut self) -> Result<()> {
        self.controller.create()
    }
    pub fn name(&self) -> *const i8 {
        crate::PLUGIN_NAME.as_ptr() as *const i8
    }
    pub fn description(&self) -> *const i8 {
        crate::PLUGIN_DESCRIPTION.as_ptr() as *const i8
    }
    pub fn version(&self) -> *const i8 {
        crate::PLUGIN_VERSION.as_ptr() as *const i8
    }
    pub fn count_all_functions(&self) -> i32 {
        self.controller.count_all_functions()
    }
    pub fn function_name(&self, value: i32) -> *const i8 {
        if let Ok(name) = self.controller.function_name(value) {
            name.as_ptr() as *const i8
        } else {
            null()
        }
    }
    pub fn set_language(&mut self, value: i32) -> Result<()> {
        self.controller.set_language(value)
    }
    pub fn set_function(&mut self, value: i32) -> Result<i32> {
        self.controller.set_function(value)
    }
    pub fn set_all_selected_vertex_indices(&mut self, data: &[i32]) -> Result<()> {
        self.controller.set_all_selected_vertex_indices(data)
    }
    pub fn set_all_selected_material_indices(&mut self, data: &[i32]) -> Result<()> {
        self.controller.set_all_selected_material_indices(data)
    }
    pub fn set_all_selected_bone_indices(&mut self, data: &[i32]) -> Result<()> {
        self.controller.set_all_selected_bone_indices(data)
    }
    pub fn set_all_selected_morph_indices(&mut self, data: &[i32]) -> Result<()> {
        self.controller.set_all_selected_morph_indices(data)
    }
    pub fn set_all_selected_label_indices(&mut self, data: &[i32]) -> Result<()> {
        self.controller.set_all_selected_label_indices(data)
    }
    pub fn set_all_selected_rigid_body_indices(&mut self, data: &[i32]) -> Result<()> {
        self.controller.set_all_selected_rigid_body_indices(data)
    }
    pub fn set_all_selected_joint_indices(&mut self, data: &[i32]) -> Result<()> {
        self.controller.set_all_selected_joint_indices(data)
    }
    pub fn set_all_selected_soft_body_indices(&mut self, data: &[i32]) -> Result<()> {
        self.controller.set_all_selected_soft_body_indices(data)
    }
    pub fn set_audio_description(&mut self, data: &[u8]) -> Result<()> {
        self.controller.set_audio_description(data)
    }
    pub fn set_camera_description(&mut self, data: &[u8]) -> Result<()> {
        self.controller.set_camera_description(data)
    }
    pub fn set_light_description(&mut self, data: &[u8]) -> Result<()> {
        self.controller.set_light_description(data)
    }
    pub fn set_audio_data(&mut self, data: &[u8]) -> Result<()> {
        self.controller.set_audio_data(data)
    }
    pub fn set_input_data(&mut self, data: &[u8]) -> Result<()> {
        self.controller.set_input_model_data(data)
    }
    pub fn execute(&mut self) -> Result<()> {
        self.controller.execute()
    }
    pub fn output_slice(&mut self) -> Vec<u8> {
        self.controller.get_output_data().unwrap_or_default()
    }
    pub fn load_window_layout(&mut self) -> Result<()> {
        self.controller.load_ui_window_layout()
    }
    pub fn set_component_layout(
        &mut self,
        id: &CStr,
        data: &[u8],
        reload: &mut bool,
    ) -> Result<()> {
        self.controller
            .set_ui_component_layout(id.to_str()?, data, reload)
    }
    pub fn window_layout_data_slice(&mut self) -> Vec<u8> {
        self.controller.get_ui_window_layout().unwrap_or_default()
    }
    pub fn failure_reason(&self) -> Option<String> {
        self.controller.failure_reason()
    }
    pub fn recovery_suggestion(&self) -> Option<String> {
        self.controller.recovery_suggestion()
    }
    pub fn assign_failure_reason(
        &mut self,
        value: anyhow::Error,
    ) -> nanoem_application_plugin_status_t {
        self.controller.assign_failure_reason(value);
        nanoem_application_plugin_status_t::ERROR_REFER_REASON
    }
}
