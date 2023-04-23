/*
  Copyright (c) 2015-2023 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

use anyhow::{Context, Result};
use assert_matches::assert_matches;
use maplit::hashmap;
use pretty_assertions::assert_eq;
use serde_json::json;
use wasmer_wasix::Pipe;

use crate::motion::{
    plugin::MotionIOPluginController,
    test::{create_random_data, inner_create_controller, read_plugin_output, PluginOutput},
};

use super::build_type_and_flags;

fn create_controller(stdout: Pipe) -> Result<MotionIOPluginController> {
    let package = "plugin_wasm_test_motion_full";
    let (ty, flag) = build_type_and_flags();
    inner_create_controller( stdout, &format!("target/wasm32-wasi/{ty}/{package}.wasm"))
        .with_context(|| {
            format!(
                "try build with \"cargo build --package {package} --target wasm32-wasi{flag}\"",
            )
        })
}

#[test]
fn create() -> Result<()> {
    let (stdout_writer, stdout_reader) = Pipe::channel();
    {
        let result = create_controller(stdout_writer);
        assert!(result.is_ok());
        let mut controller = result?;
        assert_matches!(controller.initialize(), Ok(_));
        assert_matches!(controller.create(), Ok(_));
        assert_eq!(2, controller.count_all_functions());
        assert_eq!(
            "plugin_wasm_test_motion_full: function0 (1.2.3)",
            controller.function_name(0)?.to_str()?
        );
        assert_eq!(
            "plugin_wasm_test_motion_full: function0 (1.2.3)",
            controller.function_name(1)?.to_str()?
        );
        controller.destroy();
        controller.terminate();
    }
    assert_eq!(
        vec![
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOInitialize".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOCreate".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetName".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetVersion".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOCountAllFunctions".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(1),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIODestroy".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOTerminate".to_owned(),
                ..Default::default()
            },
        ],
        read_plugin_output(stdout_reader)?
    );
    Ok(())
}

#[test]
fn set_language() -> Result<()> {
    let (stdout_writer, stdout_reader) = Pipe::channel();
    {
        let mut controller = create_controller(stdout_writer)?;
        controller.initialize()?;
        controller.create()?;
        controller.set_function(0)?;
        assert_matches!(controller.set_language(0), Ok(_));
        controller.destroy();
        controller.terminate();
    }
    assert_eq!(
        vec![
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOInitialize".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOCreate".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetName".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetVersion".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOCountAllFunctions".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(1),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOSetFunction".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOSetLanguage".to_owned(),
                arguments: Some(hashmap! {
                    "value".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIODestroy".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOTerminate".to_owned(),
                ..Default::default()
            },
        ],
        read_plugin_output(stdout_reader)?
    );
    Ok(())
}

#[test]
fn execute() -> Result<()> {
    let (stdout_writer, stdout_reader) = Pipe::channel();
    let model_data: Vec<u8> = create_random_data(4096);
    let motion_data: Vec<u8> = create_random_data(4096);
    let output_motion_data = {
        let mut controller = create_controller(stdout_writer)?;
        controller.initialize()?;
        controller.create()?;
        controller.set_function(0)?;
        assert_matches!(controller.set_input_model_data(&model_data), Ok(_));
        assert_matches!(controller.set_input_motion_data(&motion_data), Ok(_));
        assert_matches!(controller.execute(), Ok(_));
        let output = controller.get_output_data();
        assert_matches!(output, Ok(_));
        let output_motion_data = output?;
        controller.destroy();
        controller.terminate();
        output_motion_data
    };
    assert_eq!(
        vec![
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOInitialize".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOCreate".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetName".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetVersion".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOCountAllFunctions".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(1),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOSetFunction".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOSetInputActiveModelData".to_owned(),
                arguments: Some(hashmap! {
                    "data".to_owned() => json!(model_data),
                    "length".to_owned() => json!(model_data.len()),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOSetInputMotionData".to_owned(),
                arguments: Some(hashmap! {
                    "data".to_owned() => json!(motion_data),
                    "length".to_owned() => json!(motion_data.len()),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOExecute".to_owned(),
                arguments: Some(hashmap! {
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetOutputMotionDataSize".to_owned(),
                arguments: Some(hashmap! {
                    "length".to_owned() => json!(output_motion_data.len()),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetOutputMotionData".to_owned(),
                arguments: Some(hashmap! {
                    "data".to_owned() => json!(output_motion_data),
                    "length".to_owned() => json!(output_motion_data.len()),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIODestroy".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOTerminate".to_owned(),
                ..Default::default()
            },
        ],
        read_plugin_output(stdout_reader)?
    );
    Ok(())
}

#[test]
fn set_all_selected_accessory_keyframes() -> Result<()> {
    let (stdout_writer, stdout_reader) = Pipe::channel();
    let data = &[1, 4, 9, 16, 25, u32::MAX];
    {
        let mut controller = create_controller(stdout_writer)?;
        controller.initialize()?;
        controller.create()?;
        controller.set_function(0)?;
        assert_matches!(controller.set_all_selected_accessory_keyframes(data), Ok(_));
        controller.destroy();
        controller.terminate();
        data
    };
    assert_eq!(
        vec![
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOInitialize".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOCreate".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetName".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetVersion".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOCountAllFunctions".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(1),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOSetFunction".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOSetAllSelectedAccessoryKeyframes"
                    .to_owned(),
                arguments: Some(hashmap! {
                    "frameIndices".to_owned() => json!(data),
                    "length".to_owned() => json!(data.len()),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIODestroy".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOTerminate".to_owned(),
                ..Default::default()
            },
        ],
        read_plugin_output(stdout_reader)?
    );
    Ok(())
}

#[test]
fn set_all_selected_camera_keyframes() -> Result<()> {
    let (stdout_writer, stdout_reader) = Pipe::channel();
    let data = &[1, 4, 9, 16, 25, u32::MAX];
    {
        let mut controller = create_controller(stdout_writer)?;
        controller.initialize()?;
        controller.create()?;
        controller.set_function(0)?;
        assert_matches!(controller.set_all_selected_camera_keyframes(data), Ok(_));
        controller.destroy();
        controller.terminate();
    }
    assert_eq!(
        vec![
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOInitialize".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOCreate".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetName".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetVersion".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOCountAllFunctions".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(1),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOSetFunction".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOSetAllSelectedCameraKeyframes".to_owned(),
                arguments: Some(hashmap! {
                    "frameIndices".to_owned() => json!(data),
                    "length".to_owned() => json!(data.len()),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIODestroy".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOTerminate".to_owned(),
                ..Default::default()
            },
        ],
        read_plugin_output(stdout_reader)?
    );
    Ok(())
}

#[test]
fn set_all_selected_light_keyframes() -> Result<()> {
    let (stdout_writer, stdout_reader) = Pipe::channel();
    let data = &[1, 4, 9, 16, 25, u32::MAX];
    {
        let mut controller = create_controller(stdout_writer)?;
        controller.initialize()?;
        controller.create()?;
        controller.set_function(0)?;
        assert_matches!(controller.set_all_selected_light_keyframes(data), Ok(_));
        controller.destroy();
        controller.terminate();
    }
    assert_eq!(
        vec![
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOInitialize".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOCreate".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetName".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetVersion".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOCountAllFunctions".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(1),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOSetFunction".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOSetAllSelectedLightKeyframes".to_owned(),
                arguments: Some(hashmap! {
                    "frameIndices".to_owned() => json!(data),
                    "length".to_owned() => json!(data.len()),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIODestroy".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOTerminate".to_owned(),
                ..Default::default()
            },
        ],
        read_plugin_output(stdout_reader)?
    );
    Ok(())
}

#[test]
fn set_all_selected_model_keyframes() -> Result<()> {
    let (stdout_writer, stdout_reader) = Pipe::channel();
    let data = &[1, 4, 9, 16, 25, u32::MAX];
    {
        let mut controller = create_controller(stdout_writer)?;
        controller.initialize()?;
        controller.create()?;
        controller.set_function(0)?;
        assert_matches!(controller.set_all_selected_model_keyframes(data), Ok(_));
        controller.destroy();
        controller.terminate();
    }
    assert_eq!(
        vec![
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOInitialize".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOCreate".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetName".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetVersion".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOCountAllFunctions".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(1),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOSetFunction".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOSetAllSelectedModelKeyframes".to_owned(),
                arguments: Some(hashmap! {
                    "frameIndices".to_owned() => json!(data),
                    "length".to_owned() => json!(data.len()),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIODestroy".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOTerminate".to_owned(),
                ..Default::default()
            },
        ],
        read_plugin_output(stdout_reader)?
    );
    Ok(())
}

#[test]
fn set_all_selected_self_shadow_keyframes() -> Result<()> {
    let (stdout_writer, stdout_reader) = Pipe::channel();
    let data = &[1, 4, 9, 16, 25, u32::MAX];
    {
        let mut controller = create_controller(stdout_writer)?;
        controller.initialize()?;
        controller.create()?;
        controller.set_function(0)?;
        assert_matches!(
            controller.set_all_selected_self_shadow_keyframes(data),
            Ok(_)
        );
        controller.destroy();
        controller.terminate();
    }
    assert_eq!(
        vec![
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOInitialize".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOCreate".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetName".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetVersion".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOCountAllFunctions".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(1),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOSetFunction".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOSetAllSelectedSelfShadowKeyframes"
                    .to_owned(),
                arguments: Some(hashmap! {
                    "frameIndices".to_owned() => json!(data),
                    "length".to_owned() => json!(data.len()),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIODestroy".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOTerminate".to_owned(),
                ..Default::default()
            },
        ],
        read_plugin_output(stdout_reader)?
    );
    Ok(())
}

#[test]
fn set_all_named_selected_bone_keyframes() -> Result<()> {
    let (stdout_writer, stdout_reader) = Pipe::channel();
    let data = &[1, 4, 9, 16, 25, u32::MAX];
    {
        let mut controller = create_controller(stdout_writer)?;
        controller.initialize()?;
        controller.create()?;
        controller.set_function(0)?;
        assert_matches!(
            controller.set_all_named_selected_bone_keyframes("bone", data),
            Ok(_)
        );
        controller.destroy();
        controller.terminate();
    }
    assert_eq!(
        vec![
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOInitialize".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOCreate".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetName".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetVersion".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOCountAllFunctions".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(1),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOSetFunction".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOSetAllNamedSelectedBoneKeyframes"
                    .to_owned(),
                arguments: Some(hashmap! {
                    "name".to_owned() => json!("bone"),
                    "frameIndices".to_owned() => json!(data),
                    "length".to_owned() => json!(data.len()),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIODestroy".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOTerminate".to_owned(),
                ..Default::default()
            },
        ],
        read_plugin_output(stdout_reader)?
    );
    Ok(())
}

#[test]
fn set_all_named_selected_morph_keyframes() -> Result<()> {
    let (stdout_writer, stdout_reader) = Pipe::channel();
    let data = &[1, 4, 9, 16, 25, u32::MAX];
    {
        let mut controller = create_controller(stdout_writer)?;
        controller.initialize()?;
        controller.create()?;
        controller.set_function(0)?;
        assert_matches!(
            controller.set_all_named_selected_morph_keyframes("morph", data),
            Ok(_)
        );
        controller.destroy();
        controller.terminate();
    }
    assert_eq!(
        vec![
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOInitialize".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOCreate".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetName".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetVersion".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOCountAllFunctions".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(1),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOSetFunction".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOSetAllNamedSelectedMorphKeyframes"
                    .to_owned(),
                arguments: Some(hashmap! {
                    "name".to_owned() => json!("morph"),
                    "frameIndices".to_owned() => json!(data),
                    "length".to_owned() => json!(data.len()),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIODestroy".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOTerminate".to_owned(),
                ..Default::default()
            },
        ],
        read_plugin_output(stdout_reader)?
    );
    Ok(())
}

#[test]
fn set_audio_description() -> Result<()> {
    let (stdout_writer, stdout_reader) = Pipe::channel();
    let data = create_random_data(4096);
    {
        let mut controller = create_controller(stdout_writer)?;
        controller.initialize()?;
        controller.create()?;
        controller.set_function(0)?;
        assert_matches!(controller.set_audio_description(&data), Ok(_));
        controller.destroy();
        controller.terminate();
    }
    assert_eq!(
        vec![
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOInitialize".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOCreate".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetName".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetVersion".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOCountAllFunctions".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(1),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOSetFunction".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOSetAudioDescription".to_owned(),
                arguments: Some(hashmap! {
                    "data".to_owned() => json!(data),
                    "length".to_owned() => json!(data.len()),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIODestroy".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOTerminate".to_owned(),
                ..Default::default()
            },
        ],
        read_plugin_output(stdout_reader)?
    );
    Ok(())
}

#[test]
fn set_audio_data() -> Result<()> {
    let (stdout_writer, stdout_reader) = Pipe::channel();
    let data = create_random_data(4096);
    {
        let mut controller = create_controller(stdout_writer)?;
        controller.initialize()?;
        controller.create()?;
        controller.set_function(0)?;
        assert_matches!(controller.set_audio_data(&data), Ok(_));
        controller.destroy();
        controller.terminate();
    }
    assert_eq!(
        vec![
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOInitialize".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOCreate".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetName".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetVersion".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOCountAllFunctions".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(1),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOSetFunction".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOSetInputAudioData".to_owned(),
                arguments: Some(hashmap! {
                    "data".to_owned() => json!(data),
                    "length".to_owned() => json!(data.len()),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIODestroy".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOTerminate".to_owned(),
                ..Default::default()
            },
        ],
        read_plugin_output(stdout_reader)?
    );
    Ok(())
}

#[test]
fn set_camera_description() -> Result<()> {
    let (stdout_writer, stdout_reader) = Pipe::channel();
    let data = create_random_data(4096);
    {
        let mut controller = create_controller(stdout_writer)?;
        controller.initialize()?;
        controller.create()?;
        controller.set_function(0)?;
        assert_matches!(controller.set_camera_description(&data), Ok(_));
        controller.destroy();
        controller.terminate();
    }
    assert_eq!(
        vec![
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOInitialize".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOCreate".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetName".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetVersion".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOCountAllFunctions".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(1),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOSetFunction".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOSetCameraDescription".to_owned(),
                arguments: Some(hashmap! {
                    "data".to_owned() => json!(data),
                    "length".to_owned() => json!(data.len()),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIODestroy".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOTerminate".to_owned(),
                ..Default::default()
            },
        ],
        read_plugin_output(stdout_reader)?
    );
    Ok(())
}

#[test]
fn set_light_description() -> Result<()> {
    let (stdout_writer, stdout_reader) = Pipe::channel();
    let data = create_random_data(4096);
    {
        let mut controller = create_controller(stdout_writer)?;
        controller.initialize()?;
        controller.create()?;
        controller.set_function(0)?;
        assert_matches!(controller.set_light_description(&data), Ok(_));
        controller.destroy();
        controller.terminate();
    }
    assert_eq!(
        vec![
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOInitialize".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOCreate".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetName".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetVersion".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOCountAllFunctions".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(1),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOSetFunction".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOSetLightDescription".to_owned(),
                arguments: Some(hashmap! {
                    "data".to_owned() => json!(data),
                    "length".to_owned() => json!(data.len()),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIODestroy".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOTerminate".to_owned(),
                ..Default::default()
            },
        ],
        read_plugin_output(stdout_reader)?
    );
    Ok(())
}

#[test]
fn ui_window() -> Result<()> {
    let (stdout_writer, stdout_reader) = Pipe::channel();
    let data = create_random_data(4096);
    let output_ui_layout_data = {
        let mut controller = create_controller(stdout_writer)?;
        controller.initialize()?;
        controller.create()?;
        controller.set_function(0)?;
        assert_matches!(controller.load_ui_window_layout(), Ok(_));
        let output = controller.get_ui_window_layout();
        assert_matches!(output, Ok(_));
        let output = output?;
        assert!(output.is_empty());
        let mut reload = false;
        assert!(controller
            .set_ui_component_layout("ui_window", &data, &mut reload)
            .is_ok());
        controller.destroy();
        controller.terminate();
        output
    };
    assert_eq!(
        vec![
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOInitialize".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOCreate".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetName".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetVersion".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOCountAllFunctions".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(1),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOSetFunction".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOLoadUIWindowLayout".to_owned(),
                arguments: Some(hashmap! {
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetUIWindowLayoutDataSize".to_owned(),
                arguments: Some(hashmap! {
                    "length".to_owned() => json!(output_ui_layout_data.len()),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetUIWindowLayoutData".to_owned(),
                arguments: Some(hashmap! {
                    "data".to_owned() => json!(output_ui_layout_data),
                    "length".to_owned() => json!(output_ui_layout_data.len()),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOSetUIComponentLayoutData".to_owned(),
                arguments: Some(hashmap! {
                    "id".to_owned() => json!("ui_window"),
                    "data".to_owned() => json!(data),
                    "length".to_owned() => json!(data.len()),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIODestroy".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOTerminate".to_owned(),
                ..Default::default()
            },
        ],
        read_plugin_output(stdout_reader)?
    );
    Ok(())
}

#[test]
fn get_failure_reason() -> Result<()> {
    let (stdout_writer, stdout_reader) = Pipe::channel();
    {
        let mut controller = create_controller(stdout_writer)?;
        controller.initialize()?;
        controller.create()?;
        controller.set_function(1)?;
        controller.execute().unwrap_or_default();
        let result = controller.failure_reason();
        assert!(result.is_some());
        assert_eq!("Failure Reason", result.unwrap());
        controller.destroy();
        controller.terminate();
    }
    assert_eq!(
        vec![
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOInitialize".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOCreate".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetName".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetVersion".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOCountAllFunctions".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(1),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOSetFunction".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(1),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOExecute".to_owned(),
                arguments: Some(hashmap! {
                    "status".to_owned() => json!(-1),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetFailureReason".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetRecoverySuggestion".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIODestroy".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOTerminate".to_owned(),
                ..Default::default()
            },
        ],
        read_plugin_output(stdout_reader)?
    );
    Ok(())
}

#[test]
fn get_recovery_suggestion() -> Result<()> {
    let (stdout_writer, stdout_reader) = Pipe::channel();
    {
        let mut controller = create_controller(stdout_writer)?;
        controller.initialize()?;
        controller.create()?;
        controller.set_function(1)?;
        controller.execute().unwrap_or_default();
        let result = controller.recovery_suggestion();
        assert!(result.is_some());
        assert_eq!("Recovery Suggestion", result.unwrap());
        controller.destroy();
        controller.terminate();
    }
    assert_eq!(
        vec![
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOInitialize".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOCreate".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetName".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetVersion".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOCountAllFunctions".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetFunctionName".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(1),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOSetFunction".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(1),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOExecute".to_owned(),
                arguments: Some(hashmap! {
                    "status".to_owned() => json!(-1),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetFailureReason".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOGetRecoverySuggestion".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIODestroy".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginMotionIOTerminate".to_owned(),
                ..Default::default()
            },
        ],
        read_plugin_output(stdout_reader)?
    );
    Ok(())
}
