/*
  Copyright (c) 2015-2023 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

pub use application::plugin::*;
pub use common::*;

use prost::Message;

pub mod common {
    include!(concat!(env!("OUT_DIR"), "/nanoem.common.rs"));
}
pub mod application {
    pub mod plugin {
        include!(concat!(env!("OUT_DIR"), "/nanoem.application.plugin.rs"));
    }
}

#[derive(Debug)]
pub struct DataError {
    description: String,
}
type Result<T> = std::result::Result<T, DataError>;

impl DataError {
    pub fn new(description: String) -> DataError {
        DataError { description }
    }
}

impl std::fmt::Display for DataError {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "{}", self.description)
    }
}

impl std::error::Error for DataError {
    fn description(&self) -> &str {
        &self.description
    }
}

pub fn create_separator_component() -> UiComponent {
    UiComponent {
        r#type: Some(ui_component::Type::Separator(ui_component::Separator {})),
    }
}

pub fn create_label_component(text: String, color: Option<u32>) -> UiComponent {
    let label = ui_component::Label { text, color };
    UiComponent {
        r#type: Some(ui_component::Type::Label(label)),
    }
}

pub fn create_selectable(id: String, text: String) -> ui_component::Selectable {
    ui_component::Selectable { id, text }
}

pub fn create_selectable_component(id: String, text: String) -> UiComponent {
    let selectable = create_selectable(id, text);
    UiComponent {
        r#type: Some(ui_component::Type::Selectable(selectable)),
    }
}

pub fn create_button_component(id: String, text: String) -> UiComponent {
    let button = ui_component::Button { id, text };
    UiComponent {
        r#type: Some(ui_component::Type::Button(button)),
    }
}

pub fn create_checkbox_component(id: String, text: String, value: bool) -> UiComponent {
    let checkbox = ui_component::CheckBox { id, text, value };
    UiComponent {
        r#type: Some(ui_component::Type::CheckBox(checkbox)),
    }
}

pub fn create_radio_button_component(id: String, text: String, value: bool) -> UiComponent {
    let radio_button = ui_component::RadioButton {
        id,
        text,
        active: value,
    };
    UiComponent {
        r#type: Some(ui_component::Type::RadioButton(radio_button)),
    }
}

pub fn create_combobox_component(
    id: String,
    text: String,
    selectables: Vec<ui_component::Selectable>,
) -> UiComponent {
    let combobox = ui_component::ComboBox {
        id,
        text,
        selected_index: 0,
        selectables,
    };
    UiComponent {
        r#type: Some(ui_component::Type::ComboBox(combobox)),
    }
}

pub fn create_drag_component(
    id: String,
    text: String,
    speed: f32,
    format: Option<String>,
    power: Option<f32>,
) -> UiComponent {
    let slider = ui_component::DragScalarN {
        data_type: 0, //ui_component::DataType::S8,
        num_components: 0,
        id,
        text,
        value: vec![],
        min: Some(vec![]),
        max: Some(vec![]),
        format,
        power,
        speed,
    };
    UiComponent {
        r#type: Some(ui_component::Type::DragScalarN(slider)),
    }
}

pub fn create_slider_component(
    id: String,
    text: String,
    format: Option<String>,
    power: Option<f32>,
) -> UiComponent {
    let slider = ui_component::SliderScalarN {
        data_type: 0, //ui_component::DataType::S8,
        num_components: 0,
        id,
        text,
        value: vec![],
        min: Some(vec![]),
        max: Some(vec![]),
        format,
        power,
    };
    UiComponent {
        r#type: Some(ui_component::Type::SliderScalarN(slider)),
    }
}

pub fn create_input_component(id: String, text: String, format: Option<String>) -> UiComponent {
    let slider = ui_component::InputScalarN {
        data_type: 0, //ui_component::DataType::S8,
        num_components: 0,
        id,
        text,
        value: vec![],
        step: Some(vec![]),
        step_fast: Some(vec![]),
        format,
    };
    UiComponent {
        r#type: Some(ui_component::Type::InputScalarN(slider)),
    }
}

pub fn create_window(id: String, title: String, items: Vec<UiComponent>) -> UiWindow {
    UiWindow {
        id,
        title: Some(title),
        items,
    }
}

pub fn divide_rational(value: &Rational) -> f64 {
    value.numerator as f64 / f64::from(value.denominator)
}

pub fn serialize_window(window: UiWindow, data: &mut Vec<u8>) -> Result<()> {
    data.reserve(window.encoded_len());
    window
        .encode(data)
        .map_err(|value| DataError::new(value.to_string()))
}

pub fn deserialize_component(data: &[u8]) -> Result<UiComponent> {
    UiComponent::decode(data).map_err(|value| DataError::new(value.to_string()))
}

pub fn deserialize_audio_description(data: &[u8]) -> Result<AudioDescription> {
    AudioDescription::decode(data).map_err(|value| DataError::new(value.to_string()))
}

pub fn deserialize_camera_description(data: &[u8]) -> Result<CameraDescription> {
    CameraDescription::decode(data).map_err(|value| DataError::new(value.to_string()))
}

pub fn deserialize_light_description(data: &[u8]) -> Result<LightDescription> {
    LightDescription::decode(data).map_err(|value| DataError::new(value.to_string()))
}
