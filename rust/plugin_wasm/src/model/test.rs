/*
  Copyright (c) 2015-2021 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

use std::{collections::HashMap, env::current_dir};

use anyhow::Result;
use maplit::hashmap;
use rand::{thread_rng, Rng};
use serde_derive::{Deserialize, Serialize};
use serde_json::{json, Value};
use wasmer::Store;
use wasmer_wasi::{Pipe, WasiEnv, WasiState};

use super::plugin::ModelIOPluginController;

#[derive(Debug, Default, PartialEq, Eq, Serialize, Deserialize)]
struct PluginOutput {
    function: String,
    arguments: Option<HashMap<String, Value>>,
}

fn create_wasi_env() -> Result<WasiEnv> {
    let stdout = Box::new(Pipe::new());
    Ok(WasiState::new("nanoem").stdout(stdout).finalize()?)
}

fn create_controller(env: &mut WasiEnv) -> Result<ModelIOPluginController> {
    let path = current_dir()?.parent().unwrap().join(
        "plugin_wasm_test_model_full/target/wasm32-wasi/release/deps/plugin_wasm_test_model_full.wasm",
    );
    let store = Store::default();
    ModelIOPluginController::new(&path, &store, env)
}

fn create_random_data(size: usize) -> Vec<u8> {
    let mut data = vec![];
    let mut rng = thread_rng();
    for _ in 0..size {
        let v: u8 = rng.gen();
        data.push(v);
    }
    data
}

fn flush_plugin_output(env: &mut WasiEnv) -> Result<()> {
    let mut state = env.state();
    let stdout = state.fs.stdout_mut()?.as_mut().unwrap();
    let mut v = vec![];
    stdout.read_to_end(&mut v)?;
    Ok(())
}

fn read_plugin_output(env: &mut WasiEnv) -> Result<Vec<PluginOutput>> {
    let mut state = env.state();
    let stdout = state.fs.stdout_mut()?.as_mut().unwrap();
    let mut s = String::new();
    stdout.read_to_string(&mut s)?;
    let mut data: Vec<_> = s.split('\n').collect();
    data.pop();
    Ok(data
        .iter()
        .map(|s| serde_json::from_str(s).unwrap())
        .collect())
}

#[test]
fn create() -> Result<()> {
    let mut env = create_wasi_env()?;
    let result = create_controller(&mut env);
    assert!(result.is_ok());
    let mut controller = result?;
    assert!(controller.initialize().is_ok());
    assert!(controller.create().is_ok());
    assert_eq!(
        vec![
            PluginOutput {
                function: "nanoemApplicationPluginModelIOInitialize".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOCreate".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOCountAllFunctions".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOGetName".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOGetVersion".to_owned(),
                ..Default::default()
            },
        ],
        read_plugin_output(&mut env)?
    );
    assert_eq!(1, controller.count_all_functions());
    assert_eq!(
        "plugin_wasm_test_model_full: function0 (1.2.3)",
        controller.function_name(0)?.to_str()?
    );
    controller.destroy();
    controller.terminate();
    assert_eq!(
        vec![
            PluginOutput {
                function: "nanoemApplicationPluginModelIODestroy".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOTerminate".to_owned(),
                ..Default::default()
            },
        ],
        read_plugin_output(&mut env)?
    );
    Ok(())
}

#[test]
fn set_language() -> Result<()> {
    let mut env = create_wasi_env()?;
    let mut controller = create_controller(&mut env)?;
    controller.initialize()?;
    controller.create()?;
    controller.set_function(0)?;
    flush_plugin_output(&mut env)?;
    assert!(controller.set_language(0).is_ok());
    assert_eq!(
        vec![PluginOutput {
            function: "nanoemApplicationPluginModelIOSetLanguage".to_owned(),
            arguments: Some(hashmap! {
                "value".to_owned() => json!(0),
            })
        },],
        read_plugin_output(&mut env)?
    );
    controller.destroy();
    controller.terminate();
    Ok(())
}

#[test]
fn execute() -> Result<()> {
    let mut env = create_wasi_env()?;
    let mut controller = create_controller(&mut env)?;
    let data = create_random_data(4096);
    controller.initialize()?;
    controller.create()?;
    controller.set_function(0)?;
    flush_plugin_output(&mut env)?;
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
        read_plugin_output(&mut env)?
    );
    controller.destroy();
    controller.terminate();
    Ok(())
}

#[test]
fn set_all_selected_vertex_indices() -> Result<()> {
    let mut env = create_wasi_env()?;
    let mut controller = create_controller(&mut env)?;
    let data = &[1, 4, 9, 16, 25, i32::MAX];
    controller.initialize()?;
    controller.create()?;
    controller.set_function(0)?;
    flush_plugin_output(&mut env)?;
    assert!(controller.set_all_selected_vertex_indices(data).is_ok());
    assert_eq!(
        vec![PluginOutput {
            function: "nanoemApplicationPluginModelIOSetAllSelectedVertexObjectIndices".to_owned(),
            arguments: Some(hashmap! {
                "data".to_owned() => json!(data),
                "length".to_owned() => json!(data.len()),
                "status".to_owned() => json!(0),
            })
        },],
        read_plugin_output(&mut env)?
    );
    controller.destroy();
    controller.terminate();
    Ok(())
}

#[test]
fn set_all_selected_material_indices() -> Result<()> {
    let mut env = create_wasi_env()?;
    let mut controller = create_controller(&mut env)?;
    let data = &[1, 4, 9, 16, 25, i32::MAX];
    controller.initialize()?;
    controller.create()?;
    controller.set_function(0)?;
    flush_plugin_output(&mut env)?;
    assert!(controller.set_all_selected_material_indices(data).is_ok());
    assert_eq!(
        vec![PluginOutput {
            function: "nanoemApplicationPluginModelIOSetAllSelectedMaterialObjectIndices"
                .to_owned(),
            arguments: Some(hashmap! {
                "data".to_owned() => json!(data),
                "length".to_owned() => json!(data.len()),
                "status".to_owned() => json!(0),
            })
        },],
        read_plugin_output(&mut env)?
    );
    controller.destroy();
    controller.terminate();
    Ok(())
}

#[test]
fn set_all_selected_bone_indices() -> Result<()> {
    let mut env = create_wasi_env()?;
    let mut controller = create_controller(&mut env)?;
    let data = &[1, 4, 9, 16, 25, i32::MAX];
    controller.initialize()?;
    controller.create()?;
    controller.set_function(0)?;
    flush_plugin_output(&mut env)?;
    assert!(controller.set_all_selected_bone_indices(data).is_ok());
    assert_eq!(
        vec![PluginOutput {
            function: "nanoemApplicationPluginModelIOSetAllSelectedBoneObjectIndices".to_owned(),
            arguments: Some(hashmap! {
                "data".to_owned() => json!(data),
                "length".to_owned() => json!(data.len()),
                "status".to_owned() => json!(0),
            })
        },],
        read_plugin_output(&mut env)?
    );
    controller.destroy();
    controller.terminate();
    Ok(())
}

#[test]
fn set_all_selected_morph_indices() -> Result<()> {
    let mut env = create_wasi_env()?;
    let mut controller = create_controller(&mut env)?;
    let data = &[1, 4, 9, 16, 25, i32::MAX];
    controller.initialize()?;
    controller.create()?;
    controller.set_function(0)?;
    flush_plugin_output(&mut env)?;
    assert!(controller.set_all_selected_morph_indices(data).is_ok());
    assert_eq!(
        vec![PluginOutput {
            function: "nanoemApplicationPluginModelIOSetAllSelectedMorphObjectIndices".to_owned(),
            arguments: Some(hashmap! {
                "data".to_owned() => json!(data),
                "length".to_owned() => json!(data.len()),
                "status".to_owned() => json!(0),
            })
        },],
        read_plugin_output(&mut env)?
    );
    controller.destroy();
    controller.terminate();
    Ok(())
}

#[test]
fn set_all_selected_label_indices() -> Result<()> {
    let mut env = create_wasi_env()?;
    let mut controller = create_controller(&mut env)?;
    let data = &[1, 4, 9, 16, 25, i32::MAX];
    controller.initialize()?;
    controller.create()?;
    controller.set_function(0)?;
    flush_plugin_output(&mut env)?;
    assert!(controller.set_all_selected_label_indices(data).is_ok());
    assert_eq!(
        vec![PluginOutput {
            function: "nanoemApplicationPluginModelIOSetAllSelectedLabelObjectIndices".to_owned(),
            arguments: Some(hashmap! {
                "data".to_owned() => json!(data),
                "length".to_owned() => json!(data.len()),
                "status".to_owned() => json!(0),
            })
        },],
        read_plugin_output(&mut env)?
    );
    controller.destroy();
    controller.terminate();
    Ok(())
}

#[test]
fn set_all_selected_rigid_body_indices() -> Result<()> {
    let mut env = create_wasi_env()?;
    let mut controller = create_controller(&mut env)?;
    let data = &[1, 4, 9, 16, 25, i32::MAX];
    controller.initialize()?;
    controller.create()?;
    controller.set_function(0)?;
    flush_plugin_output(&mut env)?;
    assert!(controller.set_all_selected_rigid_body_indices(data).is_ok());
    assert_eq!(
        vec![PluginOutput {
            function: "nanoemApplicationPluginModelIOSetAllSelectedRigidBodyObjectIndices"
                .to_owned(),
            arguments: Some(hashmap! {
                "data".to_owned() => json!(data),
                "length".to_owned() => json!(data.len()),
                "status".to_owned() => json!(0),
            })
        },],
        read_plugin_output(&mut env)?
    );
    controller.destroy();
    controller.terminate();
    Ok(())
}

#[test]
fn set_all_selected_joint_indices() -> Result<()> {
    let mut env = create_wasi_env()?;
    let mut controller = create_controller(&mut env)?;
    let data = &[1, 4, 9, 16, 25, i32::MAX];
    controller.initialize()?;
    controller.create()?;
    controller.set_function(0)?;
    flush_plugin_output(&mut env)?;
    assert!(controller.set_all_selected_joint_indices(data).is_ok());
    assert_eq!(
        vec![PluginOutput {
            function: "nanoemApplicationPluginModelIOSetAllSelectedJointObjectIndices".to_owned(),
            arguments: Some(hashmap! {
                "data".to_owned() => json!(data),
                "length".to_owned() => json!(data.len()),
                "status".to_owned() => json!(0),
            })
        },],
        read_plugin_output(&mut env)?
    );
    controller.destroy();
    controller.terminate();
    Ok(())
}

#[test]
fn set_all_selected_soft_body_indices() -> Result<()> {
    let mut env = create_wasi_env()?;
    let mut controller = create_controller(&mut env)?;
    let data = &[1, 4, 9, 16, 25, i32::MAX];
    controller.initialize()?;
    controller.create()?;
    controller.set_function(0)?;
    flush_plugin_output(&mut env)?;
    assert!(controller.set_all_selected_soft_body_indices(data).is_ok());
    assert_eq!(
        vec![PluginOutput {
            function: "nanoemApplicationPluginModelIOSetAllSelectedSoftBodyObjectIndices"
                .to_owned(),
            arguments: Some(hashmap! {
                "data".to_owned() => json!(data),
                "length".to_owned() => json!(data.len()),
                "status".to_owned() => json!(0),
            })
        },],
        read_plugin_output(&mut env)?
    );
    controller.destroy();
    controller.terminate();
    Ok(())
}

#[test]
fn set_audio_description() -> Result<()> {
    let mut env = create_wasi_env()?;
    let mut controller = create_controller(&mut env)?;
    let data = create_random_data(4096);
    controller.initialize()?;
    controller.create()?;
    controller.set_function(0)?;
    flush_plugin_output(&mut env)?;
    assert!(controller.set_audio_description(&data).is_ok());
    assert_eq!(
        vec![PluginOutput {
            function: "nanoemApplicationPluginModelIOSetAudioDescription".to_owned(),
            arguments: Some(hashmap! {
                "data".to_owned() => json!(data),
                "length".to_owned() => json!(data.len()),
                "status".to_owned() => json!(0),
            })
        },],
        read_plugin_output(&mut env)?
    );
    controller.destroy();
    controller.terminate();
    Ok(())
}

#[test]
fn set_audio_data() -> Result<()> {
    let mut env = create_wasi_env()?;
    let mut controller = create_controller(&mut env)?;
    let data = create_random_data(4096);
    controller.initialize()?;
    controller.create()?;
    controller.set_function(0)?;
    flush_plugin_output(&mut env)?;
    assert!(controller.set_audio_data(&data).is_ok());
    assert_eq!(
        vec![PluginOutput {
            function: "nanoemApplicationPluginModelIOSetInputAudioData".to_owned(),
            arguments: Some(hashmap! {
                "data".to_owned() => json!(data),
                "length".to_owned() => json!(data.len()),
                "status".to_owned() => json!(0),
            })
        },],
        read_plugin_output(&mut env)?
    );
    controller.destroy();
    controller.terminate();
    Ok(())
}

#[test]
fn set_camera_description() -> Result<()> {
    let mut env = create_wasi_env()?;
    let mut controller = create_controller(&mut env)?;
    let data = create_random_data(4096);
    controller.initialize()?;
    controller.create()?;
    controller.set_function(0)?;
    flush_plugin_output(&mut env)?;
    assert!(controller.set_camera_description(&data).is_ok());
    assert_eq!(
        vec![PluginOutput {
            function: "nanoemApplicationPluginModelIOSetCameraDescription".to_owned(),
            arguments: Some(hashmap! {
                "data".to_owned() => json!(data),
                "length".to_owned() => json!(data.len()),
                "status".to_owned() => json!(0),
            })
        },],
        read_plugin_output(&mut env)?
    );
    controller.destroy();
    controller.terminate();
    Ok(())
}

#[test]
fn set_light_description() -> Result<()> {
    let mut env = create_wasi_env()?;
    let mut controller = create_controller(&mut env)?;
    let data = create_random_data(4096);
    controller.initialize()?;
    controller.create()?;
    controller.set_function(0)?;
    flush_plugin_output(&mut env)?;
    assert!(controller.set_light_description(&data).is_ok());
    assert_eq!(
        vec![PluginOutput {
            function: "nanoemApplicationPluginModelIOSetLightDescription".to_owned(),
            arguments: Some(hashmap! {
                "data".to_owned() => json!(data),
                "length".to_owned() => json!(data.len()),
                "status".to_owned() => json!(0),
            })
        },],
        read_plugin_output(&mut env)?
    );
    controller.destroy();
    controller.terminate();
    Ok(())
}

#[test]
fn ui_window() -> Result<()> {
    let mut env = create_wasi_env()?;
    let mut controller = create_controller(&mut env)?;
    let data = create_random_data(4096);
    controller.initialize()?;
    controller.create()?;
    controller.set_function(0)?;
    flush_plugin_output(&mut env)?;
    assert!(controller.load_ui_window_layout().is_ok());
    let output = controller.get_ui_window_layout();
    assert!(output.is_ok());
    let output_ui_layout_data = output?;
    let mut reload = false;
    assert!(controller
        .set_ui_component_layout("ui_window", &data, &mut reload)
        .is_ok());
    assert_eq!(
        vec![
            PluginOutput {
                function: "nanoemApplicationPluginModelIOLoadUIWindowLayout".to_owned(),
                arguments: Some(hashmap! {
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOGetUIWindowLayoutDataSize".to_owned(),
                arguments: Some(hashmap! {
                    "length".to_owned() => json!(output_ui_layout_data.len()),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOGetUIWindowLayoutData".to_owned(),
                arguments: Some(hashmap! {
                    "data".to_owned() => json!(output_ui_layout_data),
                    "length".to_owned() => json!(output_ui_layout_data.len()),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOSetUIComponentLayoutData".to_owned(),
                arguments: Some(hashmap! {
                    "id".to_owned() => json!("ui_window"),
                    "data".to_owned() => json!(data),
                    "length".to_owned() => json!(data.len()),
                    "status".to_owned() => json!(0),
                })
            },
        ],
        read_plugin_output(&mut env)?
    );
    controller.destroy();
    controller.terminate();
    Ok(())
}

#[test]
fn get_failure_reason() -> Result<()> {
    let mut env = create_wasi_env()?;
    let mut controller = create_controller(&mut env)?;
    controller.initialize()?;
    controller.create()?;
    controller.set_function(0)?;
    flush_plugin_output(&mut env)?;
    let result = controller.failure_reason();
    assert!(result.is_ok());
    assert_eq!("Failure Reason", result?);
    assert_eq!(
        vec![PluginOutput {
            function: "nanoemApplicationPluginModelIOGetFailureReason".to_owned(),
            ..Default::default()
        },],
        read_plugin_output(&mut env)?
    );
    controller.destroy();
    controller.terminate();
    Ok(())
}

#[test]
fn get_recovery_suggestion() -> Result<()> {
    let mut env = create_wasi_env()?;
    let mut controller = create_controller(&mut env)?;
    controller.initialize()?;
    controller.create()?;
    controller.set_function(0)?;
    flush_plugin_output(&mut env)?;
    let result = controller.recovery_suggestion();
    assert!(result.is_ok());
    assert_eq!("Recovery Suggestion", result?);
    assert_eq!(
        vec![PluginOutput {
            function: "nanoemApplicationPluginModelIOGetRecoverySuggestion".to_owned(),
            ..Default::default()
        },],
        read_plugin_output(&mut env)?
    );
    controller.destroy();
    controller.terminate();
    Ok(())
}
