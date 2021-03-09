/*
  Copyright (c) 2015-2020 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

extern crate nanoem;
extern crate nanoem_protobuf;
extern crate rand;

use rand::rngs::StdRng;
use rand::{FromEntropy, Rng};
use std::collections::HashMap;
use std::ffi::{CStr, CString};
use std::ptr::null;

pub enum MotionPluginError {
    InvalidArgument,
    FromError(String),
}
type Result<T> = std::result::Result<T, MotionPluginError>;

impl MotionPluginError {
    pub fn to_string(&self, _language: i32) -> String {
        match self {
            MotionPluginError::InvalidArgument => "invalid argument".to_string(),
            MotionPluginError::FromError(value) => value.to_string(),
        }
    }
}

impl<E: std::error::Error> From<E> for MotionPluginError {
    fn from(value: E) -> Self {
        MotionPluginError::FromError(value.to_string())
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
pub struct nanoem_application_plugin_motion_io_t {
    name: CString,
    description: CString,
    version: CString,
    failure_reason: Option<CString>,
    recovery_suggestion: Option<CString>,
    function_index: i32,
    language: i32,
    function_names: Vec<CString>,
    input_model: Option<nanoem::Model>,
    input_motion: nanoem::Motion,
    accessory_keyframes: Vec<u32>,
    bone_keyframes: HashMap<String, Vec<u32>>,
    camera_keyframes: Vec<u32>,
    light_keyframes: Vec<u32>,
    model_keyframes: Vec<u32>,
    morph_keyframes: HashMap<String, Vec<u32>>,
    self_shadow_keyframes: Vec<u32>,
    audio_description: Option<nanoem_protobuf::application::plugin::AudioDescription>,
    camera_description: Option<nanoem_protobuf::application::plugin::CameraDescription>,
    light_description: Option<nanoem_protobuf::application::plugin::LightDescription>,
    output_motion_data: Option<Vec<u8>>,
    window_layout_data: Option<Vec<u8>>,
}

impl nanoem_application_plugin_motion_io_t {
    pub fn new() -> std::result::Result<Self, MotionPluginError> {
        let name = CString::new("sandbox")?;
        let description = CString::new("sandbox plugin")?;
        let version = CString::new("1.0.0")?;
        let mut function_names: Vec<CString> = vec![];
        function_names.push(CString::new("Test Call")?);
        function_names.push(CString::new("Random Blink")?);
        let factory = nanoem::UnicodeStringFactory::create()?;
        let input_motion = nanoem::Motion::new(factory)?;
        Ok(Self {
            name,
            description,
            version,
            failure_reason: None,
            recovery_suggestion: None,
            function_index: -1,
            language: 0,
            function_names,
            input_model: None,
            input_motion,
            accessory_keyframes: vec![],
            bone_keyframes: HashMap::new(),
            camera_keyframes: vec![],
            light_keyframes: vec![],
            model_keyframes: vec![],
            morph_keyframes: HashMap::new(),
            self_shadow_keyframes: vec![],
            audio_description: None,
            camera_description: None,
            light_description: None,
            output_motion_data: None,
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
        self.output_motion_data = None;
        if value >= 0 && value < self.count_all_functions() {
            self.function_index = value
        }
        Ok(())
    }
    pub fn set_all_selected_accessory_keyframes(&mut self, value: &[u32]) -> Result<()> {
        self.accessory_keyframes.resize(value.len(), 0);
        self.accessory_keyframes.copy_from_slice(value);
        Ok(())
    }
    pub fn set_all_named_selected_bone_keyframes(
        &mut self,
        name: &CStr,
        value: &[u32],
    ) -> Result<()> {
        let mut keyframes = vec![];
        keyframes.resize(value.len(), 0);
        keyframes.copy_from_slice(value);
        self.bone_keyframes
            .insert(String::from(name.to_str()?), keyframes);
        Ok(())
    }
    pub fn set_all_selected_camera_keyframes(&mut self, value: &[u32]) -> Result<()> {
        self.camera_keyframes.resize(value.len(), 0);
        self.camera_keyframes.copy_from_slice(value);
        Ok(())
    }
    pub fn set_all_selected_light_keyframes(&mut self, value: &[u32]) -> Result<()> {
        self.light_keyframes.resize(value.len(), 0);
        self.light_keyframes.copy_from_slice(value);
        Ok(())
    }
    pub fn set_all_selected_model_keyframes(&mut self, value: &[u32]) -> Result<()> {
        self.model_keyframes.resize(value.len(), 0);
        self.model_keyframes.copy_from_slice(value);
        Ok(())
    }
    pub fn set_all_named_selected_morph_keyframes(
        &mut self,
        name: &CStr,
        value: &[u32],
    ) -> Result<()> {
        let mut keyframes = vec![];
        keyframes.resize(value.len(), 0);
        keyframes.copy_from_slice(value);
        self.morph_keyframes
            .insert(String::from(name.to_str()?), keyframes);
        Ok(())
    }
    pub fn set_all_selected_self_shadow_keyframes(&mut self, value: &[u32]) -> Result<()> {
        self.self_shadow_keyframes.resize(value.len(), 0);
        self.self_shadow_keyframes.copy_from_slice(value);
        Ok(())
    }
    pub fn set_audio_description(&mut self, data: &[u8]) -> Result<()> {
        let description = nanoem_protobuf::deserialize_audio_description(data)?;
        self.audio_description = Some(description);
        Ok(())
    }
    pub fn set_camera_description(&mut self, data: &[u8]) -> Result<()> {
        let description = nanoem_protobuf::deserialize_camera_description(data)?;
        self.camera_description = Some(description);
        Ok(())
    }
    pub fn set_light_description(&mut self, data: &[u8]) -> Result<()> {
        let description = nanoem_protobuf::deserialize_light_description(data)?;
        self.light_description = Some(description);
        Ok(())
    }
    pub fn set_audio_data(&mut self, _data: &[u8]) -> Result<()> {
        Ok(())
    }
    pub fn set_input_model_data(&mut self, data: &[u8]) -> Result<()> {
        let factory = nanoem::UnicodeStringFactory::create()?;
        let model = nanoem::Model::with_bytes(factory, data)?;
        self.input_model = Some(model);
        Ok(())
    }
    pub fn set_input_motion_data(&mut self, data: &[u8]) -> Result<()> {
        self.input_motion
            .load(data, 0, nanoem::MotionFormatType::NMD)?;
        match self.function_index {
            0 => (),
            1 => (),
            _ => (),
        }
        Ok(())
    }

    fn register_morph_keyframe(
        mutable_motion: &nanoem::MutableMotion,
        name: &str,
        frame_index: u32,
        weight: f32,
    ) -> Result<()> {
        let keyframe = mutable_motion.create_morph_keyframe()?;
        keyframe.set_weight(weight);
        mutable_motion.set_morph_keyframe(keyframe, name, frame_index)?;
        Ok(())
    }
    fn execute_random_blink(&mut self) -> Result<()> {
        let name = "まばたき";
        let preferred_fps = self.input_motion.preferred_fps().max(30.0);
        let duration = match &self.audio_description {
            Some(value) => {
                let seconds = nanoem_protobuf::divide_rational(&value.duration);
                (f64::from(preferred_fps) * seconds) as u32
            }
            None => (preferred_fps * 10.0) as u32,
        };
        let blink_seconds = 0.1;
        let min_interval = 0.1;
        let max_interval = 3.0;
        let value = 0.8;
        let mut mutable_motion = nanoem::MutableMotion::from_motion(&self.input_motion)?;
        let mut rng = StdRng::from_entropy();
        let mut current_frame_index: u32 =
            (rng.gen_range(min_interval, max_interval) * preferred_fps) as u32;
        while current_frame_index < duration {
            Self::register_morph_keyframe(&mutable_motion, name, current_frame_index, 0.0)?;
            current_frame_index += (blink_seconds * preferred_fps).max(1.0) as u32;
            Self::register_morph_keyframe(&mutable_motion, name, current_frame_index, value)?;
            current_frame_index += (blink_seconds * preferred_fps).max(1.0) as u32;
            Self::register_morph_keyframe(&mutable_motion, name, current_frame_index, value)?;
            current_frame_index += (blink_seconds * preferred_fps).max(1.0) as u32;
            Self::register_morph_keyframe(&mutable_motion, name, current_frame_index, 0.0)?;
            current_frame_index +=
                (rng.gen_range(min_interval, max_interval) * preferred_fps).max(1.0) as u32;
        }
        self.save_output_motion_data(&mut mutable_motion)
    }

    pub fn execute(&mut self) -> Result<()> {
        assert!(self.function_index >= 0);
        match self.function_index {
            0 => {
                let mut mutable_motion = nanoem::MutableMotion::from_motion(&self.input_motion)?;
                self.save_output_motion_data(&mut mutable_motion)
            }
            1 => match &self.input_model {
                Some(_) => self.execute_random_blink(),
                None => Err(MotionPluginError::InvalidArgument),
            },
            _ => Err(MotionPluginError::InvalidArgument),
        }
    }
    pub fn output_slice(&self) -> &[u8] {
        match &self.output_motion_data {
            Some(data) => data.as_slice(),
            None => &[],
        }
    }
    pub fn load_window_layout(&mut self) -> Result<()> {
        let mut components = vec![];
        components.push(nanoem_protobuf::create_label_component(format!(
            "format_type: {:?}",
            self.input_motion.format_type()
        ), None));
        components.push(nanoem_protobuf::create_label_component(format!(
            "max_frame_index: {}",
            self.input_motion.max_frame_index()
        ), None));
        components.push(nanoem_protobuf::create_label_component(format!(
            "target_model_name: {}",
            self.input_motion.target_model_name()?
        ), None));
        components.push(nanoem_protobuf::create_separator_component());
        self.layout_accessory_keyframes(&mut components);
        components.push(nanoem_protobuf::create_separator_component());
        self.layout_bone_keyframes(&mut components)?;
        components.push(nanoem_protobuf::create_separator_component());
        self.layout_camera_keyframes(&mut components);
        components.push(nanoem_protobuf::create_separator_component());
        self.layout_light_keyframes(&mut components);
        components.push(nanoem_protobuf::create_separator_component());
        self.layout_model_keyframes(&mut components);
        components.push(nanoem_protobuf::create_separator_component());
        self.layout_morph_keyframes(&mut components)?;
        components.push(nanoem_protobuf::create_separator_component());
        self.layout_self_shadow_keyframes(&mut components);
        let window = nanoem_protobuf::create_window(
            "plugin_motion_sandbox".to_string(),
            "plugin_motion_sandbox".to_string(),
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
        value: MotionPluginError,
    ) -> nanoem_application_plugin_status_t {
        match CString::new(value.to_string(self.language)) {
            Ok(reason) => {
                self.failure_reason = Some(reason);
                nanoem_application_plugin_status_t::ERROR_REFER_REASON
            }
            Err(_) => nanoem_application_plugin_status_t::ERROR_NULL_OBJECT,
        }
    }
    fn save_output_motion_data(
        &mut self,
        mutable_motion: &mut nanoem::MutableMotion,
    ) -> Result<()> {
        let mut data: Vec<u8> = vec![];
        mutable_motion.save(&mut data, nanoem::MotionFormatType::NMD)?;
        self.output_motion_data = Some(data);
        Ok(())
    }
    fn update_layout(&mut self, id: &str, component: nanoem_protobuf::UiComponent) -> Result<()> {
        println!("id = {}, component = {:?}", id, component);
        Ok(())
    }
    fn layout_accessory_keyframes(&self, components: &mut Vec<nanoem_protobuf::UiComponent>) {
        let accessory_keyframes = self.input_motion.all_accessory_keyframes();
        components.push(nanoem_protobuf::create_label_component(format!(
            "num_accessory_keyframes: {}",
            accessory_keyframes.len()
        ), None));
        let mut selectables = vec![];
        for item in accessory_keyframes {
            let id = format!("accessory_keyframe:{}", item.frame_index());
            let text = format!("frame_index: {}", item.frame_index());
            selectables.push(nanoem_protobuf::create_selectable(id, text));
        }
        components.push(nanoem_protobuf::create_combobox_component(
            "accessory_keyframes".to_string(),
            "".to_string(),
            selectables,
        ));
    }
    fn layout_bone_keyframes(
        &self,
        components: &mut Vec<nanoem_protobuf::UiComponent>,
    ) -> Result<()> {
        let bone_keyframes = self.input_motion.all_bone_keyframes();
        components.push(nanoem_protobuf::create_label_component(format!(
            "num_bone_keyframes: {}",
            bone_keyframes.len()
        ), None));
        let mut selectables = vec![];
        for item in bone_keyframes {
            let id = format!("bone_keyframe:{}", item.frame_index());
            let text = format!(
                "name: {}, frame_index: {}",
                item.name()?,
                item.frame_index()
            );
            selectables.push(nanoem_protobuf::create_selectable(id, text));
        }
        components.push(nanoem_protobuf::create_combobox_component(
            "bone_keyframes".to_string(),
            "".to_string(),
            selectables,
        ));
        Ok(())
    }
    fn layout_camera_keyframes(&self, components: &mut Vec<nanoem_protobuf::UiComponent>) {
        let camera_keyframes = self.input_motion.all_camera_keyframes();
        components.push(nanoem_protobuf::create_label_component(format!(
            "num_camera_keyframes: {}",
            camera_keyframes.len()
        ), None));
        let mut selectables = vec![];
        for item in camera_keyframes {
            let id = format!("camera_keyframe:{}", item.frame_index());
            let text = format!("frame_index: {}", item.frame_index());
            selectables.push(nanoem_protobuf::create_selectable(id, text));
        }
        components.push(nanoem_protobuf::create_combobox_component(
            "camera_keyframes".to_string(),
            "".to_string(),
            selectables,
        ));
    }
    fn layout_light_keyframes(&self, components: &mut Vec<nanoem_protobuf::UiComponent>) {
        let light_keyframes = self.input_motion.all_light_keyframes();
        components.push(nanoem_protobuf::create_label_component(format!(
            "num_light_keyframes: {}",
            light_keyframes.len()
        ), None));
        let mut selectables = vec![];
        for item in light_keyframes {
            let id = format!("light_keyframe:{}", item.frame_index());
            let text = format!("frame_index: {}", item.frame_index());
            selectables.push(nanoem_protobuf::create_selectable(id, text));
        }
        components.push(nanoem_protobuf::create_combobox_component(
            "light_keyframes".to_string(),
            "".to_string(),
            selectables,
        ));
    }
    fn layout_model_keyframes(&self, components: &mut Vec<nanoem_protobuf::UiComponent>) {
        let model_keyframes = self.input_motion.all_model_keyframes();
        components.push(nanoem_protobuf::create_label_component(format!(
            "num_model_keyframes: {}",
            model_keyframes.len()
        ), None));
        let mut selectables = vec![];
        for item in model_keyframes {
            let id = format!("model_keyframe:{}", item.frame_index());
            let text = format!("frame_index: {}", item.frame_index());
            selectables.push(nanoem_protobuf::create_selectable(id, text));
        }
        components.push(nanoem_protobuf::create_combobox_component(
            "model_keyframes".to_string(),
            "".to_string(),
            selectables,
        ));
    }
    fn layout_morph_keyframes(
        &self,
        components: &mut Vec<nanoem_protobuf::UiComponent>,
    ) -> Result<()> {
        let morph_keyframes = self.input_motion.all_morph_keyframes();
        components.push(nanoem_protobuf::create_label_component(format!(
            "num_morph_keyframes: {}",
            morph_keyframes.len()
        ), None));
        let mut selectables = vec![];
        for item in morph_keyframes {
            let id = format!("morph_keyframe:{}", item.frame_index());
            let text = format!(
                "name: {}, frame_index: {}",
                item.name()?,
                item.frame_index()
            );
            selectables.push(nanoem_protobuf::create_selectable(id, text));
        }
        components.push(nanoem_protobuf::create_combobox_component(
            "morph_keyframes".to_string(),
            "".to_string(),
            selectables,
        ));
        Ok(())
    }
    fn layout_self_shadow_keyframes(&self, components: &mut Vec<nanoem_protobuf::UiComponent>) {
        let self_shadow_keyframes = self.input_motion.all_self_shadow_keyframes();
        components.push(nanoem_protobuf::create_label_component(format!(
            "num_self_shadow_keyframes: {}",
            self_shadow_keyframes.len()
        ), None));
        let mut selectables = vec![];
        for item in self_shadow_keyframes {
            let id = format!("self_shadow_keyframe:{}", item.frame_index());
            let text = format!("frame_index: {}", item.frame_index());
            selectables.push(nanoem_protobuf::create_selectable(id, text));
        }
        components.push(nanoem_protobuf::create_combobox_component(
            "self_shadow_keyframes".to_string(),
            "".to_string(),
            selectables,
        ));
    }
}
