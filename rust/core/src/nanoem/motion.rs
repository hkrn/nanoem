/*
  Copyright (c) 2015-2020 hkrn All rights reserved

  This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
*/

use super::common::*;
use super::native::*;

#[link(name = "nanoem")]
extern "C" {
    pub fn nanoemMotionLoadFromBufferNMD(
        motion: *mut nanoem_motion_t,
        buffer: *mut nanoem_buffer_t,
        offset: u32,
        status: *mut nanoem_status_t,
    ) -> i32;
}

#[derive(Copy, Clone, Debug)]
pub enum MotionFormatType {
    Unknown,
    VMD,
    NMD,
}

impl nanoem_motion_format_type_t {
    pub fn cast(self) -> MotionFormatType {
        match self {
            nanoem_motion_format_type_t::NMD => MotionFormatType::NMD,
            nanoem_motion_format_type_t::VMD => MotionFormatType::VMD,
            _ => MotionFormatType::Unknown,
        }
    }
}

impl MotionFormatType {
    pub(crate) fn cast(self) -> nanoem_motion_format_type_t {
        match self {
            MotionFormatType::NMD => nanoem_motion_format_type_t::NMD,
            MotionFormatType::VMD => nanoem_motion_format_type_t::VMD,
            _ => nanoem_motion_format_type_t::UNKNOWN,
        }
    }
}

pub struct Motion {
    pub(crate) unicode_string_factory: UnicodeStringFactory,
    pub(crate) opaque: *mut nanoem_motion_t,
}

impl Motion {
    pub fn new(factory: UnicodeStringFactory) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe { nanoemMotionCreate(factory.opaque, status_ptr) };
        status.value(Self {
            unicode_string_factory: factory,
            opaque,
        })
    }
    pub fn with_bytes(
        factory: UnicodeStringFactory,
        data: &[u8],
        offset: u32,
        format: MotionFormatType,
    ) -> Result<Motion> {
        let motion = Motion::new(factory)?;
        motion.load(data, offset, format)?;
        Ok(motion)
    }
    pub fn load(&self, data: &[u8], offset: u32, format: MotionFormatType) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let buffer = Buffer::from_slice(data)?;
        match format {
            MotionFormatType::NMD => {
                unsafe {
                    nanoemMotionLoadFromBufferNMD(self.opaque, buffer.opaque, offset, status_ptr)
                };
            }
            MotionFormatType::VMD => {
                unsafe {
                    nanoemMotionLoadFromBuffer(self.opaque, buffer.opaque, offset, status_ptr)
                };
            }
            _ => {
                if unsafe {
                    nanoemMotionLoadFromBufferNMD(self.opaque, buffer.opaque, offset, status_ptr)
                } != 0
                {
                    return status.unit();
                }
                unsafe {
                    nanoemMotionLoadFromBuffer(self.opaque, buffer.opaque, offset, status_ptr)
                };
            }
        }
        status.unit()
    }
    pub fn target_model_name(&self) -> Result<String> {
        unsafe {
            self.unicode_string_factory
                .to_string(nanoemMotionGetTargetModelName(self.opaque))
        }
    }
    pub fn format_type(&self) -> MotionFormatType {
        unsafe { nanoemMotionGetFormatType(self.opaque).cast() }
    }
    pub fn max_frame_index(&self) -> u32 {
        unsafe { nanoemMotionGetMaxFrameIndex(self.opaque) }
    }
    pub fn preferred_fps(&self) -> f32 {
        unsafe { nanoemMotionGetPreferredFPS(self.opaque) }
    }
    pub fn all_accessory_keyframes(&self) -> Vec<AccessoryKeyframe> {
        let mut num_keyframes = 0usize;
        let ptr = unsafe {
            nanoemMotionGetAllAccessoryKeyframeObjects(
                self.opaque,
                &mut num_keyframes as *mut usize,
            )
        };
        let mut keyframes: Vec<AccessoryKeyframe> = Vec::with_capacity(num_keyframes);
        let factory = &self.unicode_string_factory;
        for i in 0..num_keyframes {
            keyframes.push(AccessoryKeyframe::new(factory, unsafe { *(ptr.add(i)) }));
        }
        keyframes
    }
    pub fn all_bone_keyframes(&self) -> Vec<BoneKeyframe> {
        let mut num_keyframes = 0usize;
        let ptr = unsafe {
            nanoemMotionGetAllBoneKeyframeObjects(self.opaque, &mut num_keyframes as *mut usize)
        };
        let mut keyframes: Vec<BoneKeyframe> = Vec::with_capacity(num_keyframes);
        let factory = &self.unicode_string_factory;
        for i in 0..num_keyframes {
            keyframes.push(BoneKeyframe::new(factory, unsafe { *(ptr.add(i)) }));
        }
        keyframes
    }
    pub fn all_camera_keyframes(&self) -> Vec<CameraKeyframe> {
        let mut num_keyframes = 0usize;
        let ptr = unsafe {
            nanoemMotionGetAllCameraKeyframeObjects(self.opaque, &mut num_keyframes as *mut usize)
        };
        let mut keyframes: Vec<CameraKeyframe> = Vec::with_capacity(num_keyframes);
        let factory = &self.unicode_string_factory;
        for i in 0..num_keyframes {
            keyframes.push(CameraKeyframe::new(factory, unsafe { *(ptr.add(i)) }));
        }
        keyframes
    }
    pub fn all_light_keyframes(&self) -> Vec<LightKeyframe> {
        let mut num_keyframes = 0usize;
        let ptr = unsafe {
            nanoemMotionGetAllLightKeyframeObjects(self.opaque, &mut num_keyframes as *mut usize)
        };
        let mut keyframes: Vec<LightKeyframe> = Vec::with_capacity(num_keyframes);
        for i in 0..num_keyframes {
            keyframes.push(LightKeyframe::new(unsafe { *(ptr.add(i)) }));
        }
        keyframes
    }
    pub fn all_model_keyframes(&self) -> Vec<ModelKeyframe> {
        let mut num_keyframes = 0usize;
        let ptr = unsafe {
            nanoemMotionGetAllModelKeyframeObjects(self.opaque, &mut num_keyframes as *mut usize)
        };
        let mut keyframes: Vec<ModelKeyframe> = Vec::with_capacity(num_keyframes);
        let factory = &self.unicode_string_factory;
        for i in 0..num_keyframes {
            keyframes.push(ModelKeyframe::new(factory, unsafe { *(ptr.add(i)) }));
        }
        keyframes
    }
    pub fn all_morph_keyframes(&self) -> Vec<MorphKeyframe> {
        let mut num_keyframes = 0usize;
        let ptr = unsafe {
            nanoemMotionGetAllMorphKeyframeObjects(self.opaque, &mut num_keyframes as *mut usize)
        };
        let mut keyframes: Vec<MorphKeyframe> = Vec::with_capacity(num_keyframes);
        let factory = &self.unicode_string_factory;
        for i in 0..num_keyframes {
            keyframes.push(MorphKeyframe::new(factory, unsafe { *(ptr.add(i)) }));
        }
        keyframes
    }
    pub fn all_self_shadow_keyframes(&self) -> Vec<SelfShadowKeyframe> {
        let mut num_keyframes = 0usize;
        let ptr = unsafe {
            nanoemMotionGetAllSelfShadowKeyframeObjects(
                self.opaque,
                &mut num_keyframes as *mut usize,
            )
        };
        let mut keyframes: Vec<SelfShadowKeyframe> = Vec::with_capacity(num_keyframes);
        for i in 0..num_keyframes {
            keyframes.push(SelfShadowKeyframe::new(unsafe { *(ptr.add(i)) }));
        }
        keyframes
    }
    pub fn find_accessory_keyframe(&self, frame_index: u32) -> Option<AccessoryKeyframe> {
        let opaque = unsafe { nanoemMotionFindAccessoryKeyframeObject(self.opaque, frame_index) };
        if opaque.is_null() {
            return None;
        }
        Some(AccessoryKeyframe::new(&self.unicode_string_factory, opaque))
    }
    pub fn find_bone_keyframe(&self, name: &str, frame_index: u32) -> Result<Option<BoneKeyframe>> {
        let name = self.unicode_string_factory.to_unicode_string(name)?;
        let opaque =
            unsafe { nanoemMotionFindBoneKeyframeObject(self.opaque, name.value, frame_index) };
        if opaque.is_null() {
            return Ok(None);
        }
        Ok(Some(BoneKeyframe::new(
            &self.unicode_string_factory,
            opaque,
        )))
    }
    pub fn find_camera_keyframe(&self, frame_index: u32) -> Option<CameraKeyframe> {
        let opaque = unsafe { nanoemMotionFindCameraKeyframeObject(self.opaque, frame_index) };
        if opaque.is_null() {
            return None;
        }
        Some(CameraKeyframe::new(&self.unicode_string_factory, opaque))
    }
    pub fn find_light_keyframe(&self, frame_index: u32) -> Option<LightKeyframe> {
        let opaque = unsafe { nanoemMotionFindLightKeyframeObject(self.opaque, frame_index) };
        if opaque.is_null() {
            return None;
        }
        Some(LightKeyframe::new(opaque))
    }
    pub fn find_model_keyframe(&self, frame_index: u32) -> Option<ModelKeyframe> {
        let opaque = unsafe { nanoemMotionFindModelKeyframeObject(self.opaque, frame_index) };
        if opaque.is_null() {
            return None;
        }
        Some(ModelKeyframe::new(&self.unicode_string_factory, opaque))
    }
    pub fn find_morph_keyframe(
        &self,
        name: &str,
        frame_index: u32,
    ) -> Result<Option<MorphKeyframe>> {
        let name = self.unicode_string_factory.to_unicode_string(name)?;
        let opaque =
            unsafe { nanoemMotionFindMorphKeyframeObject(self.opaque, name.value, frame_index) };
        if opaque.is_null() {
            return Ok(None);
        }
        Ok(Some(MorphKeyframe::new(
            &self.unicode_string_factory,
            opaque,
        )))
    }
    pub fn find_self_shadow_keyframe(&self, frame_index: u32) -> Option<SelfShadowKeyframe> {
        let opaque = unsafe { nanoemMotionFindSelfShadowKeyframeObject(self.opaque, frame_index) };
        if opaque.is_null() {
            return None;
        }
        Some(SelfShadowKeyframe::new(opaque))
    }
    pub fn search_closest_bone_keyframes(
        &self,
        name: &str,
        frame_index: u32,
    ) -> Result<(Option<BoneKeyframe>, Option<BoneKeyframe>)> {
        let name = self.unicode_string_factory.to_unicode_string(name)?;
        let prev_keyframe: *mut nanoem_model_bone_keyframe_t = std::ptr::null_mut();
        let next_keyframe: *mut nanoem_model_bone_keyframe_t = std::ptr::null_mut();
        unsafe {
            nanoemMotionSearchClosestBoneKeyframes(
                self.opaque,
                name.value,
                frame_index,
                &prev_keyframe,
                &next_keyframe,
            )
        };
        let prev_keyframe = if !prev_keyframe.is_null() {
            Some(BoneKeyframe::new(
                &self.unicode_string_factory,
                prev_keyframe,
            ))
        } else {
            None
        };
        let next_keyframe = if !next_keyframe.is_null() {
            Some(BoneKeyframe::new(
                &self.unicode_string_factory,
                next_keyframe,
            ))
        } else {
            None
        };
        Ok((prev_keyframe, next_keyframe))
    }
    pub fn search_closest_morph_keyframes(
        &self,
        name: &str,
        frame_index: u32,
    ) -> Result<(Option<MorphKeyframe>, Option<MorphKeyframe>)> {
        let name = self.unicode_string_factory.to_unicode_string(name)?;
        let prev_keyframe: *mut nanoem_model_morph_keyframe_t = std::ptr::null_mut();
        let next_keyframe: *mut nanoem_model_morph_keyframe_t = std::ptr::null_mut();
        unsafe {
            nanoemMotionSearchClosestMorphKeyframes(
                self.opaque,
                name.value,
                frame_index,
                &prev_keyframe,
                &next_keyframe,
            )
        };
        let prev_keyframe = if !prev_keyframe.is_null() {
            Some(MorphKeyframe::new(
                &self.unicode_string_factory,
                prev_keyframe,
            ))
        } else {
            None
        };
        let next_keyframe = if !next_keyframe.is_null() {
            Some(MorphKeyframe::new(
                &self.unicode_string_factory,
                next_keyframe,
            ))
        } else {
            None
        };
        Ok((prev_keyframe, next_keyframe))
    }
}

impl std::fmt::Debug for Motion {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            r#"Motion {{
    target_model_name: {},
    format_type: {:?},
    max_frame_index: {},
    num_accessory_keyframes: {},
    num_bone_keyframes: {},
    num_camera_keyframes: {},
    num_light_keyframes: {},
    num_model_keyframes: {},
    num_morph_keyframes: {},
    num_self_shadow_keyframes: {}
}}"#,
            self.target_model_name().unwrap(),
            self.format_type(),
            self.max_frame_index(),
            self.all_accessory_keyframes().len(),
            self.all_bone_keyframes().len(),
            self.all_camera_keyframes().len(),
            self.all_light_keyframes().len(),
            self.all_model_keyframes().len(),
            self.all_morph_keyframes().len(),
            self.all_self_shadow_keyframes().len()
        )
    }
}

impl Drop for Motion {
    fn drop(&mut self) {
        unsafe {
            nanoemMotionDestroy(self.opaque);
        }
    }
}

#[derive(Copy, Clone, Debug)]
pub enum EffectParameterVariant {
    Bool(bool),
    Int32(i32),
    Float32(f32),
    Vector4(f32, f32, f32, f32),
}

pub struct EffectParameter<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *const nanoem_effect_parameter_t,
}

impl<'a> EffectParameter<'a> {
    pub fn new(
        factory: &'a UnicodeStringFactory,
        opaque: *const nanoem_effect_parameter_t,
    ) -> Self {
        Self {
            unicode_string_factory: factory,
            opaque,
        }
    }
    pub fn name(&self) -> Result<String> {
        unsafe {
            self.unicode_string_factory
                .to_string(nanoemEffectParameterGetName(self.opaque))
        }
    }
    pub fn value(&self) -> EffectParameterVariant {
        unsafe {
            match nanoemEffectParameterGetType(self.opaque) {
                nanoem_effect_parameter_type_t::BOOL => EffectParameterVariant::Bool(false),
                nanoem_effect_parameter_type_t::FLOAT => EffectParameterVariant::Float32(0.0),
                nanoem_effect_parameter_type_t::INT => EffectParameterVariant::Int32(0),
                nanoem_effect_parameter_type_t::VECTOR4 => {
                    EffectParameterVariant::Vector4(0.0, 0.0, 0.0, 0.0)
                }
                _ => EffectParameterVariant::Bool(false),
            }
        }
    }
}

impl<'a> std::fmt::Debug for EffectParameter<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            r#"EffectParameter {{
    name: {}
    value: {:?},
}}"#,
            self.name().unwrap(),
            self.value(),
        )
    }
}

pub struct OutsideParent<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *const nanoem_outside_parent_t,
}

impl<'a> OutsideParent<'a> {
    pub fn new(factory: &'a UnicodeStringFactory, opaque: *const nanoem_outside_parent_t) -> Self {
        Self {
            unicode_string_factory: factory,
            opaque,
        }
    }
    pub fn subject_bone_name(&self) -> Result<String> {
        unsafe {
            self.unicode_string_factory
                .to_string(nanoemOutsideParentGetSubjectBoneName(self.opaque))
        }
    }
    pub fn target_object_name(&self) -> Result<String> {
        unsafe {
            self.unicode_string_factory
                .to_string(nanoemOutsideParentGetTargetObjectName(self.opaque))
        }
    }
    pub fn target_bone_name(&self) -> Result<String> {
        unsafe {
            self.unicode_string_factory
                .to_string(nanoemOutsideParentGetTargetBoneName(self.opaque))
        }
    }
}

impl<'a> std::fmt::Debug for OutsideParent<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            r#"OutsideParent {{
    subject_bone_name: {},
    target_object_name: {},
    target_bone_name: {}
}}"#,
            self.subject_bone_name().unwrap(),
            self.target_object_name().unwrap(),
            self.target_bone_name().unwrap()
        )
    }
}

pub struct AccessoryKeyframe<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *const nanoem_accessory_keyframe_t,
}

impl<'a> AccessoryKeyframe<'a> {
    pub fn new(
        factory: &'a UnicodeStringFactory,
        opaque: *const nanoem_accessory_keyframe_t,
    ) -> Self {
        Self {
            unicode_string_factory: factory,
            opaque,
        }
    }
    pub fn frame_index(&self) -> u32 {
        unsafe {
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemAccessoryKeyframeGetKeyframeObject(
                self.opaque,
            ))
        }
    }
    pub fn all_effect_parameters(&'a self) -> Vec<EffectParameter<'a>> {
        let mut num_objects = 0usize;
        let ptr = unsafe {
            nanoemAccessoryKeyframeGetAllEffectParameterObjects(
                self.opaque,
                &mut num_objects as *mut usize,
            )
        };
        let mut parameters: Vec<EffectParameter> = Vec::with_capacity(num_objects);
        let factory = self.unicode_string_factory;
        for i in 0..num_objects {
            parameters.push(EffectParameter::new(factory, unsafe { *(ptr.add(i)) }));
        }
        parameters
    }
    pub fn outside_parent(&self) -> OutsideParent {
        OutsideParent::new(&self.unicode_string_factory, unsafe {
            nanoemAccessoryKeyframeGetOutsideParent(self.opaque)
        })
    }
    pub fn translation(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemAccessoryKeyframeGetTranslation(self.opaque)) }
    }
    pub fn orientation(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemAccessoryKeyframeGetOrientation(self.opaque)) }
    }
    pub fn scale_factor(&self) -> f32 {
        unsafe { nanoemAccessoryKeyframeGetScaleFactor(self.opaque) }
    }
    pub fn opacity(&self) -> f32 {
        unsafe { nanoemAccessoryKeyframeGetOpacity(self.opaque) }
    }
    pub fn is_add_blend_enabled(&self) -> bool {
        unsafe { nanoemAccessoryKeyframeIsAddBlendEnabled(self.opaque) != 0 }
    }
    pub fn is_visible(&self) -> bool {
        unsafe { nanoemAccessoryKeyframeIsVisible(self.opaque) != 0 }
    }
    pub fn is_shadow_enabled(&self) -> bool {
        unsafe { nanoemAccessoryKeyframeIsShadowEnabled(self.opaque) != 0 }
    }
}

impl<'a> std::fmt::Debug for AccessoryKeyframe<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            r#"AccessoryKeyframe {{
    frame_index: {},
    num_effect_parameters: {},
    outside_parent: {:?},
    translation: {:?},
    orientation: {:?},
    scale_factor: {},
    opacity: {},
    is_add_blend_enabled: {},
    is_visible: {},
    is_shadow_enabled: {}
}}"#,
            self.frame_index(),
            self.all_effect_parameters().len(),
            self.outside_parent(),
            self.translation(),
            self.orientation(),
            self.scale_factor(),
            self.opacity(),
            self.is_add_blend_enabled(),
            self.is_visible(),
            self.is_shadow_enabled()
        )
    }
}

pub struct BoneKeyframe<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *const nanoem_model_bone_keyframe_t,
}

#[derive(Copy, Clone, Debug)]
pub enum BoneKeyframeInterpolationType {
    TranslationX,
    TranslationY,
    TranslationZ,
    Orientation,
}

impl BoneKeyframeInterpolationType {
    pub(crate) fn cast(self) -> nanoem_model_bone_keyframe_interpolation_type_t {
        match self {
            BoneKeyframeInterpolationType::TranslationX => {
                nanoem_model_bone_keyframe_interpolation_type_t::TRANSLATION_X
            }
            BoneKeyframeInterpolationType::TranslationY => {
                nanoem_model_bone_keyframe_interpolation_type_t::TRANSLATION_Y
            }
            BoneKeyframeInterpolationType::TranslationZ => {
                nanoem_model_bone_keyframe_interpolation_type_t::TRANSLATION_Z
            }
            BoneKeyframeInterpolationType::Orientation => {
                nanoem_model_bone_keyframe_interpolation_type_t::ORIENTATION
            }
        }
    }
}

impl<'a> BoneKeyframe<'a> {
    pub fn new(
        factory: &'a UnicodeStringFactory,
        opaque: *const nanoem_model_bone_keyframe_t,
    ) -> Self {
        Self {
            unicode_string_factory: factory,
            opaque,
        }
    }
    pub fn frame_index(&self) -> u32 {
        unsafe {
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemModelBoneKeyframeGetKeyframeObject(
                self.opaque,
            ))
        }
    }
    pub fn name(&self) -> Result<String> {
        unsafe {
            self.unicode_string_factory
                .to_string(nanoemModelBoneKeyframeGetName(self.opaque))
        }
    }
    pub fn translation(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelBoneKeyframeGetTranslation(self.opaque)) }
    }
    pub fn orientation(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelBoneKeyframeGetOrientation(self.opaque)) }
    }
    pub fn interpolation(&self, value: BoneKeyframeInterpolationType) -> *const u8 {
        unsafe { nanoemModelBoneKeyframeGetInterpolation(self.opaque, value.cast()) }
    }
    pub fn stage_index(&self) -> u32 {
        unsafe { nanoemModelBoneKeyframeGetStageIndex(self.opaque) }
    }
    pub fn is_physics_simulation_enabled(&self) -> bool {
        unsafe { nanoemModelBoneKeyframeIsPhysicsSimulationEnabled(self.opaque) != 0 }
    }
    pub fn is_linear_interpolation(&self, value: BoneKeyframeInterpolationType) -> bool {
        unsafe { nanoemModelBoneKeyframeIsLinearInterpolation(self.opaque, value.cast()) != 0 }
    }
}

impl<'a> std::fmt::Debug for BoneKeyframe<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            r#"BoneKeyframe {{
    frame_index: {},
    name: {},
    translation: {:?},
    orientation: {:?},
    interpolation: ({:?}, {:?}, {:?}, {:?}),
    stage_index: {},
    is_physics_simulation_enabled: {},
    is_linear_interpolation: ({}, {}, {}, {}),
}}"#,
            self.frame_index(),
            self.name().unwrap(),
            self.translation(),
            self.orientation(),
            self.interpolation(BoneKeyframeInterpolationType::TranslationX),
            self.interpolation(BoneKeyframeInterpolationType::TranslationY),
            self.interpolation(BoneKeyframeInterpolationType::TranslationZ),
            self.interpolation(BoneKeyframeInterpolationType::Orientation),
            self.stage_index(),
            self.is_physics_simulation_enabled(),
            self.is_linear_interpolation(BoneKeyframeInterpolationType::TranslationX),
            self.is_linear_interpolation(BoneKeyframeInterpolationType::TranslationY),
            self.is_linear_interpolation(BoneKeyframeInterpolationType::TranslationZ),
            self.is_linear_interpolation(BoneKeyframeInterpolationType::Orientation),
        )
    }
}

pub struct CameraKeyframe<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *const nanoem_camera_keyframe_t,
}

#[derive(Copy, Clone, Debug)]
pub enum CameraKeyframeInterpolationType {
    LookAtX,
    LookAtY,
    LookAtZ,
    Angle,
    Fov,
    Distance,
}

impl CameraKeyframeInterpolationType {
    pub(crate) fn cast(self) -> nanoem_camera_keyframe_interpolation_type_t {
        match self {
            CameraKeyframeInterpolationType::LookAtX => {
                nanoem_camera_keyframe_interpolation_type_t::LOOKAT_X
            }
            CameraKeyframeInterpolationType::LookAtY => {
                nanoem_camera_keyframe_interpolation_type_t::LOOKAT_Y
            }
            CameraKeyframeInterpolationType::LookAtZ => {
                nanoem_camera_keyframe_interpolation_type_t::LOOKAT_Z
            }
            CameraKeyframeInterpolationType::Angle => {
                nanoem_camera_keyframe_interpolation_type_t::ANGLE
            }
            CameraKeyframeInterpolationType::Fov => {
                nanoem_camera_keyframe_interpolation_type_t::FOV
            }
            CameraKeyframeInterpolationType::Distance => {
                nanoem_camera_keyframe_interpolation_type_t::DISTANCE
            }
        }
    }
}

impl<'a> CameraKeyframe<'a> {
    pub fn new(factory: &'a UnicodeStringFactory, opaque: *const nanoem_camera_keyframe_t) -> Self {
        Self {
            unicode_string_factory: factory,
            opaque,
        }
    }
    pub fn frame_index(&self) -> u32 {
        unsafe {
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemCameraKeyframeGetKeyframeObject(
                self.opaque,
            ))
        }
    }
    pub fn outside_parent(&self) -> OutsideParent {
        OutsideParent::new(&self.unicode_string_factory, unsafe {
            nanoemCameraKeyframeGetOutsideParent(self.opaque)
        })
    }
    pub fn look_at(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemCameraKeyframeGetLookAt(self.opaque)) }
    }
    pub fn angle(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemCameraKeyframeGetAngle(self.opaque)) }
    }
    pub fn interpolation(&self, value: CameraKeyframeInterpolationType) -> *const u8 {
        unsafe { nanoemCameraKeyframeGetInterpolation(self.opaque, value.cast()) }
    }
    pub fn fov(&self) -> i32 {
        unsafe { nanoemCameraKeyframeGetFov(self.opaque) }
    }
    pub fn distance(&self) -> f32 {
        unsafe { nanoemCameraKeyframeGetDistance(self.opaque) }
    }
    pub fn stage_index(&self) -> u32 {
        unsafe { nanoemCameraKeyframeGetStageIndex(self.opaque) }
    }
    pub fn is_perspective(&self) -> bool {
        unsafe { nanoemCameraKeyframeIsPerspectiveView(self.opaque) != 0 }
    }
    pub fn is_linear_interpolation(&self, value: CameraKeyframeInterpolationType) -> bool {
        unsafe { nanoemCameraKeyframeIsLinearInterpolation(self.opaque, value.cast()) != 0 }
    }
}

impl<'a> std::fmt::Debug for CameraKeyframe<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            r#"CameraKeyframe {{
    frame_index: {},
    outside_parent: {:?}
    look_at: {:?},
    angle: {:?},
    fov: {},
    distance: {},
    interpolation: ({:?}, {:?}, {:?}, {:?}, {:?}, {:?}),
    stage_index: {},
    is_perspective: {},
    is_linear_interpolation: ({}, {}, {}, {}, {}, {}),
}}"#,
            self.frame_index(),
            self.outside_parent(),
            self.look_at(),
            self.angle(),
            self.fov(),
            self.distance(),
            self.interpolation(CameraKeyframeInterpolationType::LookAtX),
            self.interpolation(CameraKeyframeInterpolationType::LookAtY),
            self.interpolation(CameraKeyframeInterpolationType::LookAtZ),
            self.interpolation(CameraKeyframeInterpolationType::Angle),
            self.interpolation(CameraKeyframeInterpolationType::Fov),
            self.interpolation(CameraKeyframeInterpolationType::Distance),
            self.stage_index(),
            self.is_perspective(),
            self.is_linear_interpolation(CameraKeyframeInterpolationType::LookAtX),
            self.is_linear_interpolation(CameraKeyframeInterpolationType::LookAtY),
            self.is_linear_interpolation(CameraKeyframeInterpolationType::LookAtZ),
            self.is_linear_interpolation(CameraKeyframeInterpolationType::Angle),
            self.is_linear_interpolation(CameraKeyframeInterpolationType::Fov),
            self.is_linear_interpolation(CameraKeyframeInterpolationType::Distance),
        )
    }
}

pub struct LightKeyframe {
    opaque: *const nanoem_light_keyframe_t,
}

impl LightKeyframe {
    pub fn new(opaque: *const nanoem_light_keyframe_t) -> Self {
        Self { opaque }
    }
    pub fn frame_index(&self) -> u32 {
        unsafe {
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemLightKeyframeGetKeyframeObject(
                self.opaque,
            ))
        }
    }
    pub fn color(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemLightKeyframeGetColor(self.opaque)) }
    }
    pub fn direction(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemLightKeyframeGetDirection(self.opaque)) }
    }
}

impl<'a> std::fmt::Debug for LightKeyframe {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            r#"LightKeyframe {{
    frame_index: {},
    color: {:?},
    direction: {:?}
}}"#,
            self.frame_index(),
            self.color(),
            self.direction()
        )
    }
}

pub struct ModelKeyframeConstraintState<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *const nanoem_model_keyframe_constraint_state_t,
}

impl<'a> ModelKeyframeConstraintState<'a> {
    pub fn new(
        factory: &'a UnicodeStringFactory,
        opaque: *const nanoem_model_keyframe_constraint_state_t,
    ) -> Self {
        Self {
            unicode_string_factory: factory,
            opaque,
        }
    }
    pub fn name(&self) -> Result<String> {
        unsafe {
            self.unicode_string_factory
                .to_string(nanoemModelKeyframeConstraintStateGetBoneName(self.opaque))
        }
    }
    pub fn bone_id(&self) -> i32 {
        unsafe { nanoemModelKeyframeConstraintStateGetBoneId(self.opaque) }
    }
    pub fn is_enabled(&self) -> bool {
        unsafe { nanoemModelKeyframeConstraintStateIsEnabled(self.opaque) != 0 }
    }
}

impl<'a> std::fmt::Debug for ModelKeyframeConstraintState<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            r#"ModelKeyframeConstraintState {{
    name: {},
    bone_id: {},
    is_enabled: {}
}}"#,
            self.name().unwrap(),
            self.bone_id(),
            self.is_enabled()
        )
    }
}

pub struct ModelKeyframe<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *const nanoem_model_keyframe_t,
}

impl<'a> ModelKeyframe<'a> {
    pub fn new(factory: &'a UnicodeStringFactory, opaque: *const nanoem_model_keyframe_t) -> Self {
        Self {
            unicode_string_factory: factory,
            opaque,
        }
    }
    pub fn frame_index(&self) -> u32 {
        unsafe {
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemModelKeyframeGetKeyframeObject(
                self.opaque,
            ))
        }
    }
    pub fn all_constraint_states(&self) -> Vec<ModelKeyframeConstraintState> {
        let mut num_states = 0usize;
        let ptr = unsafe {
            nanoemModelKeyframeGetAllConstraintStateObjects(
                self.opaque,
                &mut num_states as *mut usize,
            )
        };
        let mut states: Vec<ModelKeyframeConstraintState> = Vec::with_capacity(num_states);
        for i in 0..num_states {
            states.push(ModelKeyframeConstraintState::new(
                self.unicode_string_factory,
                unsafe { *(ptr.add(i)) },
            ))
        }
        states
    }
    pub fn all_effect_parameters(&'a self) -> Vec<EffectParameter<'a>> {
        let mut num_objects = 0usize;
        let ptr = unsafe {
            nanoemModelKeyframeGetAllEffectParameterObjects(
                self.opaque,
                &mut num_objects as *mut usize,
            )
        };
        let mut parameters: Vec<EffectParameter> = Vec::with_capacity(num_objects);
        let factory = self.unicode_string_factory;
        for i in 0..num_objects {
            parameters.push(EffectParameter::new(factory, unsafe { *(ptr.add(i)) }));
        }
        parameters
    }
    pub fn all_outside_parents(&self) -> Vec<OutsideParent> {
        let mut num_parents = 0usize;
        let ptr = unsafe {
            nanoemModelKeyframeGetAllOutsideParentObjects(
                self.opaque,
                &mut num_parents as *mut usize,
            )
        };
        let mut parents: Vec<OutsideParent> = Vec::with_capacity(num_parents);
        for i in 0..num_parents {
            parents.push(OutsideParent::new(self.unicode_string_factory, unsafe {
                *(ptr.add(i))
            }))
        }
        parents
    }
    pub fn edge_color(&self) -> Vector4 {
        unsafe { Vector4::from_ptr(nanoemModelKeyframeGetEdgeColor(self.opaque)) }
    }
    pub fn edge_scale_factor(&self) -> f32 {
        unsafe { nanoemModelKeyframeGetEdgeScaleFactor(self.opaque) }
    }
    pub fn is_add_blend_enabled(&self) -> bool {
        unsafe { nanoemModelKeyframeIsAddBlendEnabled(self.opaque) != 0 }
    }
    pub fn is_physics_simulation_enabled(&self) -> bool {
        unsafe { nanoemModelKeyframeIsPhysicsSimulationEnabled(self.opaque) != 0 }
    }
    pub fn is_visible(&self) -> bool {
        unsafe { nanoemModelKeyframeIsVisible(self.opaque) != 0 }
    }
}

impl<'a> std::fmt::Debug for ModelKeyframe<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            r#"ModelKeyframe {{
    frame_index: {},
    num_constraint_states: {},
    num_effect_parameters: {},
    num_outside_parents: {},
    edge_color: {:?},
    edge_scale_factor: {:?},
    is_add_blend_enabled: {},
    is_physics_simulation_enabled: {},
    is_visible: {}
}}"#,
            self.frame_index(),
            self.all_constraint_states().len(),
            self.all_effect_parameters().len(),
            self.all_outside_parents().len(),
            self.edge_color(),
            self.edge_scale_factor(),
            self.is_add_blend_enabled(),
            self.is_physics_simulation_enabled(),
            self.is_visible()
        )
    }
}

pub struct MorphKeyframe<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *const nanoem_model_morph_keyframe_t,
}

impl<'a> MorphKeyframe<'a> {
    pub fn new(
        factory: &'a UnicodeStringFactory,
        opaque: *const nanoem_model_morph_keyframe_t,
    ) -> Self {
        Self {
            unicode_string_factory: factory,
            opaque,
        }
    }
    pub fn frame_index(&self) -> u32 {
        unsafe {
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemModelMorphKeyframeGetKeyframeObject(
                self.opaque,
            ))
        }
    }
    pub fn name(&self) -> Result<String> {
        unsafe {
            self.unicode_string_factory
                .to_string(nanoemModelMorphKeyframeGetName(self.opaque))
        }
    }
    pub fn weight(&self) -> f32 {
        unsafe { nanoemModelMorphKeyframeGetWeight(self.opaque) }
    }
}

impl<'a> std::fmt::Debug for MorphKeyframe<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            r#"MorphKeyframe {{
    frame_index: {},
    name: {},
    weight: {}
}}"#,
            self.frame_index(),
            self.name().unwrap(),
            self.weight()
        )
    }
}

pub struct SelfShadowKeyframe {
    opaque: *const nanoem_self_shadow_keyframe_t,
}

impl<'a> SelfShadowKeyframe {
    pub fn new(opaque: *const nanoem_self_shadow_keyframe_t) -> Self {
        Self { opaque }
    }
    pub fn frame_index(&self) -> u32 {
        unsafe {
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemSelfShadowKeyframeGetKeyframeObject(
                self.opaque,
            ))
        }
    }
    pub fn distance(&self) -> f32 {
        unsafe { nanoemSelfShadowKeyframeGetDistance(self.opaque) }
    }
    pub fn mode(&self) -> i32 {
        unsafe { nanoemSelfShadowKeyframeGetMode(self.opaque) }
    }
}

impl<'a> std::fmt::Debug for SelfShadowKeyframe {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            r#"SelfShadowKeyframe {{
    frame_index: {},
    distance: {},
    mode: {}
}}"#,
            self.frame_index(),
            self.distance(),
            self.mode()
        )
    }
}
