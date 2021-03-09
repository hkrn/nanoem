/*
  Copyright (c) 2015-2020 hkrn All rights reserved

  This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
*/

use super::common::*;
use super::native::*;

pub struct Model {
    pub(crate) unicode_string_factory: UnicodeStringFactory,
    pub(crate) opaque: *mut nanoem_model_t,
}

#[derive(Copy, Clone, Debug, PartialEq)]
pub enum ModelFormatType {
    Unknown,
    PMD10,
    PMX20,
    PMX21,
}

impl nanoem_model_format_type_t {
    pub fn cast(self) -> ModelFormatType {
        match self {
            nanoem_model_format_type_t::PMD_1_0 => ModelFormatType::PMD10,
            nanoem_model_format_type_t::PMX_2_0 => ModelFormatType::PMX20,
            nanoem_model_format_type_t::PMX_2_1 => ModelFormatType::PMX21,
            _ => ModelFormatType::Unknown,
        }
    }
}

impl ModelFormatType {
    pub(crate) fn cast(self) -> nanoem_model_format_type_t {
        match self {
            ModelFormatType::PMD10 => nanoem_model_format_type_t::PMD_1_0,
            ModelFormatType::PMX20 => nanoem_model_format_type_t::PMX_2_0,
            ModelFormatType::PMX21 => nanoem_model_format_type_t::PMX_2_1,
            _ => nanoem_model_format_type_t::UNKNOWN,
        }
    }
}

#[derive(Copy, Clone, Debug, PartialEq)]
pub enum ModelCodecType {
    Unknown,
    ShiftJIS,
    UTF8,
    UTF16,
}

impl Model {
    pub fn new(factory: UnicodeStringFactory) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe { nanoemModelCreate(factory.opaque, status_ptr) };
        status.value(Self {
            unicode_string_factory: factory,
            opaque,
        })
    }
    pub fn with_bytes(factory: UnicodeStringFactory, data: &[u8]) -> Result<Model> {
        let model = Model::new(factory)?;
        model.load(data)?;
        Ok(model)
    }
    pub fn load(&self, data: &[u8]) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let buffer = Buffer::from_slice(data)?;
        unsafe { nanoemModelLoadFromBuffer(self.opaque, buffer.opaque, status_ptr) };
        status.unit()
    }
    pub fn name(&self, language: Language) -> Result<String> {
        unsafe {
            self.unicode_string_factory
                .to_string(nanoemModelGetName(self.opaque, language.cast()))
        }
    }
    pub fn comment(&self, language: Language) -> Result<String> {
        unsafe {
            self.unicode_string_factory
                .to_string(nanoemModelGetComment(self.opaque, language.cast()))
        }
    }
    pub fn format_type(&self) -> ModelFormatType {
        unsafe { nanoemModelGetFormatType(self.opaque).cast() }
    }
    pub fn codec_type(&self) -> ModelCodecType {
        match unsafe { nanoemModelGetCodecType(self.opaque) } {
            nanoem_codec_type_t::SJIS => ModelCodecType::ShiftJIS,
            nanoem_codec_type_t::UTF8 => ModelCodecType::UTF8,
            nanoem_codec_type_t::UTF16 => ModelCodecType::UTF16,
            _ => ModelCodecType::Unknown,
        }
    }
    pub fn additiona_uv_size(&self) -> u32 {
        unsafe { nanoemModelGetAdditionalUVSize(self.opaque) }
    }
    pub fn all_vertices(&self) -> Vec<Vertex> {
        let mut num_objects = 0usize;
        let ptr =
            unsafe { nanoemModelGetAllVertexObjects(self.opaque, &mut num_objects as *mut usize) };
        let mut vertices: Vec<Vertex> = Vec::with_capacity(num_objects);
        let factory = &self.unicode_string_factory;
        for i in 0..num_objects {
            vertices.push(Vertex::new(factory, unsafe { *(ptr.add(i)) }));
        }
        vertices
    }
    pub fn all_vertex_indices(&self) -> Vec<u32> {
        let mut num_objects = 0usize;
        let ptr =
            unsafe { nanoemModelGetAllVertexIndices(self.opaque, &mut num_objects as *mut usize) };
        let mut indices: Vec<u32> = Vec::with_capacity(num_objects);
        unsafe {
            indices.set_len(num_objects);
            std::ptr::copy_nonoverlapping(ptr, indices.as_mut_ptr(), num_objects);
        }
        indices
    }
    pub fn all_textures(&self) -> Vec<Texture> {
        let mut num_objects = 0usize;
        let ptr =
            unsafe { nanoemModelGetAllTextureObjects(self.opaque, &mut num_objects as *mut usize) };
        let mut textures: Vec<Texture> = Vec::with_capacity(num_objects);
        let factory = &self.unicode_string_factory;
        for i in 0..num_objects {
            textures.push(Texture::new(factory, unsafe { *(ptr.add(i)) }));
        }
        textures
    }
    pub fn all_materials(&self) -> Vec<Material> {
        let mut num_objects = 0usize;
        let ptr = unsafe {
            nanoemModelGetAllMaterialObjects(self.opaque, &mut num_objects as *mut usize)
        };
        let mut materials: Vec<Material> = Vec::with_capacity(num_objects);
        let factory = &self.unicode_string_factory;
        for i in 0..num_objects {
            materials.push(Material::new(factory, unsafe { *(ptr.add(i)) }));
        }
        materials
    }
    pub fn all_constraints(&self) -> Vec<Constraint> {
        let mut num_objects = 0usize;
        let ptr = unsafe {
            nanoemModelGetAllConstraintObjects(self.opaque, &mut num_objects as *mut usize)
        };
        let mut constraints: Vec<Constraint> = Vec::with_capacity(num_objects);
        let factory = &self.unicode_string_factory;
        for i in 0..num_objects {
            constraints.push(Constraint::new(factory, unsafe { *(ptr.add(i)) }));
        }
        constraints
    }
    pub fn all_bones(&self) -> Vec<Bone> {
        let mut num_objects = 0usize;
        let ptr =
            unsafe { nanoemModelGetAllBoneObjects(self.opaque, &mut num_objects as *mut usize) };
        let mut bones: Vec<Bone> = Vec::with_capacity(num_objects);
        let factory = &self.unicode_string_factory;
        for i in 0..num_objects {
            bones.push(Bone::new(factory, unsafe { *(ptr.add(i)) }));
        }
        bones
    }
    #[allow(dead_code)]
    pub fn all_ordered_bones(&self) -> Vec<Bone> {
        let mut num_objects = 0usize;
        let ptr = unsafe {
            nanoemModelGetAllOrderedBoneObjects(self.opaque, &mut num_objects as *mut usize)
        };
        let mut bones: Vec<Bone> = Vec::with_capacity(num_objects);
        let factory = &self.unicode_string_factory;
        for i in 0..num_objects {
            bones.push(Bone::new(factory, unsafe { *(ptr.add(i)) }));
        }
        bones
    }
    pub fn all_morphs(&self) -> Vec<Morph> {
        let mut num_objects = 0usize;
        let ptr =
            unsafe { nanoemModelGetAllMorphObjects(self.opaque, &mut num_objects as *mut usize) };
        let mut morphs: Vec<Morph> = Vec::with_capacity(num_objects);
        let factory = &self.unicode_string_factory;
        for i in 0..num_objects {
            morphs.push(Morph::new(factory, unsafe { *(ptr.add(i)) }));
        }
        morphs
    }
    pub fn all_labels(&self) -> Vec<Label> {
        let mut num_objects = 0usize;
        let ptr =
            unsafe { nanoemModelGetAllLabelObjects(self.opaque, &mut num_objects as *mut usize) };
        let mut labels: Vec<Label> = Vec::with_capacity(num_objects);
        let factory = &self.unicode_string_factory;
        for i in 0..num_objects {
            labels.push(Label::new(factory, unsafe { *(ptr.add(i)) }));
        }
        labels
    }
    pub fn all_rigid_bodies(&self) -> Vec<RigidBody> {
        let mut num_objects = 0usize;
        let ptr = unsafe {
            nanoemModelGetAllRigidBodyObjects(self.opaque, &mut num_objects as *mut usize)
        };
        let mut bodies: Vec<RigidBody> = Vec::with_capacity(num_objects);
        let factory = &self.unicode_string_factory;
        for i in 0..num_objects {
            bodies.push(RigidBody::new(factory, unsafe { *(ptr.add(i)) }));
        }
        bodies
    }
    pub fn all_joints(&self) -> Vec<Joint> {
        let mut num_objects = 0usize;
        let ptr =
            unsafe { nanoemModelGetAllJointObjects(self.opaque, &mut num_objects as *mut usize) };
        let mut joints: Vec<Joint> = Vec::with_capacity(num_objects);
        let factory = &self.unicode_string_factory;
        for i in 0..num_objects {
            joints.push(Joint::new(factory, unsafe { *(ptr.add(i)) }));
        }
        joints
    }
}

impl std::fmt::Debug for Model {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            r#"Model {{
    name: {},
    comment: {},
    format_type: {:?},
    codec_type: {:?},
    additional_uv_size: {},
    num_vertices: {},
    num_vertex_indices: {},
    num_constriants: {},
    num_bones: {},
    num_textures {},
    num_materials: {},
    num_morphs: {},
    num_labels: {},
    num_rigid_bodies: {},
    num_joints: {}
}}"#,
            self.name(Language::Japanese).unwrap(),
            self.comment(Language::Japanese).unwrap(),
            self.format_type(),
            self.codec_type(),
            self.additiona_uv_size(),
            self.all_vertices().len(),
            self.all_vertex_indices().len(),
            self.all_constraints().len(),
            self.all_bones().len(),
            self.all_textures().len(),
            self.all_materials().len(),
            self.all_morphs().len(),
            self.all_labels().len(),
            self.all_rigid_bodies().len(),
            self.all_joints().len()
        )
    }
}

impl Drop for Model {
    fn drop(&mut self) {
        unsafe {
            nanoemModelDestroy(self.opaque);
        }
    }
}

#[derive(Copy, Clone, Debug, PartialEq)]
pub enum VertexType {
    Unknown,
    Bdef1,
    Bdef2,
    Bdef4,
    Sdef,
    Qdef,
}

impl nanoem_model_vertex_type_t {
    pub(crate) fn cast(self) -> VertexType {
        match self {
            nanoem_model_vertex_type_t::BDEF1 => VertexType::Bdef1,
            nanoem_model_vertex_type_t::BDEF2 => VertexType::Bdef2,
            nanoem_model_vertex_type_t::BDEF4 => VertexType::Bdef4,
            nanoem_model_vertex_type_t::SDEF => VertexType::Sdef,
            nanoem_model_vertex_type_t::QDEF => VertexType::Qdef,
            _ => VertexType::Unknown,
        }
    }
}

impl VertexType {
    pub(crate) fn cast(self) -> nanoem_model_vertex_type_t {
        match self {
            VertexType::Bdef1 => nanoem_model_vertex_type_t::BDEF1,
            VertexType::Bdef2 => nanoem_model_vertex_type_t::BDEF2,
            VertexType::Bdef4 => nanoem_model_vertex_type_t::BDEF4,
            VertexType::Sdef => nanoem_model_vertex_type_t::SDEF,
            VertexType::Qdef => nanoem_model_vertex_type_t::QDEF,
            VertexType::Unknown => nanoem_model_vertex_type_t::UNKNOWN,
        }
    }
}

pub struct Vertex<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *const nanoem_model_vertex_t,
}

impl<'a> Vertex<'a> {
    pub fn new(factory: &'a UnicodeStringFactory, opaque: *const nanoem_model_vertex_t) -> Self {
        Self {
            unicode_string_factory: factory,
            opaque,
        }
    }
    pub fn index(&self) -> i32 {
        unsafe { nanoemModelObjectGetIndex(nanoemModelVertexGetModelObject(self.opaque)) }
    }
    pub fn origin(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelVertexGetOrigin(self.opaque)) }
    }
    pub fn normal(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelVertexGetNormal(self.opaque)) }
    }
    pub fn texcoord(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelVertexGetTexCoord(self.opaque)) }
    }
    pub fn additinal_uv(&self, index: usize) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelVertexGetAdditionalUV(self.opaque, index)) }
    }
    pub fn edge_size(&self) -> f32 {
        unsafe { nanoemModelVertexGetEdgeSize(self.opaque) }
    }
    pub fn vertex_type(&self) -> VertexType {
        unsafe { nanoemModelVertexGetType(self.opaque).cast() }
    }
    pub fn bone(&self, value: usize) -> Bone {
        Bone::new(&self.unicode_string_factory, unsafe {
            nanoemModelVertexGetBoneObject(self.opaque, value)
        })
    }
    pub fn bone_weight(&self, value: usize) -> f32 {
        unsafe { nanoemModelVertexGetBoneWeight(self.opaque, value) }
    }
    pub fn sdef_c(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelVertexGetSdefC(self.opaque)) }
    }
    pub fn sdef_r0(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelVertexGetSdefR0(self.opaque)) }
    }
    pub fn sdef_r1(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelVertexGetSdefR1(self.opaque)) }
    }
}

impl<'a> std::fmt::Debug for Vertex<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            r#"Vertex {{
    index: {},
    origin: {:?},
    normal: {:?},
    texcoord: {:?},
    edge_size: {:?},
    uv1: {:?},
    uv2: {:?},
    uv3: {:?},
    uv4: {:?},
    type: {:?}
    bone: ({}, {}, {}, {})
    weight: ({}, {}, {}, {}),
    sdef_c: {:?},
    sdef_r0: {:?},
    sdef_r1: {:?},
}}"#,
            self.index(),
            self.origin(),
            self.normal(),
            self.texcoord(),
            self.edge_size(),
            self.additinal_uv(0),
            self.additinal_uv(1),
            self.additinal_uv(2),
            self.additinal_uv(3),
            self.vertex_type(),
            self.bone(0).name(Language::Japanese).unwrap(),
            self.bone(1).name(Language::Japanese).unwrap(),
            self.bone(2).name(Language::Japanese).unwrap(),
            self.bone(3).name(Language::Japanese).unwrap(),
            self.bone_weight(0),
            self.bone_weight(1),
            self.bone_weight(2),
            self.bone_weight(3),
            self.sdef_c(),
            self.sdef_r0(),
            self.sdef_r1(),
        )
    }
}

pub struct Texture<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *const nanoem_model_texture_t,
}

impl<'a> Texture<'a> {
    pub fn new(factory: &'a UnicodeStringFactory, opaque: *const nanoem_model_texture_t) -> Self {
        Self {
            unicode_string_factory: factory,
            opaque,
        }
    }
    pub fn index(&self) -> i32 {
        unsafe { nanoemModelObjectGetIndex(nanoemModelTextureGetModelObject(self.opaque)) }
    }
    pub fn path(&self) -> Result<String> {
        unsafe {
            self.unicode_string_factory
                .to_string(nanoemModelTextureGetPath(self.opaque))
        }
    }
}

impl<'a> std::fmt::Debug for Texture<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            "Texture {{ index: {}, path: {} }}",
            self.index(),
            self.path().unwrap()
        )
    }
}

#[derive(Copy, Clone, Debug, PartialEq)]
pub enum MaterialSphereMapTextureType {
    Unknown,
    None,
    Add,
    Multiply,
    SubTexture,
}

impl nanoem_model_material_sphere_map_texture_type_t {
    pub fn cast(self) -> MaterialSphereMapTextureType {
        match self {
            nanoem_model_material_sphere_map_texture_type_t::ADD => {
                MaterialSphereMapTextureType::Add
            }
            nanoem_model_material_sphere_map_texture_type_t::MULTIPLY => {
                MaterialSphereMapTextureType::Multiply
            }
            nanoem_model_material_sphere_map_texture_type_t::NONE => {
                MaterialSphereMapTextureType::None
            }
            nanoem_model_material_sphere_map_texture_type_t::SUB_TEXTURE => {
                MaterialSphereMapTextureType::SubTexture
            }
            _ => MaterialSphereMapTextureType::Unknown,
        }
    }
}

impl MaterialSphereMapTextureType {
    pub(crate) fn cast(self) -> nanoem_model_material_sphere_map_texture_type_t {
        match self {
            MaterialSphereMapTextureType::Add => {
                nanoem_model_material_sphere_map_texture_type_t::ADD
            }
            MaterialSphereMapTextureType::Multiply => {
                nanoem_model_material_sphere_map_texture_type_t::MULTIPLY
            }
            MaterialSphereMapTextureType::None => {
                nanoem_model_material_sphere_map_texture_type_t::NONE
            }
            MaterialSphereMapTextureType::SubTexture => {
                nanoem_model_material_sphere_map_texture_type_t::SUB_TEXTURE
            }
            _ => panic!("unknown type"),
        }
    }
}

pub struct Material<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *const nanoem_model_material_t,
}

impl<'a> Material<'a> {
    pub fn new(factory: &'a UnicodeStringFactory, opaque: *const nanoem_model_material_t) -> Self {
        Self {
            unicode_string_factory: factory,
            opaque,
        }
    }
    pub fn index(&self) -> i32 {
        unsafe { nanoemModelObjectGetIndex(nanoemModelMaterialGetModelObject(self.opaque)) }
    }
    pub fn name(&self, language: Language) -> Result<String> {
        unsafe {
            self.unicode_string_factory
                .to_string(nanoemModelMaterialGetName(self.opaque, language.cast()))
        }
    }
    pub fn diffuse_texture(&self) -> Texture {
        Texture::new(&self.unicode_string_factory, unsafe {
            nanoemModelMaterialGetDiffuseTextureObject(self.opaque)
        })
    }
    pub fn sphere_map_texture(&self) -> Texture {
        Texture::new(&self.unicode_string_factory, unsafe {
            nanoemModelMaterialGetSphereMapTextureObject(self.opaque)
        })
    }
    pub fn toon_texture(&self) -> Texture {
        Texture::new(&self.unicode_string_factory, unsafe {
            nanoemModelMaterialGetToonTextureObject(self.opaque)
        })
    }
    pub fn clob(&self) -> Result<String> {
        unsafe {
            self.unicode_string_factory
                .to_string(nanoemModelMaterialGetClob(self.opaque))
        }
    }
    pub fn sphere_map_texture_type(&self) -> MaterialSphereMapTextureType {
        unsafe { nanoemModelMaterialGetSphereMapTextureType(self.opaque).cast() }
    }
    pub fn ambient_color(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelMaterialGetAmbientColor(self.opaque)) }
    }
    pub fn diffuse_color(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelMaterialGetDiffuseColor(self.opaque)) }
    }
    pub fn diffuse_opacity(&self) -> f32 {
        unsafe { nanoemModelMaterialGetDiffuseOpacity(self.opaque) }
    }
    pub fn specular_color(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelMaterialGetSpecularColor(self.opaque)) }
    }
    pub fn specular_power(&self) -> f32 {
        unsafe { nanoemModelMaterialGetSpecularPower(self.opaque) }
    }
    pub fn edge_color(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelMaterialGetEdgeColor(self.opaque)) }
    }
    pub fn edge_opacity(&self) -> f32 {
        unsafe { nanoemModelMaterialGetEdgeOpacity(self.opaque) }
    }
    pub fn edge_size(&self) -> f32 {
        unsafe { nanoemModelMaterialGetEdgeSize(self.opaque) }
    }
    pub fn num_vertex_indices(&self) -> u32 {
        unsafe { nanoemModelMaterialGetNumVertexIndices(self.opaque) }
    }
    pub fn toon_texture_index(&self) -> i32 {
        unsafe { nanoemModelMaterialGetToonTextureIndex(self.opaque) }
    }
    pub fn is_toon_shared(&self) -> bool {
        unsafe { nanoemModelMaterialIsToonShared(self.opaque) != 0 }
    }
    pub fn is_culling_disabled(&self) -> bool {
        unsafe { nanoemModelMaterialIsCullingDisabled(self.opaque) != 0 }
    }
    pub fn is_casting_shadow_enabled(&self) -> bool {
        unsafe { nanoemModelMaterialIsCastingShadowEnabled(self.opaque) != 0 }
    }
    pub fn is_casting_shadow_map_enabled(&self) -> bool {
        unsafe { nanoemModelMaterialIsCastingShadowMapEnabled(self.opaque) != 0 }
    }
    pub fn is_shadow_map_enabled(&self) -> bool {
        unsafe { nanoemModelMaterialIsShadowMapEnabled(self.opaque) != 0 }
    }
    pub fn is_edge_enabled(&self) -> bool {
        unsafe { nanoemModelMaterialIsEdgeEnabled(self.opaque) != 0 }
    }
    pub fn is_vertex_color_enabled(&self) -> bool {
        unsafe { nanoemModelMaterialIsVertexColorEnabled(self.opaque) != 0 }
    }
    pub fn is_point_draw_enabled(&self) -> bool {
        unsafe { nanoemModelMaterialIsPointDrawEnabled(self.opaque) != 0 }
    }
    pub fn is_line_draw_enabled(&self) -> bool {
        unsafe { nanoemModelMaterialIsLineDrawEnabled(self.opaque) != 0 }
    }
    pub fn has_diffuse_texture(&self) -> bool {
        unsafe { !nanoemModelMaterialGetDiffuseTextureObject(self.opaque).is_null() }
    }
    pub fn has_sphere_map_texture(&self) -> bool {
        unsafe { !nanoemModelMaterialGetSphereMapTextureObject(self.opaque).is_null() }
    }
    pub fn has_toon_texture(&self) -> bool {
        unsafe { !nanoemModelMaterialGetToonTextureObject(self.opaque).is_null() }
    }
}

impl<'a> std::fmt::Debug for Material<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            r#"Material {{
    index: {},
    name: {},
    diffuse_texture: {:?},
    sphere_map_texture: {:?},
    toon_texture: {:?},
    clob: {:?},
    sphere_map_type: {:?}
    ambient: {:?},
    diffuse: {:?},
    diffuse_opacity: {:?},
    specular: {:?},
    specular_power: {:?},
    edge: {:?},
    edge_opacity: {},
    edge_size: {},
    num_vertices_indices: {},
    toon_texture_index: {},
    is_toon_shared: {},
    is_culling_disabled: {},
    is_casting_shadow_enabled: {},
    is_casting_shadow_map_enabled: {},
    is_shadow_map_enabled: {},
    is_edge_enabled: {}
    is_vertex_color_enabled: {},
    is_point_draw_enabled: {},
    is_line_draw_enabled: {}
}}"#,
            self.index(),
            self.name(Language::Japanese).unwrap(),
            self.diffuse_texture(),
            self.sphere_map_texture(),
            self.toon_texture(),
            self.clob(),
            self.sphere_map_texture_type(),
            self.ambient_color(),
            self.diffuse_color(),
            self.diffuse_opacity(),
            self.specular_color(),
            self.specular_power(),
            self.edge_color(),
            self.edge_opacity(),
            self.edge_size(),
            self.num_vertex_indices(),
            self.toon_texture_index(),
            self.is_toon_shared(),
            self.is_culling_disabled(),
            self.is_casting_shadow_enabled(),
            self.is_casting_shadow_map_enabled(),
            self.is_shadow_map_enabled(),
            self.is_edge_enabled(),
            self.is_vertex_color_enabled(),
            self.is_point_draw_enabled(),
            self.is_line_draw_enabled()
        )
    }
}

pub struct Constraint<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *const nanoem_model_constraint_t,
}

impl<'a> Constraint<'a> {
    pub fn new(
        factory: &'a UnicodeStringFactory,
        opaque: *const nanoem_model_constraint_t,
    ) -> Self {
        Self {
            unicode_string_factory: factory,
            opaque,
        }
    }
    pub fn index(&self) -> i32 {
        unsafe { nanoemModelObjectGetIndex(nanoemModelConstraintGetModelObject(self.opaque)) }
    }
    pub fn all_constraint_joints(&self) -> Vec<ConstraintJoint> {
        let mut num_joints = 0usize;
        let ptr = unsafe {
            nanoemModelConstraintGetAllJointObjects(self.opaque, &mut num_joints as *mut usize)
        };
        let mut joints: Vec<ConstraintJoint> = Vec::with_capacity(num_joints);
        for i in 0..num_joints {
            joints.push(ConstraintJoint::new(&self.unicode_string_factory, unsafe {
                *(ptr.add(i))
            }));
        }
        joints
    }
    pub fn effector_bone(&self) -> Bone {
        Bone::new(&self.unicode_string_factory, unsafe {
            nanoemModelConstraintGetEffectorBoneObject(self.opaque)
        })
    }
    pub fn target_bone(&self) -> Bone {
        Bone::new(&self.unicode_string_factory, unsafe {
            nanoemModelConstraintGetTargetBoneObject(self.opaque)
        })
    }
    pub fn angle_limit(&self) -> f32 {
        unsafe { nanoemModelConstraintGetAngleLimit(self.opaque) }
    }
    pub fn num_iterations(&self) -> i32 {
        unsafe { nanoemModelConstraintGetNumIterations(self.opaque) }
    }
}

impl<'a> std::fmt::Debug for Constraint<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            r#"Constraint {{
    index: {},
    num_constraint_joints: {},
    effector: {},
    target: {},
    angle_limit: {},
    num_iterations: {}
}}"#,
            self.index(),
            self.all_constraint_joints().len(),
            self.effector_bone().name(Language::Japanese).unwrap(),
            self.target_bone().name(Language::Japanese).unwrap(),
            self.angle_limit(),
            self.num_iterations()
        )
    }
}

pub struct ConstraintJoint<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *const nanoem_model_constraint_joint_t,
}

impl<'a> ConstraintJoint<'a> {
    pub fn new(
        factory: &'a UnicodeStringFactory,
        opaque: *const nanoem_model_constraint_joint_t,
    ) -> Self {
        Self {
            unicode_string_factory: factory,
            opaque,
        }
    }
    pub fn bone(&self) -> Bone {
        Bone::new(&self.unicode_string_factory, unsafe {
            nanoemModelConstraintJointGetBoneObject(self.opaque)
        })
    }
    pub fn lower_limit(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelConstraintJointGetLowerLimit(self.opaque)) }
    }
    pub fn upper_limit(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelConstraintJointGetUpperLimit(self.opaque)) }
    }
    pub fn has_anglie_limit(&self) -> bool {
        unsafe { nanoemModelConstraintJointHasAngleLimit(self.opaque) != 0 }
    }
}

impl<'a> std::fmt::Debug for ConstraintJoint<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            r#"ConstraintJoint {{
    target: {},
    lower_limit: {:?},
    upper_limit: {:?},
    has_angle_limit: {}
}}"#,
            self.bone().name(Language::Japanese).unwrap(),
            self.lower_limit(),
            self.upper_limit(),
            self.has_anglie_limit(),
        )
    }
}

pub struct Bone<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *const nanoem_model_bone_t,
}

impl<'a> Bone<'a> {
    pub fn new(factory: &'a UnicodeStringFactory, opaque: *const nanoem_model_bone_t) -> Self {
        Self {
            unicode_string_factory: factory,
            opaque,
        }
    }
    pub fn index(&self) -> i32 {
        unsafe { nanoemModelObjectGetIndex(nanoemModelBoneGetModelObject(self.opaque)) }
    }
    pub fn name(&self, language: Language) -> Result<String> {
        unsafe {
            self.unicode_string_factory
                .to_string(nanoemModelBoneGetName(self.opaque, language.cast()))
        }
    }
    pub fn parent_bone(&self) -> Bone {
        Bone::new(&self.unicode_string_factory, unsafe {
            nanoemModelBoneGetParentBoneObject(self.opaque)
        })
    }
    pub fn inherent_parent_bone(&self) -> Bone {
        Bone::new(&self.unicode_string_factory, unsafe {
            nanoemModelBoneGetInherentParentBoneObject(self.opaque)
        })
    }
    pub fn effector_bone(&self) -> Bone {
        Bone::new(&self.unicode_string_factory, unsafe {
            nanoemModelBoneGetEffectorBoneObject(self.opaque)
        })
    }
    pub fn target_bone(&self) -> Bone {
        Bone::new(&self.unicode_string_factory, unsafe {
            nanoemModelBoneGetTargetBoneObject(self.opaque)
        })
    }
    pub fn constraint(&self) -> Constraint {
        Constraint::new(&self.unicode_string_factory, unsafe {
            nanoemModelBoneGetConstraintObject(self.opaque)
        })
    }
    pub fn origin(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelBoneGetOrigin(self.opaque)) }
    }
    pub fn destination_origin(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelBoneGetDestinationOrigin(self.opaque)) }
    }
    pub fn fixed_axis(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelBoneGetFixedAxis(self.opaque)) }
    }
    pub fn local_x_axis(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelBoneGetLocalXAxis(self.opaque)) }
    }
    pub fn local_z_axis(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelBoneGetLocalZAxis(self.opaque)) }
    }
    pub fn inherent_coefficient(&self) -> f32 {
        unsafe { nanoemModelBoneGetInherentCoefficient(self.opaque) }
    }
    pub fn stage_index(&self) -> i32 {
        unsafe { nanoemModelBoneGetStageIndex(self.opaque) }
    }
    pub fn has_destination_bone(&self) -> bool {
        unsafe { nanoemModelBoneHasDestinationBone(self.opaque) != 0 }
    }
    pub fn is_rotateable(&self) -> bool {
        unsafe { nanoemModelBoneIsRotateable(self.opaque) != 0 }
    }
    pub fn is_movable(&self) -> bool {
        unsafe { nanoemModelBoneIsMovable(self.opaque) != 0 }
    }
    pub fn is_visible(&self) -> bool {
        unsafe { nanoemModelBoneIsVisible(self.opaque) != 0 }
    }
    pub fn is_user_handleable(&self) -> bool {
        unsafe { nanoemModelBoneIsUserHandleable(self.opaque) != 0 }
    }
    pub fn has_constraint(&self) -> bool {
        unsafe { nanoemModelBoneHasConstraint(self.opaque) != 0 }
    }
    pub fn has_local_inherent(&self) -> bool {
        unsafe { nanoemModelBoneHasLocalInherent(self.opaque) != 0 }
    }
    pub fn has_inherent_translation(&self) -> bool {
        unsafe { nanoemModelBoneHasInherentTranslation(self.opaque) != 0 }
    }
    pub fn has_inherent_orientation(&self) -> bool {
        unsafe { nanoemModelBoneHasInherentOrientation(self.opaque) != 0 }
    }
    pub fn has_fixed_axis(&self) -> bool {
        unsafe { nanoemModelBoneHasFixedAxis(self.opaque) != 0 }
    }
    pub fn has_local_axes(&self) -> bool {
        unsafe { nanoemModelBoneHasLocalAxes(self.opaque) != 0 }
    }
    pub fn is_affected_by_physics_simulation(&self) -> bool {
        unsafe { nanoemModelBoneIsAffectedByPhysicsSimulation(self.opaque) != 0 }
    }
    pub fn has_external_parent_bone(&self) -> bool {
        unsafe { nanoemModelBoneHasExternalParentBone(self.opaque) != 0 }
    }
}

impl<'a> std::fmt::Debug for Bone<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            r#"Bone {{
    index: {},
    name: {},
    parent_bone: {},
    inherent_parent_bone: {},
    effector_bone: {},
    target_bone: {},
    constraint: {:?},
    origin: {:?},
    destination_origin: {:?},
    fixed_axis: {:?},
    local_x_axis: {:?},
    local_z_axis: {:?},
    inherent_coefficient: {},
    stage_index: {},
    has_destination_bone: {},
    is_rotateable: {},
    is_movable: {},
    is_visible: {},
    is_user_handleable: {},
    has_constraint: {},
    has_local_inherent: {},
    has_inherent_translation: {},
    has_inherent_orientation: {},
    has_fixed_axis: {},
    has_local_axes: {},
    is_affected_by_physics_simulation: {},
    has_external_parent_bone: {}
}}"#,
            self.index(),
            self.name(Language::Japanese).unwrap(),
            self.parent_bone().name(Language::Japanese).unwrap(),
            self.inherent_parent_bone()
                .name(Language::Japanese)
                .unwrap(),
            self.effector_bone().name(Language::Japanese).unwrap(),
            self.target_bone().name(Language::Japanese).unwrap(),
            self.constraint(),
            self.origin(),
            self.destination_origin(),
            self.fixed_axis(),
            self.local_x_axis(),
            self.local_z_axis(),
            self.inherent_coefficient(),
            self.stage_index(),
            self.has_destination_bone(),
            self.is_rotateable(),
            self.is_movable(),
            self.is_visible(),
            self.is_user_handleable(),
            self.has_constraint(),
            self.has_local_inherent(),
            self.has_inherent_translation(),
            self.has_inherent_orientation(),
            self.has_fixed_axis(),
            self.has_local_axes(),
            self.is_affected_by_physics_simulation(),
            self.has_external_parent_bone()
        )
    }
}

pub struct MorphBone<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *const nanoem_model_morph_bone_t,
}

impl<'a> MorphBone<'a> {
    pub fn new(
        factory: &'a UnicodeStringFactory,
        opaque: *const nanoem_model_morph_bone_t,
    ) -> Self {
        Self {
            unicode_string_factory: factory,
            opaque,
        }
    }
    pub fn bone(&self) -> Bone {
        Bone::new(&self.unicode_string_factory, unsafe {
            nanoemModelMorphBoneGetBoneObject(self.opaque)
        })
    }
    pub fn translation(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelMorphBoneGetTranslation(self.opaque)) }
    }
    pub fn orientation(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelMorphBoneGetOrientation(self.opaque)) }
    }
}

impl<'a> std::fmt::Debug for MorphBone<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            r#"MorphBone {{
    bone = {},
    translation = {:?},
    orientation = {:?},
}}"#,
            self.bone().name(Language::Japanese).unwrap(),
            self.translation(),
            self.orientation(),
        )
    }
}

pub struct MorphFlip<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *const nanoem_model_morph_flip_t,
}

impl<'a> MorphFlip<'a> {
    pub fn new(
        factory: &'a UnicodeStringFactory,
        opaque: *const nanoem_model_morph_flip_t,
    ) -> Self {
        Self {
            unicode_string_factory: factory,
            opaque,
        }
    }
    pub fn morph(&self) -> Morph {
        Morph::new(&self.unicode_string_factory, unsafe {
            nanoemModelMorphFlipGetMorphObject(self.opaque)
        })
    }
    pub fn weight(&self) -> f32 {
        unsafe { nanoemModelMorphFlipGetWeight(self.opaque) }
    }
}

impl<'a> std::fmt::Debug for MorphFlip<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            r#"MorphFlip {{
    morph = {},
    weight = {},
}}"#,
            self.morph().name(Language::Japanese).unwrap(),
            self.weight(),
        )
    }
}

pub struct MorphGroup<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *const nanoem_model_morph_group_t,
}

impl<'a> MorphGroup<'a> {
    pub fn new(
        factory: &'a UnicodeStringFactory,
        opaque: *const nanoem_model_morph_group_t,
    ) -> Self {
        Self {
            unicode_string_factory: factory,
            opaque,
        }
    }
    pub fn morph(&self) -> Morph {
        Morph::new(&self.unicode_string_factory, unsafe {
            nanoemModelMorphGroupGetMorphObject(self.opaque)
        })
    }
    pub fn weight(&self) -> f32 {
        unsafe { nanoemModelMorphGroupGetWeight(self.opaque) }
    }
}

impl<'a> std::fmt::Debug for MorphGroup<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            r#"MorphGroup {{
    morph = {},
    weight = {},
}}"#,
            self.morph().name(Language::Japanese).unwrap(),
            self.weight(),
        )
    }
}

pub struct MorphImpulse<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *const nanoem_model_morph_impulse_t,
}

impl<'a> MorphImpulse<'a> {
    pub fn new(
        factory: &'a UnicodeStringFactory,
        opaque: *const nanoem_model_morph_impulse_t,
    ) -> Self {
        Self {
            unicode_string_factory: factory,
            opaque,
        }
    }
    pub fn rigid_body(&self) -> RigidBody {
        RigidBody::new(&self.unicode_string_factory, unsafe {
            nanoemModelMorphImpulseGetRigidBodyObject(self.opaque)
        })
    }
    pub fn torque(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelMorphImpulseGetTorque(self.opaque)) }
    }
    pub fn velocity(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelMorphImpulseGetVelocity(self.opaque)) }
    }
    pub fn is_local(&self) -> bool {
        unsafe { nanoemModelMorphImpulseIsLocal(self.opaque) != 0 }
    }
}

impl<'a> std::fmt::Debug for MorphImpulse<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            r#"MorphImpulse {{
    rigid_body = {},
    torque = {:?},
    velocity = {:?},
    is_local = {}
}}"#,
            self.rigid_body().name(Language::Japanese).unwrap(),
            self.torque(),
            self.velocity(),
            self.is_local(),
        )
    }
}

pub struct MorphMaterial<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *const nanoem_model_morph_material_t,
}

#[derive(Copy, Clone, Debug, PartialEq)]
pub enum MorphMaterialOperationType {
    Unknown,
    Add,
    Multiply,
}

impl nanoem_model_morph_material_operation_type_t {
    pub fn cast(self) -> MorphMaterialOperationType {
        match self {
            nanoem_model_morph_material_operation_type_t::ADD => MorphMaterialOperationType::Add,
            nanoem_model_morph_material_operation_type_t::MULTIPLY => {
                MorphMaterialOperationType::Multiply
            }
            _ => MorphMaterialOperationType::Unknown,
        }
    }
}

impl MorphMaterialOperationType {
    pub(crate) fn cast(self) -> nanoem_model_morph_material_operation_type_t {
        match self {
            MorphMaterialOperationType::Add => nanoem_model_morph_material_operation_type_t::ADD,
            MorphMaterialOperationType::Multiply => {
                nanoem_model_morph_material_operation_type_t::MULTIPLY
            }
            _ => nanoem_model_morph_material_operation_type_t::UNKNOWN,
        }
    }
}

impl<'a> MorphMaterial<'a> {
    pub fn new(
        factory: &'a UnicodeStringFactory,
        opaque: *const nanoem_model_morph_material_t,
    ) -> Self {
        Self {
            unicode_string_factory: factory,
            opaque,
        }
    }
    pub fn material(&self) -> Material {
        Material::new(&self.unicode_string_factory, unsafe {
            nanoemModelMorphMaterialGetMaterialObject(self.opaque)
        })
    }
    pub fn diffuse_texture_blend(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelMorphMaterialGetDiffuseTextureBlend(self.opaque)) }
    }
    pub fn sphere_map_texture_blend(&self) -> Vector4 {
        unsafe {
            Vector4::from_ptr(nanoemModelMorphMaterialGetSphereMapTextureBlend(
                self.opaque,
            ))
        }
    }
    pub fn toon_texture_blend(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelMorphMaterialGetToonTextureBlend(self.opaque)) }
    }
    pub fn ambient_color(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelMorphMaterialGetAmbientColor(self.opaque)) }
    }
    pub fn diffuse_color(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelMorphMaterialGetDiffuseColor(self.opaque)) }
    }
    pub fn diffuse_opacity(&self) -> f32 {
        unsafe { nanoemModelMorphMaterialGetDiffuseOpacity(self.opaque) }
    }
    pub fn specular_color(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelMorphMaterialGetSpecularColor(self.opaque)) }
    }
    pub fn specular_power(&self) -> f32 {
        unsafe { nanoemModelMorphMaterialGetSpecularPower(self.opaque) }
    }
    pub fn edge_color(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelMorphMaterialGetEdgeColor(self.opaque)) }
    }
    pub fn edge_opacity(&self) -> f32 {
        unsafe { nanoemModelMorphMaterialGetEdgeOpacity(self.opaque) }
    }
    pub fn edge_size(&self) -> f32 {
        unsafe { nanoemModelMorphMaterialGetEdgeSize(self.opaque) }
    }
    pub fn operation_type(&self) -> MorphMaterialOperationType {
        unsafe { nanoemModelMorphMaterialGetOperationType(self.opaque).cast() }
    }
}

impl<'a> std::fmt::Debug for MorphMaterial<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            r#"MorphMaterial {{
    material = {},
    diffuse_texture_blend = {:?},
    sphere_map_texture_blend = {:?},
    toon_texture_blend = {:?},
    ambient = {:?},
    diffuse = {:?},
    diffuse_opacity = {},
    specular = {:?},
    specular_power = {},
    edge_color = {:?},
    edge_opacity = {},
    edge_size = {},
    operation_type = {:?}
}}"#,
            self.material().name(Language::Japanese).unwrap(),
            self.diffuse_texture_blend(),
            self.sphere_map_texture_blend(),
            self.toon_texture_blend(),
            self.ambient_color(),
            self.diffuse_color(),
            self.diffuse_opacity(),
            self.specular_color(),
            self.specular_power(),
            self.edge_color(),
            self.edge_opacity(),
            self.edge_size(),
            self.operation_type(),
        )
    }
}

pub struct MorphUV<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *const nanoem_model_morph_uv_t,
}

impl<'a> MorphUV<'a> {
    pub fn new(factory: &'a UnicodeStringFactory, opaque: *const nanoem_model_morph_uv_t) -> Self {
        Self {
            unicode_string_factory: factory,
            opaque,
        }
    }
    pub fn vertex(&self) -> Vertex {
        Vertex::new(&self.unicode_string_factory, unsafe {
            nanoemModelMorphUVGetVertexObject(self.opaque)
        })
    }
    pub fn position(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelMorphUVGetPosition(self.opaque)) }
    }
}

impl<'a> std::fmt::Debug for MorphUV<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            r#"MorphUV {{
    vertex = {},
    position = {:?}
}}"#,
            self.vertex().index(),
            self.position()
        )
    }
}

pub struct MorphVertex<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *const nanoem_model_morph_vertex_t,
}

impl<'a> MorphVertex<'a> {
    pub fn new(
        factory: &'a UnicodeStringFactory,
        opaque: *const nanoem_model_morph_vertex_t,
    ) -> Self {
        Self {
            unicode_string_factory: factory,
            opaque,
        }
    }
    pub fn vertex(&self) -> Vertex {
        Vertex::new(&self.unicode_string_factory, unsafe {
            nanoemModelMorphVertexGetVertexObject(self.opaque)
        })
    }
    pub fn position(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelMorphVertexGetPosition(self.opaque)) }
    }
}

impl<'a> std::fmt::Debug for MorphVertex<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            r#"MorphVertex {{
    vertex = {},
    position = {:?}
}}"#,
            self.vertex().index(),
            self.position()
        )
    }
}

#[derive(Copy, Clone, Debug, PartialEq)]
pub enum MorphCategory {
    Unknown,
    Base,
    Eyebrow,
    Eye,
    Lip,
    Other,
}

impl nanoem_model_morph_category_t {
    pub(crate) fn cast(self) -> MorphCategory {
        match self {
            nanoem_model_morph_category_t::BASE => MorphCategory::Base,
            nanoem_model_morph_category_t::EYE => MorphCategory::Eye,
            nanoem_model_morph_category_t::EYEBROW => MorphCategory::Eyebrow,
            nanoem_model_morph_category_t::LIP => MorphCategory::Lip,
            nanoem_model_morph_category_t::OTHER => MorphCategory::Other,
            _ => MorphCategory::Unknown,
        }
    }
}

impl MorphCategory {
    pub(crate) fn cast(self) -> nanoem_model_morph_category_t {
        match self {
            MorphCategory::Base => nanoem_model_morph_category_t::BASE,
            MorphCategory::Eye => nanoem_model_morph_category_t::EYE,
            MorphCategory::Eyebrow => nanoem_model_morph_category_t::EYEBROW,
            MorphCategory::Lip => nanoem_model_morph_category_t::LIP,
            MorphCategory::Other => nanoem_model_morph_category_t::OTHER,
            _ => nanoem_model_morph_category_t::UNKNOWN,
        }
    }
}

#[derive(Copy, Clone, Debug, PartialEq)]
pub enum MorphType {
    Unknown,
    Bone,
    Flip,
    Group,
    Impulse,
    Material,
    Texture,
    UVA1,
    UVA2,
    UVA3,
    UVA4,
    Vertex,
}

impl nanoem_model_morph_type_t {
    pub fn cast(self) -> MorphType {
        match self {
            nanoem_model_morph_type_t::BONE => MorphType::Bone,
            nanoem_model_morph_type_t::FLIP => MorphType::Flip,
            nanoem_model_morph_type_t::GROUP => MorphType::Group,
            nanoem_model_morph_type_t::IMPULUSE => MorphType::Impulse,
            nanoem_model_morph_type_t::MATERIAL => MorphType::Material,
            nanoem_model_morph_type_t::TEXTURE => MorphType::Texture,
            nanoem_model_morph_type_t::UVA1 => MorphType::UVA1,
            nanoem_model_morph_type_t::UVA2 => MorphType::UVA2,
            nanoem_model_morph_type_t::UVA3 => MorphType::UVA3,
            nanoem_model_morph_type_t::UVA4 => MorphType::UVA4,
            nanoem_model_morph_type_t::VERTEX => MorphType::Vertex,
            _ => MorphType::Unknown,
        }
    }
}

impl MorphType {
    pub(crate) fn cast(self) -> nanoem_model_morph_type_t {
        match self {
            MorphType::Bone => nanoem_model_morph_type_t::BONE,
            MorphType::Flip => nanoem_model_morph_type_t::FLIP,
            MorphType::Group => nanoem_model_morph_type_t::GROUP,
            MorphType::Impulse => nanoem_model_morph_type_t::IMPULUSE,
            MorphType::Material => nanoem_model_morph_type_t::MATERIAL,
            MorphType::Texture => nanoem_model_morph_type_t::TEXTURE,
            MorphType::UVA1 => nanoem_model_morph_type_t::UVA1,
            MorphType::UVA2 => nanoem_model_morph_type_t::UVA2,
            MorphType::UVA3 => nanoem_model_morph_type_t::UVA3,
            MorphType::UVA4 => nanoem_model_morph_type_t::UVA4,
            MorphType::Vertex => nanoem_model_morph_type_t::VERTEX,
            _ => nanoem_model_morph_type_t::UNKNOWN,
        }
    }
}

pub struct Morph<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *const nanoem_model_morph_t,
}

impl<'a> Morph<'a> {
    pub fn new(factory: &'a UnicodeStringFactory, opaque: *const nanoem_model_morph_t) -> Self {
        Self {
            unicode_string_factory: factory,
            opaque,
        }
    }
    pub fn index(&self) -> i32 {
        unsafe { nanoemModelObjectGetIndex(nanoemModelMorphGetModelObject(self.opaque)) }
    }
    pub fn name(&self, language: Language) -> Result<String> {
        unsafe {
            self.unicode_string_factory
                .to_string(nanoemModelMorphGetName(self.opaque, language.cast()))
        }
    }
    pub fn category(&self) -> MorphCategory {
        unsafe { nanoemModelMorphGetCategory(self.opaque).cast() }
    }
    pub fn morph_type(&self) -> MorphType {
        unsafe { nanoemModelMorphGetType(self.opaque).cast() }
    }
    pub fn all_bone_morphs(&self) -> Vec<MorphBone> {
        let mut num_objects = 0usize;
        let ptr = unsafe {
            nanoemModelMorphGetAllBoneMorphObjects(self.opaque, &mut num_objects as *mut usize)
        };
        let mut items: Vec<MorphBone> = Vec::with_capacity(num_objects);
        for i in 0..num_objects {
            items.push(MorphBone::new(&self.unicode_string_factory, unsafe {
                *(ptr.add(i))
            }));
        }
        items
    }
    pub fn all_flip_morphs(&self) -> Vec<MorphFlip> {
        let mut num_objects = 0usize;
        let ptr = unsafe {
            nanoemModelMorphGetAllFlipMorphObjects(self.opaque, &mut num_objects as *mut usize)
        };
        let mut items: Vec<MorphFlip> = Vec::with_capacity(num_objects);
        for i in 0..num_objects {
            items.push(MorphFlip::new(&self.unicode_string_factory, unsafe {
                *(ptr.add(i))
            }));
        }
        items
    }
    pub fn all_group_morphs(&self) -> Vec<MorphGroup> {
        let mut num_objects = 0usize;
        let ptr = unsafe {
            nanoemModelMorphGetAllGroupMorphObjects(self.opaque, &mut num_objects as *mut usize)
        };
        let mut items: Vec<MorphGroup> = Vec::with_capacity(num_objects);
        for i in 0..num_objects {
            items.push(MorphGroup::new(&self.unicode_string_factory, unsafe {
                *(ptr.add(i))
            }));
        }
        items
    }
    pub fn all_impulse_morphs(&self) -> Vec<MorphImpulse> {
        let mut num_objects = 0usize;
        let ptr = unsafe {
            nanoemModelMorphGetAllImpulseMorphObjects(self.opaque, &mut num_objects as *mut usize)
        };
        let mut items: Vec<MorphImpulse> = Vec::with_capacity(num_objects);
        for i in 0..num_objects {
            items.push(MorphImpulse::new(&self.unicode_string_factory, unsafe {
                *(ptr.add(i))
            }));
        }
        items
    }
    pub fn all_material_morphs(&self) -> Vec<MorphMaterial> {
        let mut num_objects = 0usize;
        let ptr = unsafe {
            nanoemModelMorphGetAllMaterialMorphObjects(self.opaque, &mut num_objects as *mut usize)
        };
        let mut items: Vec<MorphMaterial> = Vec::with_capacity(num_objects);
        for i in 0..num_objects {
            items.push(MorphMaterial::new(&self.unicode_string_factory, unsafe {
                *(ptr.add(i))
            }));
        }
        items
    }
    pub fn all_uv_morphs(&self) -> Vec<MorphUV> {
        let mut num_objects = 0usize;
        let ptr = unsafe {
            nanoemModelMorphGetAllUVMorphObjects(self.opaque, &mut num_objects as *mut usize)
        };
        let mut items: Vec<MorphUV> = Vec::with_capacity(num_objects);
        for i in 0..num_objects {
            items.push(MorphUV::new(&self.unicode_string_factory, unsafe {
                *(ptr.add(i))
            }));
        }
        items
    }
    pub fn all_vertex_morphs(&self) -> Vec<MorphVertex> {
        let mut num_objects = 0usize;
        let ptr = unsafe {
            nanoemModelMorphGetAllVertexMorphObjects(self.opaque, &mut num_objects as *mut usize)
        };
        let mut items: Vec<MorphVertex> = Vec::with_capacity(num_objects);
        for i in 0..num_objects {
            items.push(MorphVertex::new(&self.unicode_string_factory, unsafe {
                *(ptr.add(i))
            }));
        }
        items
    }
}

impl<'a> std::fmt::Debug for Morph<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            r#"Morph {{
    index: {},
    name: {},
    category: {:?},
    type: {:?},
    num_bone_morphs: {},
    num_flip_morphs: {},
    num_group_morphs: {},
    num_impulse_morphs: {},
    num_material_morphs: {},
    num_uv_morphs: {},
    num_vertex_morphs: {},
}}"#,
            self.index(),
            self.name(Language::Japanese).unwrap(),
            self.category(),
            self.morph_type(),
            self.all_bone_morphs().len(),
            self.all_flip_morphs().len(),
            self.all_group_morphs().len(),
            self.all_impulse_morphs().len(),
            self.all_material_morphs().len(),
            self.all_uv_morphs().len(),
            self.all_vertex_morphs().len(),
        )
    }
}

pub struct Label<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *const nanoem_model_label_t,
}

impl<'a> Label<'a> {
    pub fn new(factory: &'a UnicodeStringFactory, opaque: *const nanoem_model_label_t) -> Self {
        Self {
            unicode_string_factory: factory,
            opaque,
        }
    }
    pub fn index(&self) -> i32 {
        unsafe { nanoemModelObjectGetIndex(nanoemModelLabelGetModelObject(self.opaque)) }
    }
    pub fn all_items(&self) -> Vec<LabelItem> {
        let mut num_items = 0usize;
        let ptr =
            unsafe { nanoemModelLabelGetAllItemObjects(self.opaque, &mut num_items as *mut usize) };
        let mut items: Vec<LabelItem> = Vec::with_capacity(num_items);
        for i in 0..num_items {
            items.push(LabelItem::new(&self.unicode_string_factory, unsafe {
                *(ptr.add(i))
            }));
        }
        items
    }
    pub fn name(&self, language: Language) -> Result<String> {
        unsafe {
            self.unicode_string_factory
                .to_string(nanoemModelLabelGetName(self.opaque, language.cast()))
        }
    }
    pub fn is_special(&self) -> bool {
        unsafe { nanoemModelLabelIsSpecial(self.opaque) != 0 }
    }
}

impl<'a> std::fmt::Debug for Label<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            "Label {{
    index: {},
    name: {},
    is_special: {},
    num_items: {}
}}",
            self.index(),
            self.name(Language::Japanese).unwrap(),
            self.is_special(),
            self.all_items().len()
        )
    }
}

#[derive(Copy, Clone, Debug, PartialEq)]
pub enum LabelItemType {
    Unknown,
    Bone,
    Morph,
}

impl nanoem_model_label_item_type_t {
    pub fn cast(self) -> LabelItemType {
        match self {
            nanoem_model_label_item_type_t::BONE => LabelItemType::Bone,
            nanoem_model_label_item_type_t::MORPH => LabelItemType::Morph,
            _ => LabelItemType::Unknown,
        }
    }
}

pub struct LabelItem<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *const nanoem_model_label_item_t,
}

impl<'a> LabelItem<'a> {
    pub fn new(
        factory: &'a UnicodeStringFactory,
        opaque: *const nanoem_model_label_item_t,
    ) -> Self {
        Self {
            unicode_string_factory: factory,
            opaque,
        }
    }
    pub fn item_type(&self) -> LabelItemType {
        unsafe { nanoemModelLabelItemGetType(self.opaque).cast() }
    }
    pub fn bone(&self) -> Bone {
        Bone {
            unicode_string_factory: self.unicode_string_factory,
            opaque: unsafe { nanoemModelLabelItemGetBoneObject(self.opaque) },
        }
    }
    pub fn morph(&self) -> Morph {
        Morph {
            unicode_string_factory: self.unicode_string_factory,
            opaque: unsafe { nanoemModelLabelItemGetMorphObject(self.opaque) },
        }
    }
}

impl<'a> std::fmt::Debug for LabelItem<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        let item_type = self.item_type();
        match item_type {
            LabelItemType::Bone => write!(
                f,
                "LabelItem {{ type: {:?}, bone = {} }}",
                item_type,
                self.bone().name(Language::Japanese).unwrap()
            ),
            LabelItemType::Morph => write!(
                f,
                "LabelItem {{ type: {:?}, bone = {} }}",
                item_type,
                self.morph().name(Language::Japanese).unwrap()
            ),
            _ => write!(f, "LabelItem {{ type: {:?} }}", item_type),
        }
    }
}

#[derive(Copy, Clone, Debug, PartialEq)]
pub enum RigidBodyShapeType {
    Unknown,
    Sphere,
    Box,
    Capsure,
}

impl nanoem_model_rigid_body_shape_type_t {
    pub fn cast(self) -> RigidBodyShapeType {
        match self {
            nanoem_model_rigid_body_shape_type_t::BOX => RigidBodyShapeType::Box,
            nanoem_model_rigid_body_shape_type_t::CAPSULE => RigidBodyShapeType::Capsure,
            nanoem_model_rigid_body_shape_type_t::SPHERE => RigidBodyShapeType::Sphere,
            _ => RigidBodyShapeType::Unknown,
        }
    }
}

impl RigidBodyShapeType {
    pub(crate) fn cast(self) -> nanoem_model_rigid_body_shape_type_t {
        match self {
            RigidBodyShapeType::Box => nanoem_model_rigid_body_shape_type_t::BOX,
            RigidBodyShapeType::Capsure => nanoem_model_rigid_body_shape_type_t::CAPSULE,
            RigidBodyShapeType::Sphere => nanoem_model_rigid_body_shape_type_t::SPHERE,
            _ => nanoem_model_rigid_body_shape_type_t::UNKNOWN,
        }
    }
}

#[derive(Copy, Clone, Debug, PartialEq)]
pub enum RigidBodyTransformType {
    Unknown,
    FromBoneToSimulation,
    FromSimulationToBone,
    FromBoneOrientationAndSimulationTonBone,
}

impl nanoem_model_rigid_body_transform_type_t {
    pub fn cast(self) -> RigidBodyTransformType {
        match self {
            nanoem_model_rigid_body_transform_type_t::FROM_BONE_ORIENTATION_AND_SIMULATION_TO_BONE => RigidBodyTransformType::FromBoneOrientationAndSimulationTonBone,
            nanoem_model_rigid_body_transform_type_t::FROM_SIMULATION_TO_BONE => RigidBodyTransformType::FromSimulationToBone,
            nanoem_model_rigid_body_transform_type_t::FROM_BONE_TO_SIMULATION => RigidBodyTransformType::FromBoneToSimulation,
            _ => RigidBodyTransformType::Unknown,
        }
    }
}

impl RigidBodyTransformType {
    pub(crate) fn cast(self) -> nanoem_model_rigid_body_transform_type_t {
        match self {
            RigidBodyTransformType::FromBoneOrientationAndSimulationTonBone => nanoem_model_rigid_body_transform_type_t::FROM_BONE_ORIENTATION_AND_SIMULATION_TO_BONE,
            RigidBodyTransformType::FromSimulationToBone => nanoem_model_rigid_body_transform_type_t::FROM_SIMULATION_TO_BONE,
            RigidBodyTransformType::FromBoneToSimulation => nanoem_model_rigid_body_transform_type_t::FROM_BONE_TO_SIMULATION,
            _ => nanoem_model_rigid_body_transform_type_t::UNKNOWN,
        }
    }
}

pub struct RigidBody<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *const nanoem_model_rigid_body_t,
}

impl<'a> RigidBody<'a> {
    pub fn new(
        factory: &'a UnicodeStringFactory,
        opaque: *const nanoem_model_rigid_body_t,
    ) -> Self {
        Self {
            unicode_string_factory: factory,
            opaque,
        }
    }
    pub fn index(&self) -> i32 {
        unsafe { nanoemModelObjectGetIndex(nanoemModelRigidBodyGetModelObject(self.opaque)) }
    }
    pub fn name(&self, language: Language) -> Result<String> {
        unsafe {
            self.unicode_string_factory
                .to_string(nanoemModelRigidBodyGetName(self.opaque, language.cast()))
        }
    }
    pub fn bone(&self) -> Bone {
        Bone::new(&self.unicode_string_factory, unsafe {
            nanoemModelRigidBodyGetBoneObject(self.opaque)
        })
    }
    pub fn origin(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelRigidBodyGetOrigin(self.opaque)) }
    }
    pub fn orientation(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelRigidBodyGetOrientation(self.opaque)) }
    }
    pub fn shape_size(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelRigidBodyGetShapeSize(self.opaque)) }
    }
    pub fn transform_type(&self) -> RigidBodyTransformType {
        unsafe { nanoemModelRigidBodyGetTransformType(self.opaque).cast() }
    }
    pub fn shape_type(&self) -> RigidBodyShapeType {
        unsafe { nanoemModelRigidBodyGetShapeType(self.opaque).cast() }
    }
    pub fn mass(&self) -> f32 {
        unsafe { nanoemModelRigidBodyGetMass(self.opaque) }
    }
    pub fn linear_damping(&self) -> f32 {
        unsafe { nanoemModelRigidBodyGetLinearDamping(self.opaque) }
    }
    pub fn angular_damping(&self) -> f32 {
        unsafe { nanoemModelRigidBodyGetAngularDamping(self.opaque) }
    }
    pub fn friction(&self) -> f32 {
        unsafe { nanoemModelRigidBodyGetFriction(self.opaque) }
    }
    pub fn restitution(&self) -> f32 {
        unsafe { nanoemModelRigidBodyGetRestitution(self.opaque) }
    }
    pub fn collision_group_id(&self) -> i32 {
        unsafe { nanoemModelRigidBodyGetCollisionGroupId(self.opaque) }
    }
    pub fn collision_mask(&self) -> i32 {
        unsafe { nanoemModelRigidBodyGetCollisionMask(self.opaque) }
    }
}

impl<'a> std::fmt::Debug for RigidBody<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            r#"RigidBody {{
    index: {},
    name: {},
    bone: {},
    origin: {:?},
    orientation: {:?},
    shape_type: {:?},
    shape_size: {:?},
    transform_type: {:?},
    mass = {},
    linear_damping: {},
    angular_damping: {},
    friction: {},
    restitution: {},
    collision_group_id: {},
    collision_mask: {}
}}"#,
            self.index(),
            self.name(Language::Japanese).unwrap(),
            self.bone().name(Language::Japanese).unwrap(),
            self.origin(),
            self.orientation(),
            self.shape_type(),
            self.shape_size(),
            self.transform_type(),
            self.mass(),
            self.linear_damping(),
            self.angular_damping(),
            self.friction(),
            self.restitution(),
            self.collision_group_id(),
            self.collision_mask()
        )
    }
}

#[derive(Copy, Clone, Debug, PartialEq)]
pub enum JointType {
    Unknown,
    ConeTwist,
    Generic6Dof,
    Generic6DofSpring,
    Hinge,
    Point2Point,
    Slider,
}

impl nanoem_model_joint_type_t {
    pub fn cast(self) -> JointType {
        match self {
            nanoem_model_joint_type_t::CONE_TWIST_CONSTRAINT => JointType::ConeTwist,
            nanoem_model_joint_type_t::GENERIC_6DOF_CONSTRAINT => JointType::Generic6Dof,
            nanoem_model_joint_type_t::GENERIC_6DOF_SPRING_CONSTRAINT => {
                JointType::Generic6DofSpring
            }
            nanoem_model_joint_type_t::HINGE_CONSTRAINT => JointType::Hinge,
            nanoem_model_joint_type_t::POINT2POINT_CONSTRAINT => JointType::Point2Point,
            nanoem_model_joint_type_t::SLIDER_CONSTRAINT => JointType::Slider,
            _ => JointType::Unknown,
        }
    }
}

impl JointType {
    pub(crate) fn cast(self) -> nanoem_model_joint_type_t {
        match self {
            JointType::ConeTwist => nanoem_model_joint_type_t::CONE_TWIST_CONSTRAINT,
            JointType::Generic6Dof => nanoem_model_joint_type_t::GENERIC_6DOF_CONSTRAINT,
            JointType::Generic6DofSpring => {
                nanoem_model_joint_type_t::GENERIC_6DOF_SPRING_CONSTRAINT
            }
            JointType::Hinge => nanoem_model_joint_type_t::HINGE_CONSTRAINT,
            JointType::Point2Point => nanoem_model_joint_type_t::POINT2POINT_CONSTRAINT,
            JointType::Slider => nanoem_model_joint_type_t::SLIDER_CONSTRAINT,
            _ => panic!("unknown specified"),
        }
    }
}

pub struct Joint<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *const nanoem_model_joint_t,
}

impl<'a> Joint<'a> {
    pub fn new(factory: &'a UnicodeStringFactory, opaque: *const nanoem_model_joint_t) -> Self {
        Self {
            unicode_string_factory: factory,
            opaque,
        }
    }
    pub fn index(&self) -> i32 {
        unsafe { nanoemModelObjectGetIndex(nanoemModelJointGetModelObject(self.opaque)) }
    }
    pub fn name(&self, language: Language) -> Result<String> {
        unsafe {
            self.unicode_string_factory
                .to_string(nanoemModelJointGetName(self.opaque, language.cast()))
        }
    }
    pub fn rigid_body_a(&self) -> RigidBody {
        RigidBody::new(&self.unicode_string_factory, unsafe {
            nanoemModelJointGetRigidBodyAObject(self.opaque)
        })
    }
    pub fn rigid_body_b(&self) -> RigidBody {
        RigidBody::new(&self.unicode_string_factory, unsafe {
            nanoemModelJointGetRigidBodyBObject(self.opaque)
        })
    }
    pub fn joint_type(&self) -> JointType {
        unsafe { nanoemModelJointGetType(self.opaque).cast() }
    }
    pub fn origin(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelJointGetOrigin(self.opaque)) }
    }
    pub fn orientation(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelJointGetOrientation(self.opaque)) }
    }
    pub fn angular_lower_limit(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelJointGetAngularLowerLimit(self.opaque)) }
    }
    pub fn angular_upper_limit(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelJointGetAngularUpperLimit(self.opaque)) }
    }
    pub fn angular_stiffness(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelJointGetAngularStiffness(self.opaque)) }
    }
    pub fn linear_lower_limit(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelJointGetLinearLowerLimit(self.opaque)) }
    }
    pub fn linear_upper_limit(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelJointGetLinearUpperLimit(self.opaque)) }
    }
    pub fn linear_stiffness(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelJointGetLinearStiffness(self.opaque)) }
    }
}

impl<'a> std::fmt::Debug for Joint<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            r#"Joint {{
    index: {},
    name: {},
    type: {:?},
    rigid_body_a: {},
    rigid_body_b: {},
    origin: {:?},
    orientation: {:?},
    angular_lower_limit: {:?},
    angular_upper_limit: {:?},
    angular_stiffness: {:?},
    linear_lower_limit: {:?},
    linear_upper_limit: {:?},
    linear_stiffness: {:?}
}}"#,
            self.index(),
            self.name(Language::Japanese).unwrap(),
            self.joint_type(),
            self.rigid_body_a().name(Language::Japanese).unwrap(),
            self.rigid_body_b().name(Language::Japanese).unwrap(),
            self.origin(),
            self.orientation(),
            self.angular_lower_limit(),
            self.angular_upper_limit(),
            self.angular_stiffness(),
            self.linear_lower_limit(),
            self.linear_upper_limit(),
            self.linear_stiffness()
        )
    }
}
