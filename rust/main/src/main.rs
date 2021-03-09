extern crate nanoem;

use nanoem::*;

use std::fs::File;
use std::io::prelude::Read;

fn dump_model(factory: UnicodeStringFactory) -> nanoem::Result<()> {
    let mut file = File::open("test.pmx").unwrap();
    let mut data = Vec::new();
    file.read_to_end(&mut data).unwrap();
    let model = Model::with_bytes(factory, data.as_slice())?;
    println!("model = {:?}", model);
    /*
    println!("bones = {:?}", model.all_bones());
    println!("materials = {:?}", model.all_materials());
    println!("morphs = {:?}", model.all_morphs());
    println!("labels = {:?}", model.all_labels());
    println!("rigid_bodies = {:?}", model.all_rigid_bodies());
    println!("joints = {:?}", model.all_joints());
    */
    let mutable_model = MutableModel::from_model(&model)?;
    let mut new_data = vec![];
    mutable_model.save(&mut new_data)?;
    println!("data -> {}", new_data.len());
    // let mutable_model = MutableModel::new(factory)?;
    mutable_model.set_name("test", Language::Japanese)?;
    mutable_model.set_comment("test", Language::Japanese)?;
    mutable_model.set_codec_type(ModelCodecType::UTF8);
    mutable_model.set_format_type(ModelFormatType::PMX21);
    {
        let mutable_vertex = mutable_model.create_vertex()?;
        mutable_vertex.set_origin(Vector4::new());
        mutable_vertex.set_normal(Vector4::new());
        mutable_vertex.set_texcoord(Vector4::new());
        mutable_vertex.set_additinal_uv(Vector4::new(), 0);
        mutable_vertex.set_edge_size(0.0);
        mutable_vertex.set_vertex_type(VertexType::Bdef2);
        mutable_vertex.set_sdef_c(Vector4::new());
        mutable_vertex.set_sdef_r0(Vector4::new());
        mutable_vertex.set_sdef_r1(Vector4::new());
        {
            let bone = mutable_model.create_bone()?;
            bone.set_name("bdef2_0", Language::Japanese)?;
            let bone = mutable_model.find_bone(mutable_model.insert_bone(bone, -1)?)?;
            mutable_vertex.set_bone(Some(&bone), 0);
            mutable_vertex.set_bone_weight(0.42, 0);
            let bone = mutable_model.create_bone()?;
            bone.set_name("bdef2_1", Language::Japanese)?;
            let bone = mutable_model.find_bone(mutable_model.insert_bone(bone, -1)?)?;
            mutable_vertex.set_bone(Some(&bone), 1);
            mutable_vertex.set_bone_weight(0.58, 1);
        }
        println!("{:?}", mutable_vertex.as_origin());
        mutable_model.remove_vertex(
            mutable_model.find_vertex(mutable_model.insert_vertex(mutable_vertex, -1)?)?,
        )?;
    }
    {
        let mutable_bone = mutable_model.create_bone()?;
        mutable_bone.set_name("Japanese", Language::Japanese)?;
        mutable_bone.set_name("English", Language::English)?;
        {
            let bone = mutable_model.create_bone()?;
            bone.set_name("parent_bone", Language::Japanese)?;
            let bone = mutable_model.find_bone(mutable_model.insert_bone(bone, -1)?)?;
            mutable_bone.set_parent_bone(Some(&bone));
        }
        {
            let bone = mutable_model.create_bone()?;
            bone.set_name("effector_bone", Language::Japanese)?;
            let bone = mutable_model.find_bone(mutable_model.insert_bone(bone, -1)?)?;
            mutable_bone.set_effector_bone(Some(&bone));
        }
        {
            let bone = mutable_model.create_bone()?;
            bone.set_name("inherent_bone", Language::Japanese)?;
            let bone = mutable_model.find_bone(mutable_model.insert_bone(bone, -1)?)?;
            mutable_bone.set_inherent_parent_bone(Some(&bone));
        }
        {
            let bone = mutable_model.create_bone()?;
            bone.set_name("target_bone", Language::Japanese)?;
            let bone = mutable_model.find_bone(mutable_model.insert_bone(bone, -1)?)?;
            mutable_bone.set_target_bone(Some(&bone));
        }
        {
            let mutable_constraint = mutable_model.create_constraint()?;
            mutable_constraint.set_angle_limit(1.0);
            mutable_constraint.set_effector_bone(None);
            mutable_constraint.set_num_iterations(1);
            mutable_constraint.set_target_bone(None);
            {
                let joint = mutable_constraint.create_joint()?;
                let bone = mutable_model.create_bone()?;
                bone.set_name("constraint_joint", Language::Japanese)?;
                let bone = mutable_model.find_bone(mutable_model.insert_bone(bone, -1)?)?;
                joint.set_bone(Some(&bone));
                joint.set_upper_limit(Vector4::new());
                joint.set_lower_limit(Vector4::new());
                println!("{:?}", joint.as_origin());
                mutable_constraint.insert_joint(joint, -1)?;
                mutable_constraint.remove_joint(mutable_constraint.find_joint(0)?)?;
            }
            println!("{:?}", mutable_constraint.as_origin());
            mutable_bone.set_constraint(Some(&mutable_constraint));
        }
        {
            let mutable_constraint = mutable_model.create_constraint()?;
            mutable_model.remove_constraint(
                mutable_model
                    .find_constraint(mutable_model.insert_constraint(mutable_constraint, -1)?)?,
            )?;
        }
        mutable_bone.set_origin(Vector4::new());
        mutable_bone.set_destination_origin(Vector4::new());
        mutable_bone.set_fixed_axis(Vector4::new());
        mutable_bone.set_local_x_axis(Vector4::new());
        mutable_bone.set_local_z_axis(Vector4::new());
        mutable_bone.set_inherent_coefficient(0.0);
        mutable_bone.set_stage_index(0);
        mutable_bone.set_rotateable(true);
        mutable_bone.set_movable(true);
        mutable_bone.set_visible(true);
        mutable_bone.set_user_handleable(true);
        mutable_bone.set_constraint_enabled(true);
        mutable_bone.set_fixed_axis_enabled(true);
        mutable_bone.set_local_axes_enabled(true);
        mutable_bone.set_local_inherent_enabled(true);
        mutable_bone.set_inherent_translation_enabled(true);
        mutable_bone.set_inherent_orientation_enabled(true);
        mutable_bone.set_affected_by_physics_simulation(true);
        mutable_bone.enable_external_parent_bone(0);
        println!("{:?}", mutable_bone.as_origin());
        mutable_model
            .remove_bone(mutable_model.find_bone(mutable_model.insert_bone(mutable_bone, 0)?)?)?;
    }
    {
        let mutable_material = mutable_model.create_material()?;
        mutable_material.set_name("Japanese", Language::Japanese)?;
        mutable_material.set_name("English", Language::English)?;
        {
            let texture = mutable_model.create_texture()?;
            texture.set_path("diffuse.bmp")?;
            let texture = mutable_model.find_texture(mutable_model.insert_texture(texture, -1)?)?;
            mutable_material.set_diffuse_texture(Some(&texture))?;
        }
        {
            let texture = mutable_model.create_texture()?;
            texture.set_path("sphere.bmp")?;
            let texture = mutable_model.find_texture(mutable_model.insert_texture(texture, -1)?)?;
            mutable_material.set_sphere_map_texture(Some(&texture))?;
        }
        {
            let texture = mutable_model.create_texture()?;
            texture.set_path("toon.bmp")?;
            let texture = mutable_model.find_texture(mutable_model.insert_texture(texture, -1)?)?;
            mutable_material.set_toon_texture(Some(&texture))?;
        }
        mutable_material.set_sphere_map_texture_type(MaterialSphereMapTextureType::Add);
        mutable_material.set_clob("clob")?;
        mutable_material.set_ambient_color(Vector4::new());
        mutable_material.set_diffuse_color(Vector4::new());
        mutable_material.set_specular_color(Vector4::new());
        mutable_material.set_edge_color(Vector4::new());
        mutable_material.set_diffuse_opacity(1.0);
        mutable_material.set_specular_power(1.0);
        mutable_material.set_edge_opacity(1.0);
        mutable_material.set_edge_size(1.0);
        mutable_material.set_num_vertex_indices(0);
        mutable_material.set_toon_texture_index(0);
        mutable_material.set_toon_shared(true);
        mutable_material.set_culling_disabled(true);
        mutable_material.set_casting_shadow_enabled(true);
        mutable_material.set_casting_shadow_map_enabled(true);
        mutable_material.set_shadow_map_enabled(true);
        mutable_material.set_edge_enabled(true);
        mutable_material.set_vertex_color_enabled(true);
        mutable_material.set_point_draw_enabled(true);
        mutable_material.set_line_draw_enabled(true);
        println!("{:?}", mutable_material.as_origin());
        mutable_model.remove_material(
            mutable_model.find_material(mutable_model.insert_material(mutable_material, 0)?)?,
        )?;
        {
            let mutable_texture = mutable_model.create_texture()?;
            mutable_model.remove_texture(
                mutable_model.find_texture(mutable_model.insert_texture(mutable_texture, -1)?)?,
            )?;
        }
    }
    {
        let mutable_morph = mutable_model.create_morph()?;
        mutable_morph.set_name("Japanese", Language::Japanese)?;
        mutable_morph.set_name("English", Language::English)?;
        mutable_morph.set_category(MorphCategory::Other);
        mutable_morph.set_morph_type(MorphType::Bone);
        println!("{:?}", mutable_morph.as_origin());
        mutable_model.remove_morph(
            mutable_model.find_morph(mutable_model.insert_morph(mutable_morph, 0)?)?,
        )?;
        {
            let mutable_morph = mutable_model.create_morph()?;
            let morph = mutable_morph.create_bone_morph()?;
            {
                let item = mutable_model.create_bone()?;
                item.set_name("morph_bone", Language::Japanese)?;
                let item = mutable_model.find_bone(mutable_model.insert_bone(item, -1)?)?;
                morph.set_bone(Some(&item));
            }
            morph.set_translation(Vector4::new());
            morph.set_orientation(Vector4::new());
            println!("{:?}", morph.as_origin());
            mutable_morph.set_morph_type(MorphType::Bone);
            mutable_morph.insert_bone_morph(morph, -1)?;
            mutable_morph.remove_bone_morph(mutable_morph.find_bone_morph(0)?)?;
        }
        {
            let mutable_morph = mutable_model.create_morph()?;
            let morph = mutable_morph.create_flip_morph()?;
            {
                let item = mutable_model.create_morph()?;
                item.set_name("morph_flip", Language::Japanese)?;
                let item = mutable_model.find_morph(mutable_model.insert_morph(item, -1)?)?;
                morph.set_morph(Some(&item));
            }
            morph.set_weight(0.42);
            println!("{:?}", morph.as_origin());
            mutable_morph.set_morph_type(MorphType::Flip);
            mutable_morph.insert_flip_morph(morph, -1)?;
            mutable_morph.remove_flip_morph(mutable_morph.find_flip_morph(0)?)?;
        }
        {
            let mutable_morph = mutable_model.create_morph()?;
            let morph = mutable_morph.create_group_morph()?;
            {
                let item = mutable_model.create_morph()?;
                item.set_name("morph_group", Language::Japanese)?;
                let item = mutable_model.find_morph(mutable_model.insert_morph(item, -1)?)?;
                morph.set_morph(Some(&item));
            }
            morph.set_weight(0.42);
            println!("{:?}", morph.as_origin());
            mutable_morph.set_morph_type(MorphType::Group);
            mutable_morph.insert_group_morph(morph, -1)?;
            mutable_morph.remove_group_morph(mutable_morph.find_group_morph(0)?)?;
        }
        {
            let mutable_morph = mutable_model.create_morph()?;
            let morph = mutable_morph.create_impulse_morph()?;
            {
                let item = mutable_model.create_rigid_body()?;
                item.set_name("morph_rigid_body", Language::Japanese)?;
                let item =
                    mutable_model.find_rigid_body(mutable_model.insert_rigid_body(item, -1)?)?;
                morph.set_rigid_body(Some(&item));
            }
            morph.set_torque(Vector4::new());
            morph.set_velocity(Vector4::new());
            morph.set_local(true);
            println!("{:?}", morph.as_origin());
            mutable_morph.set_morph_type(MorphType::Impulse);
            mutable_morph.insert_impulse_morph(morph, -1)?;
            mutable_morph.remove_impulse_morph(mutable_morph.find_impulse_morph(0)?)?;
        }
        {
            let mutable_morph = mutable_model.create_morph()?;
            let morph = mutable_morph.create_material_morph()?;
            {
                let item = mutable_model.create_material()?;
                item.set_name("morph_material", Language::Japanese)?;
                let item = mutable_model.find_material(mutable_model.insert_material(item, -1)?)?;
                morph.set_material(Some(&item));
            }
            morph.set_diffuse_texture_blend(Vector4::new());
            morph.set_sphere_map_texture_blend(Vector4::new());
            morph.set_toon_texture_blend(Vector4::new());
            morph.set_ambient_color(Vector4::new());
            morph.set_diffuse_color(Vector4::new());
            morph.set_diffuse_opacity(1.0);
            morph.set_specular_color(Vector4::new());
            morph.set_specular_power(1.0);
            morph.set_edge_color(Vector4::new());
            morph.set_edge_opacity(1.0);
            morph.set_edge_size(1.0);
            morph.set_operation_type(MorphMaterialOperationType::Multiply);
            println!("{:?}", morph.as_origin());
            mutable_morph.set_morph_type(MorphType::Material);
            mutable_morph.insert_material_morph(morph, -1)?;
            mutable_morph.remove_material_morph(mutable_morph.find_material_morph(0)?)?;
        }
        {
            let mutable_morph = mutable_model.create_morph()?;
            let morph = mutable_morph.create_uv_morph()?;
            {
                let item = mutable_model.create_vertex()?;
                let item = mutable_model.find_vertex(mutable_model.insert_vertex(item, -1)?)?;
                morph.set_vertex(Some(&item));
            }
            morph.set_position(Vector4::new());
            println!("{:?}", morph.as_origin());
            mutable_morph.set_morph_type(MorphType::UVA1);
            mutable_morph.insert_uv_morph(morph, -1)?;
            mutable_morph.remove_uv_morph(mutable_morph.find_uv_morph(0)?)?;
        }
        {
            let mutable_morph = mutable_model.create_morph()?;
            let morph = mutable_morph.create_vertex_morph()?;
            {
                let item = mutable_model.create_vertex()?;
                let item = mutable_model.find_vertex(mutable_model.insert_vertex(item, -1)?)?;
                morph.set_vertex(Some(&item));
            }
            morph.set_position(Vector4::new());
            println!("{:?}", morph.as_origin());
            mutable_morph.set_morph_type(MorphType::Vertex);
            mutable_morph.insert_vertex_morph(morph, -1)?;
            mutable_morph.remove_vertex_morph(mutable_morph.find_vertex_morph(0)?)?;
        }
    }
    {
        let mutable_label = mutable_model.create_label()?;
        mutable_label.set_name("Japanese", Language::Japanese)?;
        mutable_label.set_name("English", Language::English)?;
        mutable_label.set_special(true);
        {
            let bone = mutable_model.create_bone()?;
            bone.set_name("label_bone", Language::Japanese)?;
            let item = mutable_label.create_item_from_bone(
                mutable_model.find_bone(mutable_model.insert_bone(bone, -1)?)?,
            )?;
            println!("{:?}", item.as_origin());
            mutable_label.insert_item(item, 0)?;
            mutable_label.remove_item(mutable_label.find_item(0)?)?;
            let morph = mutable_model.create_morph()?;
            morph.set_name("label_morph", Language::Japanese)?;
            let item = mutable_label.create_item_from_morph(
                mutable_model.find_morph(mutable_model.insert_morph(morph, -1)?)?,
            )?;
            println!("{:?}", item.as_origin());
            mutable_label.insert_item(item, 0)?;
            mutable_label.remove_item(mutable_label.find_item(0)?)?;
        }
        println!("{:?}", mutable_label.as_origin());
        mutable_model.remove_label(
            mutable_model.find_label(mutable_model.insert_label(mutable_label, 0)?)?,
        )?;
    }
    {
        let mutable_rigid_body = mutable_model.create_rigid_body()?;
        mutable_rigid_body.set_name("Japanese", Language::Japanese)?;
        mutable_rigid_body.set_name("English", Language::English)?;
        {
            let bone = MutableBone::new(&mutable_model)?;
            bone.set_name("rigid_body_bone", Language::Japanese)?;
            let bone = mutable_model.find_bone(mutable_model.insert_bone(bone, -1)?)?;
            mutable_rigid_body.set_bone(Some(&bone));
        }
        mutable_rigid_body.set_origin(Vector4::new());
        mutable_rigid_body.set_orientation(Vector4::new());
        mutable_rigid_body.set_shape_size(Vector4::new());
        mutable_rigid_body.set_shape_type(RigidBodyShapeType::Box);
        mutable_rigid_body.set_transform_type(RigidBodyTransformType::FromSimulationToBone);
        mutable_rigid_body.set_mass(0.0);
        mutable_rigid_body.set_angular_damping(0.0);
        mutable_rigid_body.set_linear_damping(0.0);
        mutable_rigid_body.set_friction(0.0);
        mutable_rigid_body.set_restitution(0.0);
        mutable_rigid_body.set_collision_group_id(0);
        mutable_rigid_body.set_collision_mask(0);
        println!("{:?}", mutable_rigid_body.as_origin());
        mutable_model.remove_rigid_body(
            mutable_model
                .find_rigid_body(mutable_model.insert_rigid_body(mutable_rigid_body, 0)?)?,
        )?;
    }
    {
        let mutable_joint = mutable_model.create_joint()?;
        mutable_joint.set_name("Japanese", Language::Japanese)?;
        mutable_joint.set_name("English", Language::English)?;
        {
            let rigid_body = mutable_model.create_rigid_body()?;
            rigid_body.set_name("rigid_body_a", Language::Japanese)?;
            let rigid_body =
                mutable_model.find_rigid_body(mutable_model.insert_rigid_body(rigid_body, -1)?)?;
            mutable_joint.set_rigid_body_a(Some(&rigid_body));
        }
        {
            let rigid_body = mutable_model.create_rigid_body()?;
            rigid_body.set_name("rigid_body_b", Language::Japanese)?;
            let rigid_body =
                mutable_model.find_rigid_body(mutable_model.insert_rigid_body(rigid_body, -1)?)?;
            mutable_joint.set_rigid_body_b(Some(&rigid_body));
        }
        mutable_joint.set_joint_type(JointType::Generic6DofSpring);
        mutable_joint.set_origin(Vector4::new());
        mutable_joint.set_orientation(Vector4::new());
        mutable_joint.set_angular_lower_limit(Vector4::new());
        mutable_joint.set_angular_stiffness(Vector4::new());
        mutable_joint.set_angular_upper_limit(Vector4::new());
        mutable_joint.set_linear_lower_limit(Vector4::new());
        mutable_joint.set_linear_stiffness(Vector4::new());
        mutable_joint.set_linear_upper_limit(Vector4::new());
        println!("{:?}", mutable_joint.as_origin());
        mutable_model.remove_joint(
            mutable_model.find_joint(mutable_model.insert_joint(mutable_joint, 0)?)?,
        )?;
    }
    Ok(())
}

fn dump_motion(factory: UnicodeStringFactory) -> nanoem::Result<()> {
    let mut file = File::open("test.vmd").unwrap();
    let mut data = Vec::new();
    file.read_to_end(&mut data).unwrap();
    let motion = Motion::with_bytes(factory, data.as_slice(), 0, MotionFormatType::Unknown)?;
    println!("motion = {:?}", motion);
    println!(
        "all_accessory_keyframes = {:?}",
        motion.all_accessory_keyframes()
    );
    motion.find_accessory_keyframe(0);
    motion.find_bone_keyframe("", 0)?;
    motion.find_camera_keyframe(0);
    motion.find_light_keyframe(0);
    motion.find_model_keyframe(0);
    motion.find_morph_keyframe("", 0)?;
    motion.find_self_shadow_keyframe(0);
    /*
    println!("all_bone_keyframes = {:?}", motion.all_bone_keyframes());
    println!("all_camera_keyframes = {:?}", motion.all_camera_keyframes());
    println!("all_light_keyframes = {:?}", motion.all_light_keyframes());
    println!("all_model_keyframes = {:?}", motion.all_model_keyframes());
    println!("all_morph_keyframes = {:?}", motion.all_morph_keyframes());
    println!("all_self_shadow_keyframes = {:?}", motion.all_self_shadow_keyframes());
    */
    let mut new_data = vec![];
    let mutable_motion = MutableMotion::from_motion(&motion)?;
    mutable_motion.save(&mut new_data, MotionFormatType::VMD)?;
    println!("data -> {}", new_data.len());
    {
        let keyframe = mutable_motion.create_accessory_keyframe()?;
        {
            let outside_parent = keyframe.create_outside_parent()?;
            outside_parent.set_subject_bone_name("subject")?;
            outside_parent.set_target_object_name("object")?;
            outside_parent.set_target_bone_name("bone")?;
            keyframe.set_outside_parent(Some(outside_parent))?;
        }
        {
            let parameter = keyframe.create_effect_parameter()?;
            parameter.set_name("test")?;
            parameter.set_value(EffectParameterVariant::Bool(false));
            println!("{:?}", parameter.as_origin());
            keyframe.add_effect_parameter(parameter)?;
            keyframe.remove_effect_parameter(keyframe.find_effect_parameter(0)?)?;
        }
        keyframe.set_translation(Vector4::new());
        keyframe.set_orientation(Vector4::new());
        keyframe.set_opacity(1.0);
        keyframe.set_scale_factor(1.0);
        keyframe.set_add_blend_enabled(true);
        keyframe.set_shadow_enabled(true);
        keyframe.set_visible(true);
        mutable_motion.add_accessory_keyframe(keyframe, 42)?;
        mutable_motion.find_accessory_keyframe(42)?.map(|keyframe| {
            println!("{:?}", keyframe.as_origin());
            mutable_motion.remove_accessory_keyframe(keyframe)
        });
    }
    {
        let keyframe = mutable_motion.create_bone_keyframe()?;
        keyframe.set_translation(Vector4::new());
        keyframe.set_orientation(Vector4::new());
        keyframe.set_interpolation(
            Interpolation::new(),
            BoneKeyframeInterpolationType::TranslationX,
        );
        keyframe.set_interpolation(
            Interpolation::new(),
            BoneKeyframeInterpolationType::TranslationY,
        );
        keyframe.set_interpolation(
            Interpolation::new(),
            BoneKeyframeInterpolationType::TranslationZ,
        );
        keyframe.set_interpolation(
            Interpolation::new(),
            BoneKeyframeInterpolationType::Orientation,
        );
        keyframe.set_stage_index(1);
        keyframe.set_physics_simulation_enabled(true);
        mutable_motion.add_bone_keyframe(keyframe, "test", 42)?;
        mutable_motion
            .find_bone_keyframe("test", 42)?
            .map(|keyframe| {
                println!("{:?}", keyframe.as_origin());
                mutable_motion.remove_bone_keyframe(keyframe)
            });
    }
    {
        let keyframe = mutable_motion.create_camera_keyframe()?;
        let outside_parent = keyframe.create_outside_parent()?;
        outside_parent.set_subject_bone_name("subject")?;
        outside_parent.set_target_object_name("object")?;
        outside_parent.set_target_bone_name("bone")?;
        keyframe.set_outside_parent(Some(outside_parent))?;
        keyframe.set_look_at(Vector4::new());
        keyframe.set_angle(Vector4::new());
        keyframe.set_distance(100.0);
        keyframe.set_fov(30);
        keyframe.set_interpolation(
            Interpolation::new(),
            CameraKeyframeInterpolationType::LookAtX,
        );
        keyframe.set_interpolation(
            Interpolation::new(),
            CameraKeyframeInterpolationType::LookAtY,
        );
        keyframe.set_interpolation(
            Interpolation::new(),
            CameraKeyframeInterpolationType::LookAtZ,
        );
        keyframe.set_interpolation(Interpolation::new(), CameraKeyframeInterpolationType::Angle);
        keyframe.set_interpolation(
            Interpolation::new(),
            CameraKeyframeInterpolationType::Distance,
        );
        keyframe.set_interpolation(Interpolation::new(), CameraKeyframeInterpolationType::Fov);
        keyframe.set_stage_index(1);
        keyframe.set_perspective(true);
        mutable_motion.add_camera_keyframe(keyframe, 42)?;
        mutable_motion.find_camera_keyframe(42)?.map(|keyframe| {
            println!("{:?}", keyframe.as_origin());
            mutable_motion.remove_camera_keyframe(keyframe)
        });
    }
    {
        let keyframe = mutable_motion.create_light_keyframe()?;
        keyframe.set_color(Vector4::new());
        keyframe.set_direction(Vector4::new());
        mutable_motion.add_light_keyframe(keyframe, 42)?;
        mutable_motion.find_light_keyframe(42)?.map(|keyframe| {
            println!("{:?}", keyframe.as_origin());
            mutable_motion.remove_light_keyframe(keyframe)
        });
    }
    {
        let keyframe = mutable_motion.create_model_keyframe()?;
        keyframe.set_edge_color(Vector4::new());
        keyframe.set_edge_scale_factor(1.0);
        keyframe.set_add_blend_enabled(true);
        keyframe.set_physics_simulation_enabled(true);
        keyframe.set_visible(true);
        {
            let state = keyframe.create_constraint_state()?;
            state.set_name("test")?;
            state.set_enabled(true);
            println!("{:?}", state.as_origin());
            keyframe.add_constraint_state(state)?;
            keyframe.remove_constraint_state(keyframe.find_constraint_state(0)?)?;
        }
        {
            let parameter = keyframe.create_effect_parameter()?;
            parameter.set_name("test")?;
            parameter.set_value(EffectParameterVariant::Int32(42));
            println!("{:?}", parameter.as_origin());
            keyframe.add_effect_parameter(parameter)?;
            keyframe.remove_effect_parameter(keyframe.find_effect_parameter(0)?)?;
        }
        {
            let outside_parent = keyframe.create_outside_parent()?;
            outside_parent.set_subject_bone_name("subject")?;
            outside_parent.set_target_object_name("object")?;
            outside_parent.set_target_bone_name("bone")?;
            println!("{:?}", outside_parent.as_origin());
            keyframe.add_outside_parent(outside_parent)?;
            keyframe.remove_outside_parent(keyframe.find_outside_parent(0)?)?;
        }
        mutable_motion.add_model_keyframe(keyframe, 42)?;
        mutable_motion.find_model_keyframe(42)?.map(|keyframe| {
            println!("{:?}", keyframe.as_origin());
            mutable_motion.remove_model_keyframe(keyframe)
        });
    }
    {
        let keyframe = mutable_motion.create_morph_keyframe()?;
        keyframe.set_weight(1.0);
        mutable_motion.add_morph_keyframe(keyframe, "morph", 42)?;
        mutable_motion
            .find_morph_keyframe("morph", 42)?
            .map(|keyframe| {
                println!("{:?}", keyframe.as_origin());
                mutable_motion.remove_morph_keyframe(keyframe)
            });
    }
    {
        let keyframe = mutable_motion.create_self_shadow_keyframe()?;
        keyframe.set_distance(1000.0);
        keyframe.set_mode(1);
        mutable_motion.add_self_shadow_keyframe(keyframe, 42)?;
        mutable_motion
            .find_self_shadow_keyframe(42)?
            .map(|keyframe| {
                println!("{:?}", keyframe.as_origin());
                mutable_motion.remove_self_shadow_keyframe(keyframe)
            });
    }
    mutable_motion.set_target_model_name("test")?;
    mutable_motion.sort_all_keyframes();
    Ok(())
}

fn main() -> nanoem::Result<()> {
    dump_model(UnicodeStringFactory::create()?)?;
    dump_motion(UnicodeStringFactory::create()?)?;
    Ok(())
}
