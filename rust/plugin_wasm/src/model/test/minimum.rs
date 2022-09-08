/*
  Copyright (c) 2015-2021 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

use anyhow::{Context, Result};
use maplit::hashmap;
use pretty_assertions::assert_eq;
use serde_json::json;

use crate::model::{
    plugin::ModelIOPluginController,
    test::{create_random_data, flush_plugin_output, read_plugin_output, PluginOutput},
};

use super::{build_type_and_flags, inner_create_controller};

fn create_controller() -> Result<ModelIOPluginController> {
    let package = "plugin_wasm_test_model_minimum";
    let (ty, flag) = build_type_and_flags();
    inner_create_controller(&format!("target/wasm32-wasi/{}/{}.wasm", ty, package)).with_context(
        || {
            format!(
                "try build with \"cargo build --package {} --target wasm32-wasi{}\"",
                package, flag
            )
        },
    )
}

#[test]
fn create() -> Result<()> {
    let result = create_controller();
    assert!(result.is_ok());
    let mut controller = result?;
    assert!(controller.initialize().is_ok());
    assert!(controller.create().is_ok());
    assert_eq!(
        vec![
            PluginOutput {
                function: "nanoemApplicationPluginModelIOCreate".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOGetName".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOGetVersion".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOCountAllFunctions".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(1),
                })
            },
        ],
        read_plugin_output(&mut controller)?
    );
    assert_eq!(2, controller.count_all_functions());
    assert_eq!(
        "plugin_wasm_test_model_minimum: function0 (1.2.3)",
        controller.function_name(0)?.to_str()?
    );
    assert_eq!(
        "plugin_wasm_test_model_minimum: function0 (1.2.3)",
        controller.function_name(1)?.to_str()?
    );
    controller.destroy();
    controller.terminate();
    assert_eq!(
        vec![PluginOutput {
            function: "nanoemApplicationPluginModelIODestroy".to_owned(),
            ..Default::default()
        },],
        read_plugin_output(&mut controller)?
    );
    Ok(())
}

#[test]
fn set_language() -> Result<()> {
    let mut controller = create_controller()?;
    controller.initialize()?;
    controller.create()?;
    controller.set_function(0)?;
    flush_plugin_output(&mut controller)?;
    assert!(controller.set_language(0).is_ok());
    assert_eq!(
        vec![PluginOutput {
            function: "nanoemApplicationPluginModelIOSetLanguage".to_owned(),
            arguments: Some(hashmap! {
                "value".to_owned() => json!(0),
            })
        },],
        read_plugin_output(&mut controller)?
    );
    controller.destroy();
    controller.terminate();
    Ok(())
}

#[test]
fn execute() -> Result<()> {
    let mut controller = create_controller()?;
    let data = create_random_data(4096);
    controller.initialize()?;
    controller.create()?;
    controller.set_function(0)?;
    flush_plugin_output(&mut controller)?;
    assert!(controller.set_input_model_data(&data).is_ok());
    assert!(controller.execute().is_ok());
    let output = controller.get_output_data();
    assert!(output.is_ok());
    let output_model_data = output?;
    assert_eq!(
        vec![
            PluginOutput {
                function: "nanoemApplicationPluginModelIOSetInputModelData".to_owned(),
                arguments: Some(hashmap! {
                    "data".to_owned() => json!(data),
                    "length".to_owned() => json!(data.len()),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOExecute".to_owned(),
                arguments: Some(hashmap! {
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOGetOutputModelDataSize".to_owned(),
                arguments: Some(hashmap! {
                    "length".to_owned() => json!(output_model_data.len()),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOGetOutputModelData".to_owned(),
                arguments: Some(hashmap! {
                    "data".to_owned() => json!(output_model_data),
                    "length".to_owned() => json!(output_model_data.len()),
                    "status".to_owned() => json!(0),
                })
            },
        ],
        read_plugin_output(&mut controller)?
    );
    controller.destroy();
    controller.terminate();
    Ok(())
}

#[test]
fn set_all_selected_vertex_indices() -> Result<()> {
    let mut controller = create_controller()?;
    let data = &[1, 4, 9, 16, 25, i32::MAX];
    controller.initialize()?;
    controller.create()?;
    controller.set_function(0)?;
    flush_plugin_output(&mut controller)?;
    assert!(controller.set_all_selected_vertex_indices(data).is_ok());
    assert!(read_plugin_output(&mut controller)?.is_empty());
    controller.destroy();
    controller.terminate();
    Ok(())
}

#[test]
fn set_all_selected_material_indices() -> Result<()> {
    let mut controller = create_controller()?;
    let data = &[1, 4, 9, 16, 25, i32::MAX];
    controller.initialize()?;
    controller.create()?;
    controller.set_function(0)?;
    flush_plugin_output(&mut controller)?;
    assert!(controller.set_all_selected_material_indices(data).is_ok());
    assert!(read_plugin_output(&mut controller)?.is_empty());
    controller.destroy();
    controller.terminate();
    Ok(())
}

#[test]
fn set_all_selected_bone_indices() -> Result<()> {
    let mut controller = create_controller()?;
    let data = &[1, 4, 9, 16, 25, i32::MAX];
    controller.initialize()?;
    controller.create()?;
    controller.set_function(0)?;
    flush_plugin_output(&mut controller)?;
    assert!(controller.set_all_selected_bone_indices(data).is_ok());
    assert!(read_plugin_output(&mut controller)?.is_empty());
    controller.destroy();
    controller.terminate();
    Ok(())
}

#[test]
fn set_all_selected_morph_indices() -> Result<()> {
    let mut controller = create_controller()?;
    let data = &[1, 4, 9, 16, 25, i32::MAX];
    controller.initialize()?;
    controller.create()?;
    controller.set_function(0)?;
    flush_plugin_output(&mut controller)?;
    assert!(controller.set_all_selected_morph_indices(data).is_ok());
    assert!(read_plugin_output(&mut controller)?.is_empty());
    controller.destroy();
    controller.terminate();
    Ok(())
}

#[test]
fn set_all_selected_label_indices() -> Result<()> {
    let mut controller = create_controller()?;
    let data = &[1, 4, 9, 16, 25, i32::MAX];
    controller.initialize()?;
    controller.create()?;
    controller.set_function(0)?;
    flush_plugin_output(&mut controller)?;
    assert!(controller.set_all_selected_label_indices(data).is_ok());
    assert!(read_plugin_output(&mut controller)?.is_empty());
    controller.destroy();
    controller.terminate();
    Ok(())
}

#[test]
fn set_all_selected_rigid_body_indices() -> Result<()> {
    let mut controller = create_controller()?;
    let data = &[1, 4, 9, 16, 25, i32::MAX];
    controller.initialize()?;
    controller.create()?;
    controller.set_function(0)?;
    flush_plugin_output(&mut controller)?;
    assert!(controller.set_all_selected_rigid_body_indices(data).is_ok());
    assert!(read_plugin_output(&mut controller)?.is_empty());
    controller.destroy();
    controller.terminate();
    Ok(())
}

#[test]
fn set_all_selected_joint_indices() -> Result<()> {
    let mut controller = create_controller()?;
    let data = &[1, 4, 9, 16, 25, i32::MAX];
    controller.initialize()?;
    controller.create()?;
    controller.set_function(0)?;
    flush_plugin_output(&mut controller)?;
    assert!(controller.set_all_selected_joint_indices(data).is_ok());
    assert!(read_plugin_output(&mut controller)?.is_empty());
    controller.destroy();
    controller.terminate();
    Ok(())
}

#[test]
fn set_all_selected_soft_body_indices() -> Result<()> {
    let mut controller = create_controller()?;
    let data = &[1, 4, 9, 16, 25, i32::MAX];
    controller.initialize()?;
    controller.create()?;
    controller.set_function(0)?;
    flush_plugin_output(&mut controller)?;
    assert!(controller.set_all_selected_soft_body_indices(data).is_ok());
    assert!(read_plugin_output(&mut controller)?.is_empty());
    controller.destroy();
    controller.terminate();
    Ok(())
}

#[test]
fn set_audio_description() -> Result<()> {
    let mut controller = create_controller()?;
    let data = create_random_data(4096);
    controller.initialize()?;
    controller.create()?;
    controller.set_function(0)?;
    flush_plugin_output(&mut controller)?;
    assert!(controller.set_audio_description(&data).is_ok());
    assert!(read_plugin_output(&mut controller)?.is_empty());
    controller.destroy();
    controller.terminate();
    Ok(())
}

#[test]
fn set_audio_data() -> Result<()> {
    let mut controller = create_controller()?;
    let data = create_random_data(4096);
    controller.initialize()?;
    controller.create()?;
    controller.set_function(0)?;
    flush_plugin_output(&mut controller)?;
    assert!(controller.set_audio_data(&data).is_ok());
    assert!(read_plugin_output(&mut controller)?.is_empty());
    controller.destroy();
    controller.terminate();
    Ok(())
}

#[test]
fn set_camera_description() -> Result<()> {
    let mut controller = create_controller()?;
    let data = create_random_data(4096);
    controller.initialize()?;
    controller.create()?;
    controller.set_function(0)?;
    flush_plugin_output(&mut controller)?;
    assert!(controller.set_camera_description(&data).is_ok());
    assert!(read_plugin_output(&mut controller)?.is_empty());
    controller.destroy();
    controller.terminate();
    Ok(())
}

#[test]
fn set_light_description() -> Result<()> {
    let mut controller = create_controller()?;
    let data = create_random_data(4096);
    controller.initialize()?;
    controller.create()?;
    controller.set_function(0)?;
    flush_plugin_output(&mut controller)?;
    assert!(controller.set_light_description(&data).is_ok());
    assert!(read_plugin_output(&mut controller)?.is_empty());
    controller.destroy();
    controller.terminate();
    Ok(())
}

#[test]
fn ui_window() -> Result<()> {
    let mut controller = create_controller()?;
    let data = create_random_data(4096);
    controller.initialize()?;
    controller.create()?;
    controller.set_function(0)?;
    flush_plugin_output(&mut controller)?;
    assert!(controller.load_ui_window_layout().is_ok());
    let output = controller.get_ui_window_layout();
    assert!(output.is_ok());
    assert!(output?.is_empty());
    let mut reload = false;
    assert!(controller
        .set_ui_component_layout("ui_window", &data, &mut reload)
        .is_ok());
    assert!(read_plugin_output(&mut controller)?.is_empty());
    controller.destroy();
    controller.terminate();
    Ok(())
}

#[test]
fn get_failure_reason() -> Result<()> {
    let mut controller = create_controller()?;
    controller.initialize()?;
    controller.create()?;
    controller.set_function(1)?;
    controller.execute().unwrap_or_default();
    flush_plugin_output(&mut controller)?;
    let result = controller.failure_reason();
    assert!(result.is_some());
    assert_eq!("Failure Reason", result.unwrap());
    assert!(read_plugin_output(&mut controller)?.is_empty());
    controller.destroy();
    controller.terminate();
    Ok(())
}

#[test]
fn get_recovery_suggestion() -> Result<()> {
    let mut controller = create_controller()?;
    controller.initialize()?;
    controller.create()?;
    controller.set_function(1)?;
    controller.execute().unwrap_or_default();
    flush_plugin_output(&mut controller)?;
    let result = controller.recovery_suggestion();
    assert!(result.is_none());
    assert!(read_plugin_output(&mut controller)?.is_empty());
    controller.destroy();
    controller.terminate();
    Ok(())
}
