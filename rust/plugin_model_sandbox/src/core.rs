/*
  Copyright (c) 2015-2020 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

extern crate nanoem;

use std::ffi::{CStr, CString};
use std::ptr::null;

pub enum ModelPluginError {
    InvalidArgument,
    FromError(String),
}
type Result<T> = std::result::Result<T, ModelPluginError>;

impl ModelPluginError {
    pub fn to_string(&self, _language: i32) -> String {
        match self {
            ModelPluginError::InvalidArgument => "invalid argument".to_string(),
            ModelPluginError::FromError(value) => value.to_string(),
        }
    }
}

impl<E: std::error::Error> From<E> for ModelPluginError {
    fn from(value: E) -> Self {
        ModelPluginError::FromError(value.to_string())
    }
}

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
            *value = self
        }
    }
}

#[allow(non_camel_case_types)]
pub struct nanoem_application_plugin_model_io_t {
    name: CString,
    description: CString,
    version: CString,
    failure_reason: Option<CString>,
    recovery_suggestion: Option<CString>,
    function_index: i32,
    language: i32,
    function_names: Vec<CString>,
    vertex_indices: Vec<i32>,
    material_indices: Vec<i32>,
    bone_indices: Vec<i32>,
    morph_indices: Vec<i32>,
    label_indices: Vec<i32>,
    rigid_body_indices: Vec<i32>,
    joint_indices: Vec<i32>,
    input_model: nanoem::Model,
    output_model_data: Option<Vec<u8>>,
    window_layout_data: Option<Vec<u8>>,
}

#[allow(non_camel_case_types)]
impl nanoem_application_plugin_model_io_t {
    pub fn new() -> std::result::Result<Self, ModelPluginError> {
        let name = CString::new("sandbox")?;
        let description = CString::new("sandbox plugin")?;
        let version = CString::new("1.0.0")?;
        let mut function_names: Vec<CString> = vec![];
        function_names.push(CString::new("test call 1")?);
        let factory = nanoem::UnicodeStringFactory::create()?;
        let input_model = nanoem::Model::new(factory)?;
        Ok(Self {
            name,
            description,
            version,
            failure_reason: None,
            recovery_suggestion: None,
            function_index: -1,
            language: 0,
            function_names,
            vertex_indices: vec![],
            material_indices: vec![],
            bone_indices: vec![],
            morph_indices: vec![],
            label_indices: vec![],
            rigid_body_indices: vec![],
            joint_indices: vec![],
            input_model,
            output_model_data: None,
            window_layout_data: None,
        })
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
    pub fn name(&self) -> *const i8 {
        self.name.as_ptr()
    }
    pub fn description(&self) -> *const i8 {
        self.description.as_ptr()
    }
    pub fn version(&self) -> *const i8 {
        self.version.as_ptr()
    }
    pub fn count_all_functions(&self) -> i32 {
        self.function_names.len() as i32
    }
    pub fn function_name(&self, value: i32) -> *const i8 {
        if value >= 0 && value < self.count_all_functions() {
            return self.function_names[value as usize].as_ptr();
        }
        null()
    }
    pub fn set_language(&mut self, value: i32) -> Result<()> {
        self.language = value;
        Ok(())
    }
    pub fn set_function(&mut self, value: i32) -> Result<()> {
        self.failure_reason = None;
        self.recovery_suggestion = None;
        self.window_layout_data = None;
        self.output_model_data = None;
        if value >= 0 && value < self.count_all_functions() {
            self.function_index = value
        }
        Ok(())
    }
    pub fn set_all_selected_vertex_indices(&mut self, data: &[i32]) -> Result<()> {
        self.vertex_indices.resize(data.len(), 0);
        self.vertex_indices.copy_from_slice(data);
        Ok(())
    }
    pub fn set_all_selected_material_indices(&mut self, data: &[i32]) -> Result<()> {
        self.material_indices.resize(data.len(), 0);
        self.material_indices.copy_from_slice(data);
        Ok(())
    }
    pub fn set_all_selected_bone_indices(&mut self, data: &[i32]) -> Result<()> {
        self.bone_indices.resize(data.len(), 0);
        self.bone_indices.copy_from_slice(data);
        Ok(())
    }
    pub fn set_all_selected_morph_indices(&mut self, data: &[i32]) -> Result<()> {
        self.morph_indices.resize(data.len(), 0);
        self.morph_indices.copy_from_slice(data);
        Ok(())
    }
    pub fn set_all_selected_label_indices(&mut self, data: &[i32]) -> Result<()> {
        self.label_indices.resize(data.len(), 0);
        self.label_indices.copy_from_slice(data);
        Ok(())
    }
    pub fn set_all_selected_rigid_body_indices(&mut self, data: &[i32]) -> Result<()> {
        self.rigid_body_indices.resize(data.len(), 0);
        self.rigid_body_indices.copy_from_slice(data);
        Ok(())
    }
    pub fn set_all_selected_joint_indices(&mut self, data: &[i32]) -> Result<()> {
        self.joint_indices.resize(data.len(), 0);
        self.joint_indices.copy_from_slice(data);
        Ok(())
    }
    pub fn set_audio_description(&mut self, data: &[u8]) -> Result<()> {
        let description = nanoem_protobuf::deserialize_audio_description(data)?;
        println!("{:?}", description);
        Ok(())
    }
    pub fn set_camera_description(&mut self, data: &[u8]) -> Result<()> {
        let description = nanoem_protobuf::deserialize_camera_description(data)?;
        println!("{:?}", description);
        Ok(())
    }
    pub fn set_light_description(&mut self, data: &[u8]) -> Result<()> {
        let description = nanoem_protobuf::deserialize_light_description(data)?;
        println!("{:?}", description);
        Ok(())
    }
    pub fn set_audio_data(&mut self, _data: &[u8]) -> Result<()> {
        Ok(())
    }
    pub fn set_input_data(&mut self, data: &[u8]) -> Result<()> {
        self.input_model.load(data)?;
        Ok(())
    }
    pub fn execute(&mut self) -> Result<()> {
        assert!(self.function_index >= 0);
        match self.function_index {
            0 => self.save_output_model_data(),
            _ => Err(ModelPluginError::InvalidArgument),
        }
    }
    pub fn output_slice(&self) -> &[u8] {
        assert!(self.output_model_data.is_some());
        match &self.output_model_data {
            Some(data) => data.as_slice(),
            None => &[],
        }
    }
    pub fn load_window_layout(&mut self) -> Result<()> {
        let mut components = vec![];
        components.push(nanoem_protobuf::create_label_component(format!(
            "format_type: {:?}",
            self.input_model.format_type()
        ), None));
        components.push(nanoem_protobuf::create_separator_component());
        components.push(nanoem_protobuf::create_label_component(format!(
            "codec_type: {:?}",
            self.input_model.codec_type()
        ), None));
        components.push(nanoem_protobuf::create_separator_component());
        components.push(nanoem_protobuf::create_label_component(format!(
            "name: {}",
            self.input_model.name(nanoem::Language::Japanese)?
        ), None));
        components.push(nanoem_protobuf::create_separator_component());
        components.push(nanoem_protobuf::create_label_component(format!(
            "comment: {}",
            self.input_model.comment(nanoem::Language::Japanese)?
        ), None));
        components.push(nanoem_protobuf::create_separator_component());
        self.layout_vertices(&mut components);
        components.push(nanoem_protobuf::create_separator_component());
        self.layout_materials(&mut components)?;
        components.push(nanoem_protobuf::create_separator_component());
        self.layout_bones(&mut components)?;
        components.push(nanoem_protobuf::create_separator_component());
        self.layout_morphs(&mut components)?;
        components.push(nanoem_protobuf::create_separator_component());
        self.layout_labels(&mut components)?;
        components.push(nanoem_protobuf::create_separator_component());
        self.layout_rigid_bodys(&mut components)?;
        components.push(nanoem_protobuf::create_separator_component());
        self.layout_joints(&mut components)?;
        let window = nanoem_protobuf::create_window(
            "plugin_model_sandbox".to_string(),
            "plugin_model_sandbox".to_string(),
            components,
        );
        let mut data = vec![];
        nanoem_protobuf::serialize_window(window, &mut data)?;
        self.window_layout_data = Some(data);
        Ok(())
    }
    pub fn set_component_layout(&mut self, id: &CStr, data: &[u8]) -> Result<()> {
        let id = String::from_utf8_lossy(id.to_bytes_with_nul());
        let component = nanoem_protobuf::deserialize_component(data)?;
        self.update_layout(&id, component)
    }
    pub fn window_layout_data_slice(&self) -> &[u8] {
        match &self.window_layout_data {
            Some(data) => data.as_slice(),
            None => &[],
        }
    }
    pub fn failure_reason(&self) -> &Option<CString> {
        &self.failure_reason
    }
    pub fn recovery_suggestion(&self) -> &Option<CString> {
        &self.recovery_suggestion
    }
    pub fn assign_failure_reason(
        &mut self,
        value: ModelPluginError,
    ) -> nanoem_application_plugin_status_t {
        match CString::new(value.to_string(self.language)) {
            Ok(reason) => {
                self.failure_reason = Some(reason);
                nanoem_application_plugin_status_t::ERROR_REFER_REASON
            }
            Err(_) => nanoem_application_plugin_status_t::ERROR_NULL_OBJECT,
        }
    }
    fn save_output_model_data(&mut self) -> Result<()> {
        let mutable_model = nanoem::MutableModel::from_model(&self.input_model)?;
        let mut data: Vec<u8> = vec![];
        mutable_model.save(&mut data)?;
        self.output_model_data = Some(data);
        Ok(())
    }
    fn layout_vertices(&self, components: &mut Vec<nanoem_protobuf::UiComponent>) {
        let vertices = self.input_model.all_vertices();
        components.push(nanoem_protobuf::create_label_component(format!(
            "num_vertices: {}",
            vertices.len()
        ), None));
        let mut selectables = vec![];
        for item in vertices {
            let id = format!("vertex:{}", item.index());
            let text = format!("vertex: index={}", item.index());
            selectables.push(nanoem_protobuf::create_selectable(id, text));
        }
        components.push(nanoem_protobuf::create_combobox_component(
            "vertices".to_string(),
            "".to_string(),
            selectables,
        ));
    }
    fn layout_materials(&self, components: &mut Vec<nanoem_protobuf::UiComponent>) -> Result<()> {
        let materials = self.input_model.all_materials();
        components.push(nanoem_protobuf::create_label_component(format!(
            "num_materials: {}",
            materials.len()
        ), None));
        let mut selectables = vec![];
        for item in materials {
            let id = format!("material:{}", item.index());
            let text = format!(
                "material: name={}, index={}",
                item.name(nanoem::Language::Japanese)?,
                item.index()
            );
            selectables.push(nanoem_protobuf::create_selectable(id, text));
        }
        components.push(nanoem_protobuf::create_combobox_component(
            "materials".to_string(),
            "".to_string(),
            selectables,
        ));
        Ok(())
    }
    fn layout_bones(&self, components: &mut Vec<nanoem_protobuf::UiComponent>) -> Result<()> {
        let bones = self.input_model.all_bones();
        components.push(nanoem_protobuf::create_label_component(format!(
            "num_bones: {}",
            bones.len()
        ), None));
        let mut selectables = vec![];
        for item in bones {
            let id = format!("bone:{}", item.index());
            let text = format!(
                "bone: name={}, index={}",
                item.name(nanoem::Language::Japanese)?,
                item.index()
            );
            selectables.push(nanoem_protobuf::create_selectable(id, text));
        }
        components.push(nanoem_protobuf::create_combobox_component(
            "bones".to_string(),
            "".to_string(),
            selectables,
        ));
        Ok(())
    }
    fn layout_morphs(&self, components: &mut Vec<nanoem_protobuf::UiComponent>) -> Result<()> {
        let morphs = self.input_model.all_morphs();
        components.push(nanoem_protobuf::create_label_component(format!(
            "num_morphs: {}",
            morphs.len()
        ), None));
        let mut selectables = vec![];
        for item in morphs {
            let id = format!("morph:{}", item.index());
            let text = format!(
                "morph: name={}, index={} category={:?} type={:?}",
                item.name(nanoem::Language::Japanese)?,
                item.index(),
                item.category(),
                item.morph_type()
            );
            selectables.push(nanoem_protobuf::create_selectable(id, text));
        }
        components.push(nanoem_protobuf::create_combobox_component(
            "morphs".to_string(),
            "".to_string(),
            selectables,
        ));
        Ok(())
    }
    fn layout_labels(&self, components: &mut Vec<nanoem_protobuf::UiComponent>) -> Result<()> {
        let labels = self.input_model.all_labels();
        components.push(nanoem_protobuf::create_label_component(format!(
            "num_labels: {}",
            labels.len()
        ), None));
        let mut selectables = vec![];
        for item in labels {
            let id = format!("label:{}", item.index());
            let text = format!(
                "label: name={}, index={} num_items={}",
                item.name(nanoem::Language::Japanese)?,
                item.index(),
                item.all_items().len()
            );
            selectables.push(nanoem_protobuf::create_selectable(id, text));
        }
        components.push(nanoem_protobuf::create_combobox_component(
            "labels".to_string(),
            "".to_string(),
            selectables,
        ));
        Ok(())
    }
    fn layout_rigid_bodys(&self, components: &mut Vec<nanoem_protobuf::UiComponent>) -> Result<()> {
        let rigid_bodies = self.input_model.all_rigid_bodies();
        components.push(nanoem_protobuf::create_label_component(format!(
            "num_rigid_bodies: {}",
            rigid_bodies.len()
        ), None));
        let mut selectables = vec![];
        for item in rigid_bodies {
            let id = format!("rigid_body:{}", item.index());
            let text = format!(
                "rigid_body: name={}, index={} shape_type={:?}, transform_type={:?}",
                item.name(nanoem::Language::Japanese)?,
                item.index(),
                item.shape_type(),
                item.transform_type()
            );
            selectables.push(nanoem_protobuf::create_selectable(id, text));
        }
        components.push(nanoem_protobuf::create_combobox_component(
            "rigid_bodies".to_string(),
            "".to_string(),
            selectables,
        ));
        Ok(())
    }
    fn layout_joints(&self, components: &mut Vec<nanoem_protobuf::UiComponent>) -> Result<()> {
        let joints = self.input_model.all_joints();
        components.push(nanoem_protobuf::create_label_component(format!(
            "num_joints: {}",
            joints.len()
        ), None));
        let mut selectables = vec![];
        for item in joints {
            let id = format!("joint:{}", item.index());
            let text = format!(
                "joint: name={}, index={}",
                item.name(nanoem::Language::Japanese)?,
                item.index()
            );
            selectables.push(nanoem_protobuf::create_selectable(id, text));
        }
        components.push(nanoem_protobuf::create_combobox_component(
            "joints".to_string(),
            "".to_string(),
            selectables,
        ));
        Ok(())
    }
    fn update_layout(&mut self, id: &str, component: nanoem_protobuf::UiComponent) -> Result<()> {
        println!("id = {}, component = {:?}", id, component);
        Ok(())
    }
}
