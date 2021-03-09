/*
  Copyright (c) 2015-2020 hkrn All rights reserved

  This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
*/

use super::common::*;
use super::motion::*;
use super::native::*;

#[link(name = "nanoem")]
extern "C" {
    pub fn nanoemMutableMotionSaveToBufferNMD(
        motion: *mut nanoem_mutable_motion_t,
        buffer: *mut nanoem_mutable_buffer_t,
        status: *mut nanoem_status_t,
    ) -> i32;
}

pub struct MutableMotion {
    unicode_string_factory: UnicodeStringFactory,
    opaque: *mut nanoem_mutable_motion_t,
}

impl<'a> MutableMotion {
    #[allow(dead_code)]
    pub fn new(factory: UnicodeStringFactory) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe { nanoemMutableMotionCreate(factory.opaque, status_ptr) };
        status.value(Self {
            unicode_string_factory: factory,
            opaque,
        })
    }
    pub fn from_motion(motion: &'a Motion) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let factory = UnicodeStringFactory::create()?;
        let opaque = unsafe { nanoemMutableMotionCreateAsReference(motion.opaque, status_ptr) };
        status.value(Self {
            unicode_string_factory: factory,
            opaque,
        })
    }
    pub fn save(&self, data: &mut Vec<u8>, format: MotionFormatType) -> Result<()> {
        let mut mutable_buffer = MutableBuffer::new()?;
        match format.cast() {
            nanoem_motion_format_type_t::NMD => self.save_buffer_as_nmd(&mut mutable_buffer)?,
            nanoem_motion_format_type_t::VMD => self.save_buffer_as_vmd(&mut mutable_buffer)?,
            _ => (),
        }
        let buffer = Buffer::from_mutable_buffer(&mutable_buffer)?;
        buffer.copy_to(data);
        Ok(())
    }
    pub fn set_target_model_name(&self, value: &str) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let name = self.unicode_string_factory.to_unicode_string(value)?;
        unsafe {
            nanoemMutableMotionSetTargetModelName(self.opaque, name.value, status_ptr);
        };
        status.unit()
    }
    pub fn sort_all_keyframes(&self) {
        unsafe { nanoemMutableMotionSortAllKeyframes(self.opaque) }
    }
    pub fn find_accessory_keyframe(
        &self,
        frame_index: u32,
    ) -> Result<Option<MutableAccessoryKeyframe>> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe {
            nanoemMutableAccessoryKeyframeCreateByFound(
                nanoemMutableMotionGetOriginObject(self.opaque),
                frame_index,
                status_ptr,
            )
        };
        if opaque.is_null() {
            return status.value(None);
        }
        status.value(Some(MutableAccessoryKeyframe {
            unicode_string_factory: &self.unicode_string_factory,
            opaque,
        }))
    }
    pub fn create_accessory_keyframe(&self) -> Result<MutableAccessoryKeyframe> {
        MutableAccessoryKeyframe::new(&self)
    }
    pub fn find_bone_keyframe(
        &self,
        name: &str,
        frame_index: u32,
    ) -> Result<Option<MutableBoneKeyframe>> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let name = self.unicode_string_factory.to_unicode_string(name)?;
        let opaque = unsafe {
            nanoemMutableModelBoneKeyframeCreateByFound(
                nanoemMutableMotionGetOriginObject(self.opaque),
                name.value,
                frame_index,
                status_ptr,
            )
        };
        if opaque.is_null() {
            return status.value(None);
        }
        status.value(Some(MutableBoneKeyframe {
            unicode_string_factory: &self.unicode_string_factory,
            opaque,
        }))
    }
    pub fn create_bone_keyframe(&self) -> Result<MutableBoneKeyframe> {
        MutableBoneKeyframe::new(&self)
    }
    pub fn find_camera_keyframe(&self, frame_index: u32) -> Result<Option<MutableCameraKeyframe>> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe {
            nanoemMutableCameraKeyframeCreateByFound(
                nanoemMutableMotionGetOriginObject(self.opaque),
                frame_index,
                status_ptr,
            )
        };
        if opaque.is_null() {
            return status.value(None);
        }
        status.value(Some(MutableCameraKeyframe {
            unicode_string_factory: &self.unicode_string_factory,
            opaque,
        }))
    }
    pub fn create_camera_keyframe(&self) -> Result<MutableCameraKeyframe> {
        MutableCameraKeyframe::new(&self)
    }
    pub fn find_light_keyframe(&self, frame_index: u32) -> Result<Option<MutableLightKeyframe>> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe {
            nanoemMutableLightKeyframeCreateByFound(
                nanoemMutableMotionGetOriginObject(self.opaque),
                frame_index,
                status_ptr,
            )
        };
        if opaque.is_null() {
            return status.value(None);
        }
        status.value(Some(MutableLightKeyframe { opaque }))
    }
    pub fn create_light_keyframe(&self) -> Result<MutableLightKeyframe> {
        MutableLightKeyframe::new(&self)
    }
    pub fn find_model_keyframe(&self, frame_index: u32) -> Result<Option<MutableModelKeyframe>> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe {
            nanoemMutableModelKeyframeCreateByFound(
                nanoemMutableMotionGetOriginObject(self.opaque),
                frame_index,
                status_ptr,
            )
        };
        if opaque.is_null() {
            return status.value(None);
        }
        status.value(Some(MutableModelKeyframe {
            unicode_string_factory: &self.unicode_string_factory,
            opaque,
        }))
    }
    pub fn create_model_keyframe(&self) -> Result<MutableModelKeyframe> {
        MutableModelKeyframe::new(&self)
    }
    pub fn find_morph_keyframe(
        &self,
        name: &str,
        frame_index: u32,
    ) -> Result<Option<MutableMorphKeyframe>> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let name = self.unicode_string_factory.to_unicode_string(name)?;
        let opaque = unsafe {
            nanoemMutableModelMorphKeyframeCreateByFound(
                nanoemMutableMotionGetOriginObject(self.opaque),
                name.value,
                frame_index,
                status_ptr,
            )
        };
        if opaque.is_null() {
            return status.value(None);
        }
        status.value(Some(MutableMorphKeyframe {
            unicode_string_factory: &self.unicode_string_factory,
            opaque,
        }))
    }
    pub fn create_morph_keyframe(&self) -> Result<MutableMorphKeyframe> {
        MutableMorphKeyframe::new(&self)
    }
    pub fn find_self_shadow_keyframe(
        &self,
        frame_index: u32,
    ) -> Result<Option<MutableSelfShadowKeyframe>> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe {
            nanoemMutableSelfShadowKeyframeCreateByFound(
                nanoemMutableMotionGetOriginObject(self.opaque),
                frame_index,
                status_ptr,
            )
        };
        if opaque.is_null() {
            return status.value(None);
        }
        status.value(Some(MutableSelfShadowKeyframe { opaque }))
    }
    pub fn create_self_shadow_keyframe(&self) -> Result<MutableSelfShadowKeyframe> {
        MutableSelfShadowKeyframe::new(&self)
    }
    pub fn add_accessory_keyframe(
        &self,
        keyframe: MutableAccessoryKeyframe,
        frame_index: u32,
    ) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableMotionAddAccessoryKeyframe(
                self.opaque,
                keyframe.opaque,
                frame_index,
                status_ptr,
            )
        };
        status.unit()
    }
    pub fn set_accessory_keyframe(
        &self,
        keyframe: MutableAccessoryKeyframe,
        frame_index: u32,
    ) -> Result<()> {
        match self.find_accessory_keyframe(frame_index)? {
            Some(value) => value.copy_from(&keyframe),
            None => self.add_accessory_keyframe(keyframe, frame_index),
        }
    }
    pub fn add_bone_keyframe(
        &self,
        keyframe: MutableBoneKeyframe,
        name: &str,
        frame_index: u32,
    ) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let name = self.unicode_string_factory.to_unicode_string(name)?;
        unsafe {
            nanoemMutableMotionAddBoneKeyframe(
                self.opaque,
                keyframe.opaque,
                name.value,
                frame_index,
                status_ptr,
            )
        };
        status.unit()
    }
    pub fn set_bone_keyframe(
        &self,
        keyframe: MutableBoneKeyframe,
        name: &str,
        frame_index: u32,
    ) -> Result<()> {
        match self.find_bone_keyframe(name, frame_index)? {
            Some(value) => {
                value.copy_from(&keyframe);
                Ok(())
            }
            None => self.add_bone_keyframe(keyframe, name, frame_index),
        }
    }
    pub fn add_camera_keyframe(
        &self,
        keyframe: MutableCameraKeyframe,
        frame_index: u32,
    ) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableMotionAddCameraKeyframe(
                self.opaque,
                keyframe.opaque,
                frame_index,
                status_ptr,
            )
        };
        status.unit()
    }
    pub fn set_camera_keyframe(
        &self,
        keyframe: MutableCameraKeyframe,
        frame_index: u32,
    ) -> Result<()> {
        match self.find_camera_keyframe(frame_index)? {
            Some(value) => {
                value.copy_from(&keyframe);
                Ok(())
            }
            None => self.add_camera_keyframe(keyframe, frame_index),
        }
    }
    pub fn add_light_keyframe(
        &self,
        keyframe: MutableLightKeyframe,
        frame_index: u32,
    ) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableMotionAddLightKeyframe(
                self.opaque,
                keyframe.opaque,
                frame_index,
                status_ptr,
            )
        };
        status.unit()
    }
    pub fn set_light_keyframe(
        &self,
        keyframe: MutableLightKeyframe,
        frame_index: u32,
    ) -> Result<()> {
        match self.find_light_keyframe(frame_index)? {
            Some(value) => {
                value.copy_from(&keyframe);
                Ok(())
            }
            None => self.add_light_keyframe(keyframe, frame_index),
        }
    }
    pub fn add_model_keyframe(
        &self,
        keyframe: MutableModelKeyframe,
        frame_index: u32,
    ) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableMotionAddModelKeyframe(
                self.opaque,
                keyframe.opaque,
                frame_index,
                status_ptr,
            )
        };
        status.unit()
    }
    pub fn set_model_keyframe(
        &self,
        keyframe: MutableModelKeyframe,
        frame_index: u32,
    ) -> Result<()> {
        match self.find_model_keyframe(frame_index)? {
            Some(value) => value.copy_from(&keyframe),
            None => self.add_model_keyframe(keyframe, frame_index),
        }
    }
    pub fn add_morph_keyframe(
        &self,
        keyframe: MutableMorphKeyframe,
        name: &str,
        frame_index: u32,
    ) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let name = self.unicode_string_factory.to_unicode_string(name)?;
        unsafe {
            nanoemMutableMotionAddMorphKeyframe(
                self.opaque,
                keyframe.opaque,
                name.value,
                frame_index,
                status_ptr,
            )
        };
        status.unit()
    }
    pub fn set_morph_keyframe(
        &self,
        keyframe: MutableMorphKeyframe,
        name: &str,
        frame_index: u32,
    ) -> Result<()> {
        match self.find_morph_keyframe(name, frame_index)? {
            Some(value) => {
                value.copy_from(&keyframe);
                Ok(())
            }
            None => self.add_morph_keyframe(keyframe, name, frame_index),
        }
    }
    pub fn add_self_shadow_keyframe(
        &self,
        keyframe: MutableSelfShadowKeyframe,
        frame_index: u32,
    ) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableMotionAddSelfShadowKeyframe(
                self.opaque,
                keyframe.opaque,
                frame_index,
                status_ptr,
            )
        };
        status.unit()
    }
    pub fn set_self_shadow_keyframe(
        &self,
        keyframe: MutableSelfShadowKeyframe,
        frame_index: u32,
    ) -> Result<()> {
        match self.find_self_shadow_keyframe(frame_index)? {
            Some(value) => {
                value.copy_from(&keyframe);
                Ok(())
            }
            None => self.add_self_shadow_keyframe(keyframe, frame_index),
        }
    }
    pub fn remove_bone_keyframe(&self, keyframe: MutableBoneKeyframe) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe { nanoemMutableMotionRemoveBoneKeyframe(self.opaque, keyframe.opaque, status_ptr) };
        status.unit()
    }
    pub fn remove_accessory_keyframe(&self, keyframe: MutableAccessoryKeyframe) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableMotionRemoveAccessoryKeyframe(self.opaque, keyframe.opaque, status_ptr)
        };
        status.unit()
    }
    pub fn remove_camera_keyframe(&self, keyframe: MutableCameraKeyframe) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableMotionRemoveCameraKeyframe(self.opaque, keyframe.opaque, status_ptr)
        };
        status.unit()
    }
    pub fn remove_light_keyframe(&self, keyframe: MutableLightKeyframe) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe { nanoemMutableMotionRemoveLightKeyframe(self.opaque, keyframe.opaque, status_ptr) };
        status.unit()
    }
    pub fn remove_model_keyframe(&self, keyframe: MutableModelKeyframe) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe { nanoemMutableMotionRemoveModelKeyframe(self.opaque, keyframe.opaque, status_ptr) };
        status.unit()
    }
    pub fn remove_morph_keyframe(&self, keyframe: MutableMorphKeyframe) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe { nanoemMutableMotionRemoveMorphKeyframe(self.opaque, keyframe.opaque, status_ptr) };
        status.unit()
    }
    pub fn remove_self_shadow_keyframe(&self, keyframe: MutableSelfShadowKeyframe) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableMotionRemoveSelfShadowKeyframe(self.opaque, keyframe.opaque, status_ptr)
        };
        status.unit()
    }
    fn save_buffer_as_nmd(&self, buffer: &mut MutableBuffer) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableMotionSaveToBufferNMD(self.opaque, buffer.opaque, status_ptr);
        };
        status.unit()
    }
    fn save_buffer_as_vmd(&self, buffer: &mut MutableBuffer) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableMotionSaveToBuffer(self.opaque, buffer.opaque, status_ptr);
        };
        status.unit()
    }
}

impl Drop for MutableMotion {
    fn drop(&mut self) {
        unsafe {
            nanoemMutableMotionDestroy(self.opaque);
        }
    }
}

pub struct MutableEffectParameter<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *mut nanoem_mutable_effect_parameter_t,
}

impl<'a> MutableEffectParameter<'a> {
    pub fn new(
        factory: &'a UnicodeStringFactory,
        opaque: *mut nanoem_mutable_effect_parameter_t,
    ) -> Self {
        Self {
            unicode_string_factory: factory,
            opaque,
        }
    }
    pub(crate) unsafe fn from_ptr(
        factory: &'a UnicodeStringFactory,
        opaque: *mut nanoem_effect_parameter_t,
    ) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = nanoemMutableEffectParameterCreateAsReference(opaque, status_ptr);
        status.closure(|| Self::new(factory, opaque))
    }
    pub fn as_origin(&self) -> EffectParameter {
        EffectParameter::new(self.unicode_string_factory, unsafe {
            nanoemMutableEffectParameterGetOriginObject(self.opaque)
        })
    }
    pub fn set_name(&self, value: &str) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let name = self.unicode_string_factory.to_unicode_string(value)?;
        unsafe { nanoemMutableEffectParameterSetName(self.opaque, name.value, status_ptr) };
        status.unit()
    }
    pub fn set_value(&self, value: EffectParameterVariant) {
        unsafe {
            match value {
                EffectParameterVariant::Bool(_value) => {
                    nanoemMutableEffectParameterSetType(
                        self.opaque,
                        nanoem_effect_parameter_type_t::BOOL,
                    );
                }
                EffectParameterVariant::Float32(_value) => {
                    nanoemMutableEffectParameterSetType(
                        self.opaque,
                        nanoem_effect_parameter_type_t::FLOAT,
                    );
                }
                EffectParameterVariant::Int32(_value) => {
                    nanoemMutableEffectParameterSetType(
                        self.opaque,
                        nanoem_effect_parameter_type_t::INT,
                    );
                }
                EffectParameterVariant::Vector4(_x, _y, _z, _w) => {
                    nanoemMutableEffectParameterSetType(
                        self.opaque,
                        nanoem_effect_parameter_type_t::VECTOR4,
                    );
                }
            }
        }
    }
}

impl<'a> Drop for MutableEffectParameter<'a> {
    fn drop(&mut self) {
        unsafe {
            nanoemMutableEffectParameterDestroy(self.opaque);
        }
    }
}

pub struct MutableOutsideParent<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *mut nanoem_mutable_outside_parent_t,
}

impl<'a> MutableOutsideParent<'a> {
    pub fn new(
        factory: &'a UnicodeStringFactory,
        opaque: *mut nanoem_mutable_outside_parent_t,
    ) -> Self {
        Self {
            unicode_string_factory: factory,
            opaque,
        }
    }
    pub(crate) unsafe fn from_ptr(
        factory: &'a UnicodeStringFactory,
        opaque: *mut nanoem_outside_parent_t,
    ) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = nanoemMutableOutsideParentCreateAsReference(opaque, status_ptr);
        status.closure(|| Self::new(factory, opaque))
    }
    pub fn as_origin(&self) -> OutsideParent {
        OutsideParent::new(self.unicode_string_factory, unsafe {
            nanoemMutableOutsideParentGetOriginObject(self.opaque)
        })
    }
    pub fn set_subject_bone_name(&self, value: &str) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let name = self.unicode_string_factory.to_unicode_string(value)?;
        unsafe {
            nanoemMutableOutsideParentSetSubjectBoneName(self.opaque, name.value, status_ptr)
        };
        status.unit()
    }
    pub fn set_target_object_name(&self, value: &str) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let name = self.unicode_string_factory.to_unicode_string(value)?;
        unsafe {
            nanoemMutableOutsideParentSetTargetObjectName(self.opaque, name.value, status_ptr)
        };
        status.unit()
    }
    pub fn set_target_bone_name(&self, value: &str) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let name = self.unicode_string_factory.to_unicode_string(value)?;
        unsafe { nanoemMutableOutsideParentSetTargetBoneName(self.opaque, name.value, status_ptr) };
        status.unit()
    }
}

impl<'a> Drop for MutableOutsideParent<'a> {
    fn drop(&mut self) {
        unsafe {
            nanoemMutableOutsideParentDestroy(self.opaque);
        }
    }
}

pub struct MutableAccessoryKeyframe<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *mut nanoem_mutable_accessory_keyframe_t,
}

impl<'a> MutableAccessoryKeyframe<'a> {
    pub fn new(motion: &'a MutableMotion) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe {
            nanoemMutableAccessoryKeyframeCreate(
                nanoemMutableMotionGetOriginObject(motion.opaque),
                status_ptr,
            )
        };
        status.closure(|| Self {
            unicode_string_factory: &motion.unicode_string_factory,
            opaque,
        })
    }
    #[allow(dead_code)]
    pub(crate) unsafe fn from_ptr(
        factory: &'a UnicodeStringFactory,
        opaque: *mut nanoem_accessory_keyframe_t,
    ) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = nanoemMutableAccessoryKeyframeCreateAsReference(opaque, status_ptr);
        status.closure(|| Self {
            unicode_string_factory: factory,
            opaque,
        })
    }
    pub fn as_origin(&self) -> AccessoryKeyframe {
        AccessoryKeyframe::new(&self.unicode_string_factory, unsafe {
            nanoemMutableAccessoryKeyframeGetOriginObject(self.opaque)
        })
    }
    pub fn create_outside_parent(&self) -> Result<MutableOutsideParent> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe {
            nanoemMutableOutsideParentCreateFromAccessoryKeyframe(
                nanoemMutableAccessoryKeyframeGetOriginObject(self.opaque),
                status_ptr,
            )
        };
        status.closure(|| MutableOutsideParent::new(&self.unicode_string_factory, opaque))
    }
    pub fn set_outside_parent(&self, value: Option<MutableOutsideParent>) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableAccessoryKeyframeSetOutsideParent(
                self.opaque,
                match value {
                    Some(value) => value.opaque,
                    None => std::ptr::null_mut(),
                },
                status_ptr,
            )
        };
        status.unit()
    }
    pub fn find_effect_parameter(&self, value: i32) -> Result<MutableEffectParameter> {
        let mut num_objects = 0usize;
        let ptr = unsafe {
            nanoemAccessoryKeyframeGetAllEffectParameterObjects(
                nanoemMutableAccessoryKeyframeGetOriginObject(self.opaque),
                &mut num_objects as *mut usize,
            )
        };
        let offset = value as usize;
        if offset < num_objects {
            Ok(unsafe {
                MutableEffectParameter::from_ptr(&self.unicode_string_factory, *(ptr.add(offset)))?
            })
        } else {
            Err(Status::null_object())
        }
    }
    pub fn create_effect_parameter(&self) -> Result<MutableEffectParameter> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe {
            nanoemMutableEffectParameterCreateFromAccessoryKeyframe(
                nanoemMutableAccessoryKeyframeGetOriginObject(self.opaque),
                status_ptr,
            )
        };
        status.closure(|| MutableEffectParameter::new(&self.unicode_string_factory, opaque))
    }
    pub fn add_effect_parameter(&self, value: MutableEffectParameter) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableAccessoryKeyframeAddEffectParameter(self.opaque, value.opaque, status_ptr)
        }
        status.unit()
    }
    pub fn remove_effect_parameter(&self, value: MutableEffectParameter) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableAccessoryKeyframeRemoveEffectParameter(
                self.opaque,
                value.opaque,
                status_ptr,
            )
        }
        status.unit()
    }
    pub fn set_translation(&self, value: Vector4) {
        unsafe { nanoemMutableAccessoryKeyframeSetTranslation(self.opaque, value.as_ptr()) }
    }
    pub fn set_orientation(&self, value: Vector4) {
        unsafe { nanoemMutableAccessoryKeyframeSetOrientation(self.opaque, value.as_ptr()) }
    }
    pub fn set_scale_factor(&self, value: f32) {
        unsafe { nanoemMutableAccessoryKeyframeSetScaleFactor(self.opaque, value) }
    }
    pub fn set_opacity(&self, value: f32) {
        unsafe { nanoemMutableAccessoryKeyframeSetOpacity(self.opaque, value) }
    }
    pub fn set_add_blend_enabled(&self, value: bool) {
        unsafe { nanoemMutableAccessoryKeyframeSetAddBlendEnabled(self.opaque, value as i32) }
    }
    pub fn set_visible(&self, value: bool) {
        unsafe { nanoemMutableAccessoryKeyframeSetVisible(self.opaque, value as i32) }
    }
    pub fn set_shadow_enabled(&self, value: bool) {
        unsafe { nanoemMutableAccessoryKeyframeSetShadowEnabled(self.opaque, value as i32) }
    }
    pub fn copy_from(&self, value: &MutableAccessoryKeyframe) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableAccessoryKeyframeCopy(
                self.opaque,
                nanoemMutableAccessoryKeyframeGetOriginObject(value.opaque),
                status_ptr,
            )
        }
        status.unit()
    }
}

impl<'a> Drop for MutableAccessoryKeyframe<'a> {
    fn drop(&mut self) {
        unsafe { nanoemMutableAccessoryKeyframeDestroy(self.opaque) }
    }
}

pub struct MutableBoneKeyframe<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *mut nanoem_mutable_model_bone_keyframe_t,
}

impl<'a> MutableBoneKeyframe<'a> {
    pub fn new(motion: &'a MutableMotion) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe {
            nanoemMutableModelBoneKeyframeCreate(
                nanoemMutableMotionGetOriginObject(motion.opaque),
                status_ptr,
            )
        };
        status.closure(|| Self {
            unicode_string_factory: &motion.unicode_string_factory,
            opaque,
        })
    }
    #[allow(dead_code)]
    pub(crate) unsafe fn from_ptr(
        factory: &'a UnicodeStringFactory,
        opaque: *mut nanoem_model_bone_keyframe_t,
    ) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = nanoemMutableModelBoneKeyframeCreateAsReference(opaque, status_ptr);
        status.closure(|| Self {
            unicode_string_factory: factory,
            opaque,
        })
    }
    pub fn as_origin(&self) -> BoneKeyframe {
        BoneKeyframe::new(&self.unicode_string_factory, unsafe {
            nanoemMutableModelBoneKeyframeGetOriginObject(self.opaque)
        })
    }
    pub fn set_translation(&self, value: Vector4) {
        unsafe { nanoemMutableModelBoneKeyframeSetTranslation(self.opaque, value.as_ptr()) }
    }
    pub fn set_orientation(&self, value: Vector4) {
        unsafe { nanoemMutableModelBoneKeyframeSetOrientation(self.opaque, value.as_ptr()) }
    }
    pub fn set_interpolation(&self, value: Interpolation, t: BoneKeyframeInterpolationType) {
        unsafe {
            nanoemMutableModelBoneKeyframeSetInterpolation(self.opaque, t.cast(), value.as_ptr())
        }
    }
    pub fn set_stage_index(&self, value: u32) {
        unsafe { nanoemMutableModelBoneKeyframeSetStageIndex(self.opaque, value) }
    }
    pub fn set_physics_simulation_enabled(&self, value: bool) {
        unsafe {
            nanoemMutableModelBoneKeyframeSetPhysicsSimulationEnabled(self.opaque, value as i32)
        }
    }
    pub fn copy_from(&self, value: &MutableBoneKeyframe) {
        unsafe {
            nanoemMutableModelBoneKeyframeCopy(
                self.opaque,
                nanoemMutableModelBoneKeyframeGetOriginObject(value.opaque),
            )
        }
    }
}

impl<'a> Drop for MutableBoneKeyframe<'a> {
    fn drop(&mut self) {
        unsafe { nanoemMutableModelBoneKeyframeDestroy(self.opaque) }
    }
}

pub struct MutableCameraKeyframe<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *mut nanoem_mutable_camera_keyframe_t,
}

impl<'a> MutableCameraKeyframe<'a> {
    pub fn new(motion: &'a MutableMotion) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe {
            nanoemMutableCameraKeyframeCreate(
                nanoemMutableMotionGetOriginObject(motion.opaque),
                status_ptr,
            )
        };
        status.closure(|| Self {
            unicode_string_factory: &motion.unicode_string_factory,
            opaque,
        })
    }
    #[allow(dead_code)]
    pub(crate) unsafe fn from_ptr(
        factory: &'a UnicodeStringFactory,
        opaque: *mut nanoem_camera_keyframe_t,
    ) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = nanoemMutableCameraKeyframeCreateAsReference(opaque, status_ptr);
        status.closure(|| Self {
            unicode_string_factory: factory,
            opaque,
        })
    }
    pub fn as_origin(&self) -> CameraKeyframe {
        CameraKeyframe::new(&self.unicode_string_factory, unsafe {
            nanoemMutableCameraKeyframeGetOriginObject(self.opaque)
        })
    }
    pub fn create_outside_parent(&self) -> Result<MutableOutsideParent> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe {
            nanoemMutableOutsideParentCreateFromCameraKeyframe(
                nanoemMutableCameraKeyframeGetOriginObject(self.opaque),
                status_ptr,
            )
        };
        status.closure(|| MutableOutsideParent::new(&self.unicode_string_factory, opaque))
    }
    pub fn set_outside_parent(&self, value: Option<MutableOutsideParent>) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableCameraKeyframeSetOutsideParent(
                self.opaque,
                match value {
                    Some(value) => value.opaque,
                    None => std::ptr::null_mut(),
                },
                status_ptr,
            )
        };
        status.unit()
    }
    pub fn set_look_at(&self, value: Vector4) {
        unsafe { nanoemMutableCameraKeyframeSetLookAt(self.opaque, value.as_ptr()) }
    }
    pub fn set_angle(&self, value: Vector4) {
        unsafe { nanoemMutableCameraKeyframeSetAngle(self.opaque, value.as_ptr()) }
    }
    pub fn set_interpolation(&self, value: Interpolation, t: CameraKeyframeInterpolationType) {
        unsafe {
            nanoemMutableCameraKeyframeSetInterpolation(self.opaque, t.cast(), value.as_ptr())
        }
    }
    pub fn set_fov(&self, value: i32) {
        unsafe { nanoemMutableCameraKeyframeSetFov(self.opaque, value) }
    }
    pub fn set_distance(&self, value: f32) {
        unsafe { nanoemMutableCameraKeyframeSetDistance(self.opaque, value) }
    }
    pub fn set_stage_index(&self, value: u32) {
        unsafe { nanoemMutableCameraKeyframeSetStageIndex(self.opaque, value) }
    }
    pub fn set_perspective(&self, value: bool) {
        unsafe { nanoemMutableCameraKeyframeSetPerspectiveView(self.opaque, value as i32) }
    }
    pub fn copy_from(&self, value: &MutableCameraKeyframe) {
        unsafe {
            nanoemMutableCameraKeyframeCopy(
                self.opaque,
                nanoemMutableCameraKeyframeGetOriginObject(value.opaque),
            )
        }
    }
}

impl<'a> Drop for MutableCameraKeyframe<'a> {
    fn drop(&mut self) {
        unsafe { nanoemMutableCameraKeyframeDestroy(self.opaque) }
    }
}

pub struct MutableLightKeyframe {
    opaque: *mut nanoem_mutable_light_keyframe_t,
}

impl<'a> MutableLightKeyframe {
    pub fn new(motion: &'a MutableMotion) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe {
            nanoemMutableLightKeyframeCreate(
                nanoemMutableMotionGetOriginObject(motion.opaque),
                status_ptr,
            )
        };
        status.closure(|| Self { opaque })
    }
    #[allow(dead_code)]
    pub(crate) unsafe fn from_ptr(opaque: *mut nanoem_light_keyframe_t) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = nanoemMutableLightKeyframeCreateAsReference(opaque, status_ptr);
        status.closure(|| Self { opaque })
    }
    pub fn as_origin(&self) -> LightKeyframe {
        LightKeyframe::new(unsafe { nanoemMutableLightKeyframeGetOriginObject(self.opaque) })
    }
    pub fn set_color(&self, value: Vector4) {
        unsafe { nanoemMutableLightKeyframeSetColor(self.opaque, value.as_ptr()) }
    }
    pub fn set_direction(&self, value: Vector4) {
        unsafe { nanoemMutableLightKeyframeSetDirection(self.opaque, value.as_ptr()) }
    }
    pub fn copy_from(&self, value: &MutableLightKeyframe) {
        unsafe {
            nanoemMutableLightKeyframeCopy(
                self.opaque,
                nanoemMutableLightKeyframeGetOriginObject(value.opaque),
            )
        }
    }
}

impl Drop for MutableLightKeyframe {
    fn drop(&mut self) {
        unsafe { nanoemMutableLightKeyframeDestroy(self.opaque) }
    }
}

pub struct MutableModelKeyframeConstraintState<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *mut nanoem_mutable_model_keyframe_constraint_state_t,
}

impl<'a> MutableModelKeyframeConstraintState<'a> {
    pub fn new(keyframe: &'a MutableModelKeyframe) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe {
            nanoemMutableModelKeyframeConstraintStateCreate(
                nanoemMutableModelKeyframeGetOriginObject(keyframe.opaque),
                status_ptr,
            )
        };
        status.closure(|| Self {
            unicode_string_factory: keyframe.unicode_string_factory,
            opaque,
        })
    }
    pub(crate) unsafe fn from_ptr(
        factory: &'a UnicodeStringFactory,
        opaque: *mut nanoem_model_keyframe_constraint_state_t,
    ) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = nanoemMutableModelKeyframeConstraintStateCreateAsReference(opaque, status_ptr);
        status.closure(|| Self {
            unicode_string_factory: factory,
            opaque,
        })
    }
    pub fn as_origin(&self) -> ModelKeyframeConstraintState {
        ModelKeyframeConstraintState::new(&self.unicode_string_factory, unsafe {
            nanoemMutableModelKeyframeConstraintStateGetOriginObject(self.opaque)
        })
    }
    pub fn set_name(&self, value: &str) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let name = self.unicode_string_factory.to_unicode_string(value)?;
        unsafe {
            nanoemMutableModelKeyframeConstraintStateSetBoneName(
                self.opaque,
                name.value,
                status_ptr,
            );
        };
        status.unit()
    }
    pub fn set_enabled(&self, value: bool) {
        unsafe { nanoemMutableModelKeyframeConstraintStateSetEnabled(self.opaque, value as i32) }
    }
}

impl<'a> Drop for MutableModelKeyframeConstraintState<'a> {
    fn drop(&mut self) {
        unsafe { nanoemMutableModelKeyframeConstraintStateDestroy(self.opaque) }
    }
}

pub struct MutableModelKeyframe<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *mut nanoem_mutable_model_keyframe_t,
}

impl<'a> MutableModelKeyframe<'a> {
    pub fn new(motion: &'a MutableMotion) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe {
            nanoemMutableModelKeyframeCreate(
                nanoemMutableMotionGetOriginObject(motion.opaque),
                status_ptr,
            )
        };
        status.closure(|| Self {
            unicode_string_factory: &motion.unicode_string_factory,
            opaque,
        })
    }
    #[allow(dead_code)]
    pub(crate) unsafe fn from_ptr(
        factory: &'a UnicodeStringFactory,
        opaque: *mut nanoem_model_keyframe_t,
    ) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = nanoemMutableModelKeyframeCreateAsReference(opaque, status_ptr);
        status.closure(|| Self {
            unicode_string_factory: factory,
            opaque,
        })
    }
    pub fn as_origin(&self) -> ModelKeyframe {
        ModelKeyframe::new(&self.unicode_string_factory, unsafe {
            nanoemMutableModelKeyframeGetOriginObject(self.opaque)
        })
    }
    pub fn set_edge_color(&self, value: Vector4) {
        unsafe { nanoemMutableModelKeyframeSetEdgeColor(self.opaque, value.as_ptr()) }
    }
    pub fn set_edge_scale_factor(&self, value: f32) {
        unsafe { nanoemMutableModelKeyframeSetEdgeScaleFactor(self.opaque, value) }
    }
    pub fn set_add_blend_enabled(&self, value: bool) {
        unsafe { nanoemMutableModelKeyframeSetAddBlendEnabled(self.opaque, value as i32) }
    }
    pub fn set_physics_simulation_enabled(&self, value: bool) {
        unsafe { nanoemMutableModelKeyframeSetPhysicsSimulationEnabled(self.opaque, value as i32) }
    }
    pub fn set_visible(&self, value: bool) {
        unsafe { nanoemMutableModelKeyframeSetVisible(self.opaque, value as i32) }
    }
    pub fn find_constraint_state(&self, value: i32) -> Result<MutableModelKeyframeConstraintState> {
        let mut num_objects = 0usize;
        let ptr = unsafe {
            nanoemModelKeyframeGetAllConstraintStateObjects(
                nanoemMutableModelKeyframeGetOriginObject(self.opaque),
                &mut num_objects as *mut usize,
            )
        };
        let offset = value as usize;
        if offset < num_objects {
            Ok(unsafe {
                MutableModelKeyframeConstraintState::from_ptr(
                    &self.unicode_string_factory,
                    *(ptr.add(offset)),
                )?
            })
        } else {
            Err(Status::null_object())
        }
    }
    pub fn create_constraint_state(&self) -> Result<MutableModelKeyframeConstraintState> {
        MutableModelKeyframeConstraintState::new(&self)
    }
    pub fn add_constraint_state(&self, value: MutableModelKeyframeConstraintState) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableModelKeyframeAddConstraintState(self.opaque, value.opaque, status_ptr)
        }
        status.unit()
    }
    pub fn remove_constraint_state(
        &self,
        value: MutableModelKeyframeConstraintState,
    ) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableModelKeyframeRemoveConstraintState(self.opaque, value.opaque, status_ptr)
        }
        status.unit()
    }
    pub fn find_effect_parameter(&self, value: i32) -> Result<MutableEffectParameter> {
        let mut num_objects = 0usize;
        let ptr = unsafe {
            nanoemModelKeyframeGetAllEffectParameterObjects(
                nanoemMutableModelKeyframeGetOriginObject(self.opaque),
                &mut num_objects as *mut usize,
            )
        };
        let offset = value as usize;
        if offset < num_objects {
            Ok(unsafe {
                MutableEffectParameter::from_ptr(&self.unicode_string_factory, *(ptr.add(offset)))?
            })
        } else {
            Err(Status::null_object())
        }
    }
    pub fn create_effect_parameter(&self) -> Result<MutableEffectParameter> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe {
            nanoemMutableEffectParameterCreateFromModelKeyframe(
                nanoemMutableModelKeyframeGetOriginObject(self.opaque),
                status_ptr,
            )
        };
        status.closure(|| MutableEffectParameter::new(&self.unicode_string_factory, opaque))
    }
    pub fn add_effect_parameter(&self, value: MutableEffectParameter) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableModelKeyframeAddEffectParameter(self.opaque, value.opaque, status_ptr)
        }
        status.unit()
    }
    pub fn remove_effect_parameter(&self, value: MutableEffectParameter) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableModelKeyframeRemoveEffectParameter(self.opaque, value.opaque, status_ptr)
        }
        status.unit()
    }
    pub fn find_outside_parent(&self, value: i32) -> Result<MutableOutsideParent> {
        let mut num_objects = 0usize;
        let ptr = unsafe {
            nanoemModelKeyframeGetAllOutsideParentObjects(
                nanoemMutableModelKeyframeGetOriginObject(self.opaque),
                &mut num_objects as *mut usize,
            )
        };
        let offset = value as usize;
        if offset < num_objects {
            Ok(unsafe {
                MutableOutsideParent::from_ptr(&self.unicode_string_factory, *(ptr.add(offset)))?
            })
        } else {
            Err(Status::null_object())
        }
    }
    pub fn create_outside_parent(&self) -> Result<MutableOutsideParent> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe {
            nanoemMutableOutsideParentCreateFromModelKeyframe(
                nanoemMutableModelKeyframeGetOriginObject(self.opaque),
                status_ptr,
            )
        };
        status.closure(|| MutableOutsideParent::new(&self.unicode_string_factory, opaque))
    }
    pub fn add_outside_parent(&self, value: MutableOutsideParent) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe { nanoemMutableModelKeyframeAddOutsideParent(self.opaque, value.opaque, status_ptr) }
        status.unit()
    }
    pub fn remove_outside_parent(&self, value: MutableOutsideParent) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableModelKeyframeRemoveOutsideParent(self.opaque, value.opaque, status_ptr)
        }
        status.unit()
    }
    pub fn copy_from(&self, value: &MutableModelKeyframe) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableModelKeyframeCopy(
                self.opaque,
                nanoemMutableModelKeyframeGetOriginObject(value.opaque),
                status_ptr,
            )
        }
        status.unit()
    }
}

impl<'a> Drop for MutableModelKeyframe<'a> {
    fn drop(&mut self) {
        unsafe { nanoemMutableModelKeyframeDestroy(self.opaque) }
    }
}

pub struct MutableMorphKeyframe<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *mut nanoem_mutable_model_morph_keyframe_t,
}

impl<'a> MutableMorphKeyframe<'a> {
    pub fn new(motion: &'a MutableMotion) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe {
            nanoemMutableModelMorphKeyframeCreate(
                nanoemMutableMotionGetOriginObject(motion.opaque),
                status_ptr,
            )
        };
        status.closure(|| Self {
            unicode_string_factory: &motion.unicode_string_factory,
            opaque,
        })
    }
    #[allow(dead_code)]
    pub(crate) unsafe fn from_ptr(
        factory: &'a UnicodeStringFactory,
        opaque: *mut nanoem_model_morph_keyframe_t,
    ) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = nanoemMutableModelMorphKeyframeCreateAsReference(opaque, status_ptr);
        status.closure(|| Self {
            unicode_string_factory: factory,
            opaque,
        })
    }
    pub fn as_origin(&self) -> MorphKeyframe {
        MorphKeyframe::new(&self.unicode_string_factory, unsafe {
            nanoemMutableModelMorphKeyframeGetOriginObject(self.opaque)
        })
    }
    pub fn set_weight(&self, value: f32) {
        unsafe { nanoemMutableModelMorphKeyframeSetWeight(self.opaque, value) }
    }
    pub fn copy_from(&self, value: &MutableMorphKeyframe) {
        unsafe {
            nanoemMutableModelMorphKeyframeCopy(
                self.opaque,
                nanoemMutableModelMorphKeyframeGetOriginObject(value.opaque),
            )
        }
    }
}

impl<'a> Drop for MutableMorphKeyframe<'a> {
    fn drop(&mut self) {
        unsafe { nanoemMutableModelMorphKeyframeDestroy(self.opaque) }
    }
}

pub struct MutableSelfShadowKeyframe {
    opaque: *mut nanoem_mutable_self_shadow_keyframe_t,
}

impl<'a> MutableSelfShadowKeyframe {
    pub fn new(motion: &'a MutableMotion) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe {
            nanoemMutableSelfShadowKeyframeCreate(
                nanoemMutableMotionGetOriginObject(motion.opaque),
                status_ptr,
            )
        };
        status.closure(|| Self { opaque })
    }
    #[allow(dead_code)]
    pub(crate) unsafe fn from_ptr(opaque: *mut nanoem_self_shadow_keyframe_t) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = nanoemMutableSelfShadowKeyframeCreateAsReference(opaque, status_ptr);
        status.closure(|| Self { opaque })
    }
    pub fn as_origin(&self) -> SelfShadowKeyframe {
        SelfShadowKeyframe::new(unsafe {
            nanoemMutableSelfShadowKeyframeGetOriginObject(self.opaque)
        })
    }
    pub fn set_distance(&self, value: f32) {
        unsafe { nanoemMutableSelfShadowKeyframeSetDistance(self.opaque, value) }
    }
    pub fn set_mode(&self, value: i32) {
        unsafe { nanoemMutableSelfShadowKeyframeSetMode(self.opaque, value) }
    }
    pub fn copy_from(&self, value: &MutableSelfShadowKeyframe) {
        unsafe {
            nanoemMutableSelfShadowKeyframeCopy(
                self.opaque,
                nanoemMutableSelfShadowKeyframeGetOriginObject(value.opaque),
            )
        }
    }
}

impl Drop for MutableSelfShadowKeyframe {
    fn drop(&mut self) {
        unsafe { nanoemMutableSelfShadowKeyframeDestroy(self.opaque) }
    }
}
