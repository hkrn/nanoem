/*
  Copyright (c) 2015-2023 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

use anyhow::{Context, Result};
use assert_matches::assert_matches;
use maplit::hashmap;
use pretty_assertions::assert_eq;
use serde_json::json;
use wasmtime_wasi::WasiFile;

use crate::model::{
    plugin::ModelIOPluginController,
    test::{create_random_data, read_plugin_output, Pipe, PluginOutput},
};

use super::{build_type_and_flags, inner_create_controller};

fn create_controller(pipe: Box<dyn WasiFile>) -> Result<ModelIOPluginController> {
    let package = "plugin_wasm_test_model_full";
    let (ty, flag) = build_type_and_flags();
    inner_create_controller(pipe, &format!("target/wasm32-wasi/{ty}/{package}.wasm")).with_context(
        || format!("try build with \"cargo build --package {package} --target wasm32-wasi{flag}\""),
    )
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
            "plugin_wasm_test_model_full: function0 (1.2.3)",
            controller.function_name(0)?.to_str()?
        );
        assert_eq!(
            "plugin_wasm_test_model_full: function0 (1.2.3)",
            controller.function_name(1)?.to_str()?
        );
        controller.destroy();
        controller.terminate();
    }
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
            PluginOutput {
                function: "nanoemApplicationPluginModelIODestroy".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOTerminate".to_owned(),
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
                function: "nanoemApplicationPluginModelIOInitialize".to_owned(),
                ..Default::default()
            },
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
            PluginOutput {
                function: "nanoemApplicationPluginModelIOSetFunction".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOSetLanguage".to_owned(),
                arguments: Some(hashmap! {
                    "value".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIODestroy".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOTerminate".to_owned(),
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
    let data = create_random_data(4096);
    let output_model_data = {
        let mut controller = create_controller(stdout_writer)?;
        controller.initialize()?;
        controller.create()?;
        controller.set_function(0)?;
        assert_matches!(controller.set_input_model_data(&data), Ok(_));
        assert_matches!(controller.execute(), Ok(_));
        let output = controller.get_output_data();
        assert_matches!(output, Ok(_));
        let output_model_data = output?;
        controller.destroy();
        controller.terminate();
        output_model_data
    };
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
            PluginOutput {
                function: "nanoemApplicationPluginModelIOSetFunction".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                    "status".to_owned() => json!(0),
                })
            },
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
            PluginOutput {
                function: "nanoemApplicationPluginModelIODestroy".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOTerminate".to_owned(),
                ..Default::default()
            },
        ],
        read_plugin_output(stdout_reader)?
    );
    Ok(())
}

#[test]
fn set_all_selected_vertex_indices() -> Result<()> {
    let (stdout_writer, stdout_reader) = Pipe::channel();
    let data = &[1, 4, 9, 16, 25, i32::MAX];
    {
        let mut controller = create_controller(stdout_writer)?;
        controller.initialize()?;
        controller.create()?;
        controller.set_function(0)?;
        assert_matches!(controller.set_all_selected_vertex_indices(data), Ok(_));
        controller.destroy();
        controller.terminate();
    }
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
            PluginOutput {
                function: "nanoemApplicationPluginModelIOSetFunction".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOSetAllSelectedVertexObjectIndices"
                    .to_owned(),
                arguments: Some(hashmap! {
                    "data".to_owned() => json!(data),
                    "length".to_owned() => json!(data.len()),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIODestroy".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOTerminate".to_owned(),
                ..Default::default()
            },
        ],
        read_plugin_output(stdout_reader)?
    );
    Ok(())
}

#[test]
fn set_all_selected_material_indices() -> Result<()> {
    let (stdout_writer, stdout_reader) = Pipe::channel();
    let data = &[1, 4, 9, 16, 25, i32::MAX];
    {
        let mut controller = create_controller(stdout_writer)?;
        controller.initialize()?;
        controller.create()?;
        controller.set_function(0)?;
        assert_matches!(controller.set_all_selected_material_indices(data), Ok(_));
        controller.destroy();
        controller.terminate();
    }
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
            PluginOutput {
                function: "nanoemApplicationPluginModelIOSetFunction".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOSetAllSelectedMaterialObjectIndices"
                    .to_owned(),
                arguments: Some(hashmap! {
                    "data".to_owned() => json!(data),
                    "length".to_owned() => json!(data.len()),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIODestroy".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOTerminate".to_owned(),
                ..Default::default()
            },
        ],
        read_plugin_output(stdout_reader)?
    );
    Ok(())
}

#[test]
fn set_all_selected_bone_indices() -> Result<()> {
    let (stdout_writer, stdout_reader) = Pipe::channel();
    let data = &[1, 4, 9, 16, 25, i32::MAX];
    {
        let mut controller = create_controller(stdout_writer)?;
        controller.initialize()?;
        controller.create()?;
        controller.set_function(0)?;
        assert_matches!(controller.set_all_selected_bone_indices(data), Ok(_));
        controller.destroy();
        controller.terminate();
    }
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
            PluginOutput {
                function: "nanoemApplicationPluginModelIOSetFunction".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOSetAllSelectedBoneObjectIndices"
                    .to_owned(),
                arguments: Some(hashmap! {
                    "data".to_owned() => json!(data),
                    "length".to_owned() => json!(data.len()),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIODestroy".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOTerminate".to_owned(),
                ..Default::default()
            },
        ],
        read_plugin_output(stdout_reader)?
    );
    Ok(())
}

#[test]
fn set_all_selected_morph_indices() -> Result<()> {
    let (stdout_writer, stdout_reader) = Pipe::channel();
    let data = &[1, 4, 9, 16, 25, i32::MAX];
    {
        let mut controller = create_controller(stdout_writer)?;
        controller.initialize()?;
        controller.create()?;
        controller.set_function(0)?;
        assert_matches!(controller.set_all_selected_morph_indices(data), Ok(_));
        controller.destroy();
        controller.terminate();
    }
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
            PluginOutput {
                function: "nanoemApplicationPluginModelIOSetFunction".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOSetAllSelectedMorphObjectIndices"
                    .to_owned(),
                arguments: Some(hashmap! {
                    "data".to_owned() => json!(data),
                    "length".to_owned() => json!(data.len()),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIODestroy".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOTerminate".to_owned(),
                ..Default::default()
            },
        ],
        read_plugin_output(stdout_reader)?
    );
    Ok(())
}

#[test]
fn set_all_selected_label_indices() -> Result<()> {
    let (stdout_writer, stdout_reader) = Pipe::channel();
    let data = &[1, 4, 9, 16, 25, i32::MAX];
    {
        let mut controller = create_controller(stdout_writer)?;
        controller.initialize()?;
        controller.create()?;
        controller.set_function(0)?;
        assert_matches!(controller.set_all_selected_label_indices(data), Ok(_));
        controller.destroy();
        controller.terminate();
    }
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
            PluginOutput {
                function: "nanoemApplicationPluginModelIOSetFunction".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOSetAllSelectedLabelObjectIndices"
                    .to_owned(),
                arguments: Some(hashmap! {
                    "data".to_owned() => json!(data),
                    "length".to_owned() => json!(data.len()),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIODestroy".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOTerminate".to_owned(),
                ..Default::default()
            },
        ],
        read_plugin_output(stdout_reader)?
    );
    Ok(())
}

#[test]
fn set_all_selected_rigid_body_indices() -> Result<()> {
    let (stdout_writer, stdout_reader) = Pipe::channel();
    let data = &[1, 4, 9, 16, 25, i32::MAX];
    {
        let mut controller = create_controller(stdout_writer)?;
        controller.initialize()?;
        controller.create()?;
        controller.set_function(0)?;
        assert_matches!(controller.set_all_selected_rigid_body_indices(data), Ok(_));
        controller.destroy();
        controller.terminate();
    }
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
            PluginOutput {
                function: "nanoemApplicationPluginModelIOSetFunction".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOSetAllSelectedRigidBodyObjectIndices"
                    .to_owned(),
                arguments: Some(hashmap! {
                    "data".to_owned() => json!(data),
                    "length".to_owned() => json!(data.len()),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIODestroy".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOTerminate".to_owned(),
                ..Default::default()
            },
        ],
        read_plugin_output(stdout_reader)?
    );
    Ok(())
}

#[test]
fn set_all_selected_joint_indices() -> Result<()> {
    let (stdout_writer, stdout_reader) = Pipe::channel();
    let data = &[1, 4, 9, 16, 25, i32::MAX];
    {
        let mut controller = create_controller(stdout_writer)?;
        controller.initialize()?;
        controller.create()?;
        controller.set_function(0)?;
        assert_matches!(controller.set_all_selected_joint_indices(data), Ok(_));
        controller.destroy();
        controller.terminate();
    }
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
            PluginOutput {
                function: "nanoemApplicationPluginModelIOSetFunction".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOSetAllSelectedJointObjectIndices"
                    .to_owned(),
                arguments: Some(hashmap! {
                    "data".to_owned() => json!(data),
                    "length".to_owned() => json!(data.len()),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIODestroy".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOTerminate".to_owned(),
                ..Default::default()
            },
        ],
        read_plugin_output(stdout_reader)?
    );
    Ok(())
}

#[test]
fn set_all_selected_soft_body_indices() -> Result<()> {
    let (stdout_writer, stdout_reader) = Pipe::channel();
    let data = &[1, 4, 9, 16, 25, i32::MAX];
    {
        let mut controller = create_controller(stdout_writer)?;
        controller.initialize()?;
        controller.create()?;
        controller.set_function(0)?;
        assert_matches!(controller.set_all_selected_soft_body_indices(data), Ok(_));
        controller.destroy();
        controller.terminate();
    }
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
            PluginOutput {
                function: "nanoemApplicationPluginModelIOSetFunction".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOSetAllSelectedSoftBodyObjectIndices"
                    .to_owned(),
                arguments: Some(hashmap! {
                    "data".to_owned() => json!(data),
                    "length".to_owned() => json!(data.len()),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIODestroy".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOTerminate".to_owned(),
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
                function: "nanoemApplicationPluginModelIOInitialize".to_owned(),
                ..Default::default()
            },
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
            PluginOutput {
                function: "nanoemApplicationPluginModelIOSetFunction".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOSetAudioDescription".to_owned(),
                arguments: Some(hashmap! {
                    "data".to_owned() => json!(data),
                    "length".to_owned() => json!(data.len()),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIODestroy".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOTerminate".to_owned(),
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
                function: "nanoemApplicationPluginModelIOInitialize".to_owned(),
                ..Default::default()
            },
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
            PluginOutput {
                function: "nanoemApplicationPluginModelIOSetFunction".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOSetInputAudioData".to_owned(),
                arguments: Some(hashmap! {
                    "data".to_owned() => json!(data),
                    "length".to_owned() => json!(data.len()),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIODestroy".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOTerminate".to_owned(),
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
                function: "nanoemApplicationPluginModelIOInitialize".to_owned(),
                ..Default::default()
            },
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
            PluginOutput {
                function: "nanoemApplicationPluginModelIOSetFunction".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOSetCameraDescription".to_owned(),
                arguments: Some(hashmap! {
                    "data".to_owned() => json!(data),
                    "length".to_owned() => json!(data.len()),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIODestroy".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOTerminate".to_owned(),
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
                function: "nanoemApplicationPluginModelIOInitialize".to_owned(),
                ..Default::default()
            },
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
            PluginOutput {
                function: "nanoemApplicationPluginModelIOSetFunction".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOSetLightDescription".to_owned(),
                arguments: Some(hashmap! {
                    "data".to_owned() => json!(data),
                    "length".to_owned() => json!(data.len()),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIODestroy".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOTerminate".to_owned(),
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
        let output_ui_layout_data = output?;
        assert!(output_ui_layout_data.is_empty());
        let mut reload: bool = false;
        assert_matches!(
            controller.set_ui_component_layout("ui_window", &data, &mut reload),
            Ok(_)
        );
        controller.destroy();
        controller.terminate();
        output_ui_layout_data
    };
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
            PluginOutput {
                function: "nanoemApplicationPluginModelIOSetFunction".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(0),
                    "status".to_owned() => json!(0),
                })
            },
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
            PluginOutput {
                function: "nanoemApplicationPluginModelIODestroy".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOTerminate".to_owned(),
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
                function: "nanoemApplicationPluginModelIOInitialize".to_owned(),
                ..Default::default()
            },
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
            PluginOutput {
                function: "nanoemApplicationPluginModelIOSetFunction".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(1),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOExecute".to_owned(),
                arguments: Some(hashmap! {
                    "status".to_owned() => json!(-1),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOGetFailureReason".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOGetRecoverySuggestion".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIODestroy".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOTerminate".to_owned(),
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
                function: "nanoemApplicationPluginModelIOInitialize".to_owned(),
                ..Default::default()
            },
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
            PluginOutput {
                function: "nanoemApplicationPluginModelIOSetFunction".to_owned(),
                arguments: Some(hashmap! {
                    "index".to_owned() => json!(1),
                    "status".to_owned() => json!(0),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOExecute".to_owned(),
                arguments: Some(hashmap! {
                    "status".to_owned() => json!(-1),
                })
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOGetFailureReason".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOGetRecoverySuggestion".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIODestroy".to_owned(),
                ..Default::default()
            },
            PluginOutput {
                function: "nanoemApplicationPluginModelIOTerminate".to_owned(),
                ..Default::default()
            },
        ],
        read_plugin_output(stdout_reader)?
    );
    Ok(())
}
