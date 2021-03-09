/*
  Copyright (c) 2015-2020 hkrn All rights reserved

  This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
*/

use super::common::*;
use super::model::*;
use super::native::*;

pub struct MutableModel {
    unicode_string_factory: UnicodeStringFactory,
    opaque: *mut nanoem_mutable_model_t,
}

impl MutableModel {
    #[allow(dead_code)]
    pub fn new(factory: UnicodeStringFactory) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe { nanoemMutableModelCreate(factory.opaque, status_ptr) };
        status.value(Self {
            unicode_string_factory: factory,
            opaque,
        })
    }
    pub fn from_model(model: &Model) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let factory = UnicodeStringFactory::create()?;
        let opaque = unsafe { nanoemMutableModelCreateAsReference(model.opaque, status_ptr) };
        status.value(Self {
            unicode_string_factory: factory,
            opaque,
        })
    }
    pub fn save(&self, data: &mut Vec<u8>) -> Result<()> {
        let mut mutable_buffer = MutableBuffer::new()?;
        self.save_buffer(&mut mutable_buffer)?;
        let buffer = Buffer::from_mutable_buffer(&mutable_buffer)?;
        buffer.copy_to(data);
        Ok(())
    }
    pub fn set_name(&self, value: &str, language: Language) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let name = self.unicode_string_factory.to_unicode_string(value)?;
        unsafe {
            nanoemMutableModelSetName(self.opaque, name.value, language.cast(), status_ptr);
        };
        status.unit()
    }
    pub fn set_comment(&self, value: &str, language: Language) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let name = self.unicode_string_factory.to_unicode_string(value)?;
        unsafe {
            nanoemMutableModelSetComment(self.opaque, name.value, language.cast(), status_ptr);
        };
        status.unit()
    }
    pub fn set_format_type(&self, value: ModelFormatType) {
        unsafe { nanoemMutableModelSetFormatType(self.opaque, value.cast()) }
    }
    pub fn set_codec_type(&self, value: ModelCodecType) {
        unsafe {
            nanoemMutableModelSetCodecType(
                self.opaque,
                match value {
                    ModelCodecType::ShiftJIS => nanoem_codec_type_t::SJIS,
                    ModelCodecType::UTF8 => nanoem_codec_type_t::UTF8,
                    ModelCodecType::UTF16 => nanoem_codec_type_t::UTF16,
                    ModelCodecType::Unknown => nanoem_codec_type_t::UNKNOWN,
                },
            )
        }
    }
    #[allow(dead_code)]
    pub fn set_vertex_indices(&self, value: &[u32]) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableModelSetVertexIndices(self.opaque, value.as_ptr(), value.len(), status_ptr)
        }
        status.unit()
    }
    pub fn find_vertex(&self, value: i32) -> Result<MutableVertex> {
        let mut num_objects = 0usize;
        let ptr = unsafe {
            nanoemModelGetAllVertexObjects(
                nanoemMutableModelGetOriginObject(self.opaque),
                &mut num_objects as *mut usize,
            )
        };
        let offset = value as usize;
        if offset < num_objects {
            Ok(unsafe {
                MutableVertex::from_ptr(&self.unicode_string_factory, *(ptr.add(offset)))?
            })
        } else {
            Err(Status::null_object())
        }
    }
    pub fn create_vertex(&self) -> Result<MutableVertex> {
        MutableVertex::new(&self)
    }
    pub fn insert_vertex(&self, value: MutableVertex, index: i32) -> Result<i32> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let index = unsafe {
            nanoemMutableModelInsertVertexObject(self.opaque, value.opaque, index, status_ptr);
            nanoemModelObjectGetIndex(nanoemModelVertexGetModelObject(
                nanoemMutableModelVertexGetOriginObject(value.opaque),
            ))
        };
        status.value(index)
    }
    pub fn remove_vertex(&self, value: MutableVertex) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe { nanoemMutableModelRemoveVertexObject(self.opaque, value.opaque, status_ptr) }
        status.unit()
    }
    pub fn find_texture(&self, value: i32) -> Result<MutableTexture> {
        let mut num_objects = 0usize;
        let ptr = unsafe {
            nanoemModelGetAllTextureObjects(
                nanoemMutableModelGetOriginObject(self.opaque),
                &mut num_objects as *mut usize,
            )
        };
        let offset = value as usize;
        if offset < num_objects {
            Ok(unsafe {
                MutableTexture::from_ptr(&self.unicode_string_factory, *(ptr.add(offset)))?
            })
        } else {
            Err(Status::null_object())
        }
    }
    pub fn create_texture(&self) -> Result<MutableTexture> {
        MutableTexture::new(&self)
    }
    pub fn insert_texture(&self, value: MutableTexture, index: i32) -> Result<i32> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let index = unsafe {
            nanoemMutableModelInsertTextureObject(self.opaque, value.opaque, index, status_ptr);
            nanoemModelObjectGetIndex(nanoemModelTextureGetModelObject(
                nanoemMutableModelTextureGetOriginObject(value.opaque),
            ))
        };
        status.value(index)
    }
    pub fn remove_texture(&self, value: MutableTexture) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe { nanoemMutableModelRemoveTextureObject(self.opaque, value.opaque, status_ptr) }
        status.unit()
    }
    pub fn find_material(&self, value: i32) -> Result<MutableMaterial> {
        let mut num_objects = 0usize;
        let ptr = unsafe {
            nanoemModelGetAllMaterialObjects(
                nanoemMutableModelGetOriginObject(self.opaque),
                &mut num_objects as *mut usize,
            )
        };
        let offset = value as usize;
        if offset < num_objects {
            Ok(unsafe {
                MutableMaterial::from_ptr(&self.unicode_string_factory, *(ptr.add(offset)))?
            })
        } else {
            Err(Status::null_object())
        }
    }
    pub fn create_material(&self) -> Result<MutableMaterial> {
        MutableMaterial::new(&self)
    }
    pub fn insert_material(&self, value: MutableMaterial, index: i32) -> Result<i32> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let index = unsafe {
            nanoemMutableModelInsertMaterialObject(self.opaque, value.opaque, index, status_ptr);
            nanoemModelObjectGetIndex(nanoemModelMaterialGetModelObject(
                nanoemMutableModelMaterialGetOriginObject(value.opaque),
            ))
        };
        status.value(index)
    }
    pub fn remove_material(&self, value: MutableMaterial) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe { nanoemMutableModelRemoveMaterialObject(self.opaque, value.opaque, status_ptr) }
        status.unit()
    }
    pub fn find_constraint(&self, value: i32) -> Result<MutableConstraint> {
        let mut num_objects = 0usize;
        let ptr = unsafe {
            nanoemModelGetAllConstraintObjects(
                nanoemMutableModelGetOriginObject(self.opaque),
                &mut num_objects as *mut usize,
            )
        };
        let offset = value as usize;
        if offset < num_objects {
            Ok(unsafe {
                MutableConstraint::from_ptr(&self.unicode_string_factory, *(ptr.add(offset)))?
            })
        } else {
            Err(Status::null_object())
        }
    }
    pub fn create_constraint(&self) -> Result<MutableConstraint> {
        MutableConstraint::new(&self)
    }
    pub fn insert_constraint(&self, value: MutableConstraint, index: i32) -> Result<i32> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let index = unsafe {
            nanoemMutableModelInsertConstraintObject(self.opaque, value.opaque, index, status_ptr);
            nanoemModelObjectGetIndex(nanoemModelConstraintGetModelObject(
                nanoemMutableModelConstraintGetOriginObject(value.opaque),
            ))
        };
        status.value(index)
    }
    pub fn remove_constraint(&self, value: MutableConstraint) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe { nanoemMutableModelRemoveConstraintObject(self.opaque, value.opaque, status_ptr) }
        status.unit()
    }
    pub fn find_bone(&self, value: i32) -> Result<MutableBone> {
        let mut num_objects = 0usize;
        let ptr = unsafe {
            nanoemModelGetAllBoneObjects(
                nanoemMutableModelGetOriginObject(self.opaque),
                &mut num_objects as *mut usize,
            )
        };
        let offset = value as usize;
        if offset < num_objects {
            Ok(unsafe { MutableBone::from_ptr(&self.unicode_string_factory, *(ptr.add(offset)))? })
        } else {
            Err(Status::null_object())
        }
    }
    pub fn create_bone(&self) -> Result<MutableBone> {
        MutableBone::new(&self)
    }
    pub fn insert_bone(&self, value: MutableBone, index: i32) -> Result<i32> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let index = unsafe {
            nanoemMutableModelInsertBoneObject(self.opaque, value.opaque, index, status_ptr);
            nanoemModelObjectGetIndex(nanoemModelBoneGetModelObject(
                nanoemMutableModelBoneGetOriginObject(value.opaque),
            ))
        };
        status.value(index)
    }
    pub fn remove_bone(&self, value: MutableBone) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe { nanoemMutableModelRemoveBoneObject(self.opaque, value.opaque, status_ptr) }
        status.unit()
    }
    pub fn find_morph(&self, value: i32) -> Result<MutableMorph> {
        let mut num_objects = 0usize;
        let ptr = unsafe {
            nanoemModelGetAllMorphObjects(
                nanoemMutableModelGetOriginObject(self.opaque),
                &mut num_objects as *mut usize,
            )
        };
        let offset = value as usize;
        if offset < num_objects {
            Ok(
                unsafe {
                    MutableMorph::from_ptr(&self.unicode_string_factory, *(ptr.add(offset)))?
                },
            )
        } else {
            Err(Status::null_object())
        }
    }
    pub fn create_morph(&self) -> Result<MutableMorph> {
        MutableMorph::new(&self)
    }
    pub fn insert_morph(&self, value: MutableMorph, index: i32) -> Result<i32> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let index = unsafe {
            nanoemMutableModelInsertMorphObject(self.opaque, value.opaque, index, status_ptr);
            nanoemModelObjectGetIndex(nanoemModelMorphGetModelObject(
                nanoemMutableModelMorphGetOriginObject(value.opaque),
            ))
        };
        status.value(index)
    }
    pub fn remove_morph(&self, value: MutableMorph) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe { nanoemMutableModelRemoveMorphObject(self.opaque, value.opaque, status_ptr) }
        status.unit()
    }
    pub fn find_label(&self, value: i32) -> Result<MutableLabel> {
        let mut num_objects = 0usize;
        let ptr = unsafe {
            nanoemModelGetAllLabelObjects(
                nanoemMutableModelGetOriginObject(self.opaque),
                &mut num_objects as *mut usize,
            )
        };
        let offset = value as usize;
        if offset < num_objects {
            Ok(
                unsafe {
                    MutableLabel::from_ptr(&self.unicode_string_factory, *(ptr.add(offset)))?
                },
            )
        } else {
            Err(Status::null_object())
        }
    }
    pub fn create_label(&self) -> Result<MutableLabel> {
        MutableLabel::new(&self)
    }
    pub fn insert_label(&self, value: MutableLabel, index: i32) -> Result<i32> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let index = unsafe {
            nanoemMutableModelInsertLabelObject(self.opaque, value.opaque, index, status_ptr);
            nanoemModelObjectGetIndex(nanoemModelLabelGetModelObject(
                nanoemMutableModelLabelGetOriginObject(value.opaque),
            ))
        };
        status.value(index)
    }
    pub fn remove_label(&self, value: MutableLabel) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe { nanoemMutableModelRemoveLabelObject(self.opaque, value.opaque, status_ptr) }
        status.unit()
    }
    pub fn find_rigid_body(&self, value: i32) -> Result<MutableRigidBody> {
        let mut num_objects = 0usize;
        let ptr = unsafe {
            nanoemModelGetAllRigidBodyObjects(
                nanoemMutableModelGetOriginObject(self.opaque),
                &mut num_objects as *mut usize,
            )
        };
        let offset = value as usize;
        if offset < num_objects {
            Ok(unsafe {
                MutableRigidBody::from_ptr(&self.unicode_string_factory, *(ptr.add(offset)))?
            })
        } else {
            Err(Status::null_object())
        }
    }
    pub fn create_rigid_body(&self) -> Result<MutableRigidBody> {
        MutableRigidBody::new(&self)
    }
    pub fn insert_rigid_body(&self, value: MutableRigidBody, index: i32) -> Result<i32> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let index = unsafe {
            nanoemMutableModelInsertRigidBodyObject(self.opaque, value.opaque, index, status_ptr);
            nanoemModelObjectGetIndex(nanoemModelRigidBodyGetModelObject(
                nanoemMutableModelRigidBodyGetOriginObject(value.opaque),
            ))
        };
        status.value(index)
    }
    pub fn remove_rigid_body(&self, value: MutableRigidBody) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe { nanoemMutableModelRemoveRigidBodyObject(self.opaque, value.opaque, status_ptr) }
        status.unit()
    }
    pub fn find_joint(&self, value: i32) -> Result<MutableJoint> {
        let mut num_objects = 0usize;
        let ptr = unsafe {
            nanoemModelGetAllJointObjects(
                nanoemMutableModelGetOriginObject(self.opaque),
                &mut num_objects as *mut usize,
            )
        };
        let offset = value as usize;
        if offset < num_objects {
            Ok(
                unsafe {
                    MutableJoint::from_ptr(&self.unicode_string_factory, *(ptr.add(offset)))?
                },
            )
        } else {
            Err(Status::null_object())
        }
    }
    pub fn create_joint(&self) -> Result<MutableJoint> {
        MutableJoint::new(&self)
    }
    pub fn insert_joint(&self, value: MutableJoint, index: i32) -> Result<i32> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let index = unsafe {
            nanoemMutableModelInsertJointObject(self.opaque, value.opaque, index, status_ptr);
            nanoemModelObjectGetIndex(nanoemModelJointGetModelObject(
                nanoemMutableModelJointGetOriginObject(value.opaque),
            ))
        };
        status.value(index)
    }
    pub fn remove_joint(&self, value: MutableJoint) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe { nanoemMutableModelRemoveJointObject(self.opaque, value.opaque, status_ptr) }
        status.unit()
    }
    fn save_buffer(&self, buffer: &mut MutableBuffer) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableModelSaveToBuffer(self.opaque, buffer.opaque, status_ptr);
        };
        status.unit()
    }
}

impl<'a> Drop for MutableModel {
    fn drop(&mut self) {
        unsafe {
            nanoemMutableModelDestroy(self.opaque);
        }
    }
}

pub struct MutableVertex<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *mut nanoem_mutable_model_vertex_t,
}

impl<'a> MutableVertex<'a> {
    pub fn new(model: &'a MutableModel) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe {
            nanoemMutableModelVertexCreate(
                nanoemMutableModelGetOriginObject(model.opaque),
                status_ptr,
            )
        };
        status.closure(|| Self {
            unicode_string_factory: &model.unicode_string_factory,
            opaque,
        })
    }
    pub(crate) unsafe fn from_ptr(
        factory: &'a UnicodeStringFactory,
        opaque: *mut nanoem_model_vertex_t,
    ) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = nanoemMutableModelVertexCreateAsReference(opaque, status_ptr);
        status.closure(|| Self {
            unicode_string_factory: factory,
            opaque,
        })
    }
    pub fn as_origin(&self) -> Vertex {
        Vertex::new(&self.unicode_string_factory, unsafe {
            nanoemMutableModelVertexGetOriginObject(self.opaque)
        })
    }
    pub fn set_origin(&self, value: Vector4) {
        unsafe { nanoemMutableModelVertexSetOrigin(self.opaque, value.as_ptr()) }
    }
    pub fn set_normal(&self, value: Vector4) {
        unsafe { nanoemMutableModelVertexSetNormal(self.opaque, value.as_ptr()) }
    }
    pub fn set_texcoord(&self, value: Vector4) {
        unsafe { nanoemMutableModelVertexSetTexCoord(self.opaque, value.as_ptr()) }
    }
    pub fn set_additinal_uv(&self, value: Vector4, index: usize) {
        unsafe { nanoemMutableModelVertexSetAdditionalUV(self.opaque, value.as_ptr(), index) }
    }
    pub fn set_edge_size(&self, value: f32) {
        unsafe { nanoemMutableModelVertexSetEdgeSize(self.opaque, value) }
    }
    pub fn set_vertex_type(&self, value: VertexType) {
        unsafe { nanoemMutableModelVertexSetType(self.opaque, value.cast()) }
    }
    pub fn set_bone(&self, value: Option<&MutableBone>, offset: usize) {
        unsafe {
            nanoemMutableModelVertexSetBoneObject(
                self.opaque,
                match value {
                    Some(value) => nanoemMutableModelBoneGetOriginObject(value.opaque),
                    None => std::ptr::null(),
                },
                offset,
            )
        }
    }
    pub fn set_bone_weight(&self, value: f32, index: usize) {
        unsafe { nanoemMutableModelVertexSetBoneWeight(self.opaque, value, index) }
    }
    pub fn set_sdef_c(&self, value: Vector4) {
        unsafe { nanoemMutableModelVertexSetSdefC(self.opaque, value.as_ptr()) }
    }
    pub fn set_sdef_r0(&self, value: Vector4) {
        unsafe { nanoemMutableModelVertexSetSdefR0(self.opaque, value.as_ptr()) }
    }
    pub fn set_sdef_r1(&self, value: Vector4) {
        unsafe { nanoemMutableModelVertexSetSdefR1(self.opaque, value.as_ptr()) }
    }
}

impl<'a> Drop for MutableVertex<'a> {
    fn drop(&mut self) {
        unsafe { nanoemMutableModelVertexDestroy(self.opaque) }
    }
}

pub struct MutableTexture<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *mut nanoem_mutable_model_texture_t,
}

impl<'a> MutableTexture<'a> {
    pub fn new(model: &'a MutableModel) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe {
            nanoemMutableModelTextureCreate(
                nanoemMutableModelGetOriginObject(model.opaque),
                status_ptr,
            )
        };
        status.closure(|| Self {
            unicode_string_factory: &model.unicode_string_factory,
            opaque,
        })
    }
    pub(crate) unsafe fn from_ptr(
        factory: &'a UnicodeStringFactory,
        opaque: *mut nanoem_model_texture_t,
    ) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = nanoemMutableModelTextureCreateAsReference(opaque, status_ptr);
        status.closure(|| Self {
            unicode_string_factory: factory,
            opaque,
        })
    }
    pub fn set_path(&self, value: &str) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let name = self.unicode_string_factory.to_unicode_string(value)?;
        unsafe {
            nanoemMutableModelTextureSetPath(self.opaque, name.value, status_ptr);
        };
        status.unit()
    }
}

impl<'a> Drop for MutableTexture<'a> {
    fn drop(&mut self) {
        unsafe { nanoemMutableModelTextureDestroy(self.opaque) }
    }
}

pub struct MutableMaterial<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *mut nanoem_mutable_model_material_t,
}

impl<'a> MutableMaterial<'a> {
    pub fn new(model: &'a MutableModel) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe {
            nanoemMutableModelMaterialCreate(
                nanoemMutableModelGetOriginObject(model.opaque),
                status_ptr,
            )
        };
        status.closure(|| Self {
            unicode_string_factory: &model.unicode_string_factory,
            opaque,
        })
    }
    pub(crate) unsafe fn from_ptr(
        factory: &'a UnicodeStringFactory,
        opaque: *mut nanoem_model_material_t,
    ) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = nanoemMutableModelMaterialCreateAsReference(opaque, status_ptr);
        status.closure(|| Self {
            unicode_string_factory: factory,
            opaque,
        })
    }
    pub fn as_origin(&self) -> Material {
        Material::new(&self.unicode_string_factory, unsafe {
            nanoemMutableModelMaterialGetOriginObject(self.opaque)
        })
    }
    pub fn set_name(&self, value: &str, language: Language) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let name = self.unicode_string_factory.to_unicode_string(value)?;
        unsafe {
            nanoemMutableModelMaterialSetName(self.opaque, name.value, language.cast(), status_ptr);
        };
        status.unit()
    }
    pub fn set_diffuse_texture(&self, value: Option<&MutableTexture>) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableModelMaterialSetDiffuseTextureObject(
                self.opaque,
                match value {
                    Some(value) => value.opaque,
                    None => std::ptr::null_mut(),
                },
                status_ptr,
            )
        }
        status.unit()
    }
    pub fn set_sphere_map_texture(&self, value: Option<&MutableTexture>) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableModelMaterialSetSphereMapTextureObject(
                self.opaque,
                match value {
                    Some(value) => value.opaque,
                    None => std::ptr::null_mut(),
                },
                status_ptr,
            )
        }
        status.unit()
    }
    pub fn set_toon_texture(&self, value: Option<&MutableTexture>) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableModelMaterialSetToonTextureObject(
                self.opaque,
                match value {
                    Some(value) => value.opaque,
                    None => std::ptr::null_mut(),
                },
                status_ptr,
            )
        }
        status.unit()
    }
    pub fn set_sphere_map_texture_type(&self, value: MaterialSphereMapTextureType) {
        unsafe { nanoemMutableModelMaterialSetSphereMapTextureType(self.opaque, value.cast()) }
    }
    pub fn set_clob(&self, value: &str) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let name = self.unicode_string_factory.to_unicode_string(value)?;
        unsafe {
            nanoemMutableModelMaterialSetClob(self.opaque, name.value, status_ptr);
        };
        status.unit()
    }
    pub fn set_ambient_color(&self, value: Vector4) {
        unsafe { nanoemMutableModelMaterialSetAmbientColor(self.opaque, value.as_ptr()) }
    }
    pub fn set_diffuse_color(&self, value: Vector4) {
        unsafe { nanoemMutableModelMaterialSetDiffuseColor(self.opaque, value.as_ptr()) }
    }
    pub fn set_diffuse_opacity(&self, value: f32) {
        unsafe { nanoemMutableModelMaterialSetDiffuseOpacity(self.opaque, value) }
    }
    pub fn set_specular_color(&self, value: Vector4) {
        unsafe { nanoemMutableModelMaterialSetSpecularColor(self.opaque, value.as_ptr()) }
    }
    pub fn set_specular_power(&self, value: f32) {
        unsafe { nanoemMutableModelMaterialSetSpecularPower(self.opaque, value) }
    }
    pub fn set_edge_color(&self, value: Vector4) {
        unsafe { nanoemMutableModelMaterialSetEdgeColor(self.opaque, value.as_ptr()) }
    }
    pub fn set_edge_opacity(&self, value: f32) {
        unsafe { nanoemMutableModelMaterialSetEdgeOpacity(self.opaque, value) }
    }
    pub fn set_edge_size(&self, value: f32) {
        unsafe { nanoemMutableModelMaterialSetEdgeSize(self.opaque, value) }
    }
    pub fn set_num_vertex_indices(&self, value: u32) {
        unsafe { nanoemMutableModelMaterialSetNumVertexIndices(self.opaque, value as usize) }
    }
    pub fn set_toon_texture_index(&self, value: i32) {
        unsafe { nanoemMutableModelMaterialSetToonTextureIndex(self.opaque, value) }
    }
    pub fn set_toon_shared(&self, value: bool) {
        unsafe { nanoemMutableModelMaterialSetToonShared(self.opaque, value as i32) }
    }
    pub fn set_culling_disabled(&self, value: bool) {
        unsafe { nanoemMutableModelMaterialSetCullingDisabled(self.opaque, value as i32) }
    }
    pub fn set_casting_shadow_enabled(&self, value: bool) {
        unsafe { nanoemMutableModelMaterialSetCastingShadowEnabled(self.opaque, value as i32) }
    }
    pub fn set_casting_shadow_map_enabled(&self, value: bool) {
        unsafe { nanoemMutableModelMaterialSetCastingShadowMapEnabled(self.opaque, value as i32) }
    }
    pub fn set_shadow_map_enabled(&self, value: bool) {
        unsafe { nanoemMutableModelMaterialSetShadowMapEnabled(self.opaque, value as i32) }
    }
    pub fn set_edge_enabled(&self, value: bool) {
        unsafe { nanoemMutableModelMaterialSetEdgeEnabled(self.opaque, value as i32) }
    }
    pub fn set_vertex_color_enabled(&self, value: bool) {
        unsafe { nanoemMutableModelMaterialSetVertexColorEnabled(self.opaque, value as i32) }
    }
    pub fn set_point_draw_enabled(&self, value: bool) {
        unsafe { nanoemMutableModelMaterialSetPointDrawEnabled(self.opaque, value as i32) }
    }
    pub fn set_line_draw_enabled(&self, value: bool) {
        unsafe { nanoemMutableModelMaterialSetLineDrawEnabled(self.opaque, value as i32) }
    }
}

impl<'a> Drop for MutableMaterial<'a> {
    fn drop(&mut self) {
        unsafe { nanoemMutableModelMaterialDestroy(self.opaque) }
    }
}

pub struct MutableConstraintJoint<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *mut nanoem_mutable_model_constraint_joint_t,
}

impl<'a> MutableConstraintJoint<'a> {
    pub fn new(value: &'a MutableConstraint) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe { nanoemMutableModelConstraintJointCreate(value.opaque, status_ptr) };
        status.closure(|| Self {
            unicode_string_factory: value.unicode_string_factory,
            opaque,
        })
    }
    pub(crate) unsafe fn from_ptr(
        factory: &'a UnicodeStringFactory,
        opaque: *mut nanoem_model_constraint_joint_t,
    ) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = nanoemMutableModelConstraintJointCreateAsReference(opaque, status_ptr);
        status.closure(|| Self {
            unicode_string_factory: factory,
            opaque,
        })
    }
    pub fn as_origin(&self) -> ConstraintJoint {
        ConstraintJoint::new(&self.unicode_string_factory, unsafe {
            nanoemMutableModelConstraintJointGetOriginObject(self.opaque)
        })
    }
    pub fn set_bone(&self, value: Option<&MutableBone>) {
        unsafe {
            nanoemMutableModelConstraintJointSetBoneObject(
                self.opaque,
                match value {
                    Some(value) => nanoemMutableModelBoneGetOriginObject(value.opaque),
                    None => std::ptr::null_mut(),
                },
            )
        }
    }
    pub fn set_lower_limit(&self, value: Vector4) {
        unsafe { nanoemMutableModelConstraintJointSetLowerLimit(self.opaque, value.as_ptr()) }
    }
    pub fn set_upper_limit(&self, value: Vector4) {
        unsafe { nanoemMutableModelConstraintJointSetUpperLimit(self.opaque, value.as_ptr()) }
    }
}

impl<'a> Drop for MutableConstraintJoint<'a> {
    fn drop(&mut self) {
        unsafe { nanoemMutableModelConstraintJointDestroy(self.opaque) }
    }
}

pub struct MutableConstraint<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *mut nanoem_mutable_model_constraint_t,
}

impl<'a> MutableConstraint<'a> {
    pub fn new(model: &'a MutableModel) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe {
            nanoemMutableModelConstraintCreate(
                nanoemMutableModelGetOriginObject(model.opaque),
                status_ptr,
            )
        };
        status.closure(|| Self {
            unicode_string_factory: &model.unicode_string_factory,
            opaque,
        })
    }
    pub(crate) unsafe fn from_ptr(
        factory: &'a UnicodeStringFactory,
        opaque: *mut nanoem_model_constraint_t,
    ) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = nanoemMutableModelConstraintCreateAsReference(opaque, status_ptr);
        status.closure(|| Self {
            unicode_string_factory: factory,
            opaque,
        })
    }
    pub fn as_origin(&self) -> Constraint {
        Constraint::new(&self.unicode_string_factory, unsafe {
            nanoemMutableModelConstraintGetOriginObject(self.opaque)
        })
    }
    pub fn set_effector_bone(&self, value: Option<&MutableBone>) {
        unsafe {
            nanoemMutableModelConstraintSetEffectorBoneObject(
                self.opaque,
                match value {
                    Some(value) => nanoemMutableModelBoneGetOriginObject(value.opaque),
                    None => std::ptr::null(),
                },
            )
        }
    }
    pub fn set_target_bone(&self, value: Option<&MutableBone>) {
        unsafe {
            nanoemMutableModelConstraintSetTargetBoneObject(
                self.opaque,
                match value {
                    Some(value) => nanoemMutableModelBoneGetOriginObject(value.opaque),
                    None => std::ptr::null(),
                },
            )
        }
    }
    pub fn set_angle_limit(&self, value: f32) {
        unsafe { nanoemMutableModelConstraintSetAngleLimit(self.opaque, value) }
    }
    pub fn set_num_iterations(&self, value: i32) {
        unsafe { nanoemMutableModelConstraintSetNumIterations(self.opaque, value) }
    }
    pub fn find_joint(&self, value: i32) -> Result<MutableConstraintJoint> {
        let mut num_objects = 0usize;
        let ptr = unsafe {
            nanoemModelConstraintGetAllJointObjects(
                nanoemMutableModelConstraintGetOriginObject(self.opaque),
                &mut num_objects as *mut usize,
            )
        };
        let offset = value as usize;
        if offset < num_objects {
            Ok(unsafe {
                MutableConstraintJoint::from_ptr(&self.unicode_string_factory, *(ptr.add(offset)))?
            })
        } else {
            Err(Status::null_object())
        }
    }
    pub fn create_joint(&self) -> Result<MutableConstraintJoint> {
        MutableConstraintJoint::new(&self)
    }
    pub fn insert_joint(&self, value: MutableConstraintJoint, index: i32) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableModelConstraintInsertJointObject(
                self.opaque,
                value.opaque,
                index,
                status_ptr,
            )
        };
        status.unit()
    }
    pub fn remove_joint(&self, value: MutableConstraintJoint) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableModelConstraintRemoveJointObject(self.opaque, value.opaque, status_ptr)
        }
        status.unit()
    }
}

impl<'a> Drop for MutableConstraint<'a> {
    fn drop(&mut self) {
        unsafe { nanoemMutableModelConstraintDestroy(self.opaque) }
    }
}

pub struct MutableBone<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *mut nanoem_mutable_model_bone_t,
}

impl<'a> MutableBone<'a> {
    pub fn new(model: &'a MutableModel) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe {
            nanoemMutableModelBoneCreate(
                nanoemMutableModelGetOriginObject(model.opaque),
                status_ptr,
            )
        };
        status.closure(|| Self {
            unicode_string_factory: &model.unicode_string_factory,
            opaque,
        })
    }
    pub(crate) unsafe fn from_ptr(
        factory: &'a UnicodeStringFactory,
        opaque: *mut nanoem_model_bone_t,
    ) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = nanoemMutableModelBoneCreateAsReference(opaque, status_ptr);
        status.closure(|| Self {
            unicode_string_factory: factory,
            opaque,
        })
    }
    pub fn as_origin(&self) -> Bone {
        Bone::new(&self.unicode_string_factory, unsafe {
            nanoemMutableModelBoneGetOriginObject(self.opaque)
        })
    }
    pub fn set_name(&self, value: &str, language: Language) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let name = self.unicode_string_factory.to_unicode_string(value)?;
        unsafe {
            nanoemMutableModelBoneSetName(self.opaque, name.value, language.cast(), status_ptr);
        };
        status.unit()
    }
    pub fn set_parent_bone(&self, value: Option<&MutableBone>) {
        unsafe {
            nanoemMutableModelBoneSetParentBoneObject(
                self.opaque,
                match value {
                    Some(value) => nanoemMutableModelBoneGetOriginObject(value.opaque),
                    None => std::ptr::null(),
                },
            )
        }
    }
    pub fn set_inherent_parent_bone(&self, value: Option<&MutableBone>) {
        unsafe {
            nanoemMutableModelBoneSetInherentParentBoneObject(
                self.opaque,
                match value {
                    Some(value) => nanoemMutableModelBoneGetOriginObject(value.opaque),
                    None => std::ptr::null(),
                },
            )
        }
    }
    pub fn set_effector_bone(&self, value: Option<&MutableBone>) {
        unsafe {
            nanoemMutableModelBoneSetEffectorBoneObject(
                self.opaque,
                match value {
                    Some(value) => nanoemMutableModelBoneGetOriginObject(value.opaque),
                    None => std::ptr::null(),
                },
            )
        }
    }
    pub fn set_target_bone(&self, value: Option<&MutableBone>) {
        unsafe {
            nanoemMutableModelBoneSetTargetBoneObject(
                self.opaque,
                match value {
                    Some(value) => nanoemMutableModelBoneGetOriginObject(value.opaque),
                    None => std::ptr::null(),
                },
            )
        }
    }
    pub fn set_constraint(&self, value: Option<&MutableConstraint>) {
        unsafe {
            nanoemMutableModelBoneSetConstraintObject(
                self.opaque,
                match value {
                    Some(value) => value.opaque,
                    None => std::ptr::null_mut(),
                },
            )
        }
    }
    pub fn set_origin(&self, value: Vector4) {
        unsafe { nanoemMutableModelBoneSetOrigin(self.opaque, value.as_ptr()) }
    }
    pub fn set_destination_origin(&self, value: Vector4) {
        unsafe { nanoemMutableModelBoneSetDestinationOrigin(self.opaque, value.as_ptr()) }
    }
    pub fn set_fixed_axis(&self, value: Vector4) {
        unsafe { nanoemMutableModelBoneSetFixedAxis(self.opaque, value.as_ptr()) }
    }
    pub fn set_local_x_axis(&self, value: Vector4) {
        unsafe { nanoemMutableModelBoneSetLocalXAxis(self.opaque, value.as_ptr()) }
    }
    pub fn set_local_z_axis(&self, value: Vector4) {
        unsafe { nanoemMutableModelBoneSetLocalZAxis(self.opaque, value.as_ptr()) }
    }
    pub fn set_inherent_coefficient(&self, value: f32) {
        unsafe { nanoemMutableModelBoneSetInherentCoefficient(self.opaque, value) }
    }
    pub fn set_stage_index(&self, value: i32) {
        unsafe { nanoemMutableModelBoneSetStageIndex(self.opaque, value) }
    }
    pub fn set_rotateable(&self, value: bool) {
        unsafe { nanoemMutableModelBoneSetRotateable(self.opaque, value as i32) }
    }
    pub fn set_movable(&self, value: bool) {
        unsafe { nanoemMutableModelBoneSetMovable(self.opaque, value as i32) }
    }
    pub fn set_visible(&self, value: bool) {
        unsafe { nanoemMutableModelBoneSetVisible(self.opaque, value as i32) }
    }
    pub fn set_user_handleable(&self, value: bool) {
        unsafe { nanoemMutableModelBoneSetUserHandleable(self.opaque, value as i32) }
    }
    pub fn set_fixed_axis_enabled(&self, value: bool) {
        unsafe { nanoemMutableModelBoneSetFixedAxisEnabled(self.opaque, value as i32) }
    }
    pub fn set_local_axes_enabled(&self, value: bool) {
        unsafe { nanoemMutableModelBoneSetLocalAxesEnabled(self.opaque, value as i32) }
    }
    pub fn set_constraint_enabled(&self, value: bool) {
        unsafe { nanoemMutableModelBoneSetConstraintEnabled(self.opaque, value as i32) }
    }
    pub fn set_local_inherent_enabled(&self, value: bool) {
        unsafe { nanoemMutableModelBoneSetLocalInherentEnabled(self.opaque, value as i32) }
    }
    pub fn set_inherent_translation_enabled(&self, value: bool) {
        unsafe { nanoemMutableModelBoneSetInherentTranslationEnabled(self.opaque, value as i32) }
    }
    pub fn set_inherent_orientation_enabled(&self, value: bool) {
        unsafe { nanoemMutableModelBoneSetInherentOrientationEnabled(self.opaque, value as i32) }
    }
    pub fn set_affected_by_physics_simulation(&self, value: bool) {
        unsafe { nanoemMutableModelBoneSetAffectedByPhysicsSimulation(self.opaque, value as i32) }
    }
    pub fn enable_external_parent_bone(&self, value: i32) {
        unsafe { nanoemMutableModelBoneEnableExternalParentBone(self.opaque, value) }
    }
}

impl<'a> Drop for MutableBone<'a> {
    fn drop(&mut self) {
        unsafe { nanoemMutableModelBoneDestroy(self.opaque) }
    }
}

pub struct MutableMorphBone<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *mut nanoem_mutable_model_morph_bone_t,
}

impl<'a> MutableMorphBone<'a> {
    pub fn new(
        factory: &'a UnicodeStringFactory,
        opaque: *mut nanoem_mutable_model_morph_bone_t,
    ) -> Self {
        Self {
            unicode_string_factory: factory,
            opaque,
        }
    }
    pub(crate) unsafe fn from_ptr(
        factory: &'a UnicodeStringFactory,
        opaque: *mut nanoem_model_morph_bone_t,
    ) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = nanoemMutableModelMorphBoneCreateAsReference(opaque, status_ptr);
        status.closure(|| Self {
            unicode_string_factory: factory,
            opaque,
        })
    }
    pub fn as_origin(&self) -> MorphBone {
        MorphBone::new(&self.unicode_string_factory, unsafe {
            nanoemMutableModelMorphBoneGetOriginObject(self.opaque)
        })
    }
    pub fn set_bone(&self, value: Option<&MutableBone>) {
        unsafe {
            nanoemMutableModelMorphBoneSetBoneObject(
                self.opaque,
                match value {
                    Some(value) => nanoemMutableModelBoneGetOriginObject(value.opaque),
                    None => std::ptr::null_mut(),
                },
            )
        }
    }
    pub fn set_translation(&self, value: Vector4) {
        unsafe { nanoemMutableModelMorphBoneSetTranslation(self.opaque, value.as_ptr()) }
    }
    pub fn set_orientation(&self, value: Vector4) {
        unsafe { nanoemMutableModelMorphBoneSetOrientation(self.opaque, value.as_ptr()) }
    }
}

impl<'a> Drop for MutableMorphBone<'a> {
    fn drop(&mut self) {
        unsafe { nanoemMutableModelMorphBoneDestroy(self.opaque) }
    }
}

pub struct MutableMorphFlip<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *mut nanoem_mutable_model_morph_flip_t,
}

impl<'a> MutableMorphFlip<'a> {
    pub fn new(
        factory: &'a UnicodeStringFactory,
        opaque: *mut nanoem_mutable_model_morph_flip_t,
    ) -> Self {
        Self {
            unicode_string_factory: factory,
            opaque,
        }
    }
    pub(crate) unsafe fn from_ptr(
        factory: &'a UnicodeStringFactory,
        opaque: *mut nanoem_model_morph_flip_t,
    ) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = nanoemMutableModelMorphFlipCreateAsReference(opaque, status_ptr);
        status.closure(|| Self {
            unicode_string_factory: factory,
            opaque,
        })
    }
    pub fn as_origin(&self) -> MorphFlip {
        MorphFlip::new(&self.unicode_string_factory, unsafe {
            nanoemMutableModelMorphFlipGetOriginObject(self.opaque)
        })
    }
    pub fn set_morph(&self, value: Option<&MutableMorph>) {
        unsafe {
            nanoemMutableModelMorphFlipSetMorphObject(
                self.opaque,
                match value {
                    Some(value) => nanoemMutableModelMorphGetOriginObject(value.opaque),
                    None => std::ptr::null_mut(),
                },
            )
        }
    }
    pub fn set_weight(&self, value: f32) {
        unsafe { nanoemMutableModelMorphFlipSetWeight(self.opaque, value) }
    }
}

impl<'a> Drop for MutableMorphFlip<'a> {
    fn drop(&mut self) {
        unsafe { nanoemMutableModelMorphFlipDestroy(self.opaque) }
    }
}

pub struct MutableMorphGroup<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *mut nanoem_mutable_model_morph_group_t,
}

impl<'a> MutableMorphGroup<'a> {
    pub fn new(
        factory: &'a UnicodeStringFactory,
        opaque: *mut nanoem_mutable_model_morph_group_t,
    ) -> Self {
        Self {
            unicode_string_factory: factory,
            opaque,
        }
    }
    pub(crate) unsafe fn from_ptr(
        factory: &'a UnicodeStringFactory,
        opaque: *mut nanoem_model_morph_group_t,
    ) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = nanoemMutableModelMorphGroupCreateAsReference(opaque, status_ptr);
        status.closure(|| Self {
            unicode_string_factory: factory,
            opaque,
        })
    }
    pub fn as_origin(&self) -> MorphGroup {
        MorphGroup::new(&self.unicode_string_factory, unsafe {
            nanoemMutableModelMorphGroupGetOriginObject(self.opaque)
        })
    }
    pub fn set_morph(&self, value: Option<&MutableMorph>) {
        unsafe {
            nanoemMutableModelMorphGroupSetMorphObject(
                self.opaque,
                match value {
                    Some(value) => nanoemMutableModelMorphGetOriginObject(value.opaque),
                    None => std::ptr::null_mut(),
                },
            )
        }
    }
    pub fn set_weight(&self, value: f32) {
        unsafe { nanoemMutableModelMorphGroupSetWeight(self.opaque, value) }
    }
}

impl<'a> Drop for MutableMorphGroup<'a> {
    fn drop(&mut self) {
        unsafe { nanoemMutableModelMorphGroupDestroy(self.opaque) }
    }
}

pub struct MutableMorphImpulse<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *mut nanoem_mutable_model_morph_impulse_t,
}

impl<'a> MutableMorphImpulse<'a> {
    pub fn new(
        factory: &'a UnicodeStringFactory,
        opaque: *mut nanoem_mutable_model_morph_impulse_t,
    ) -> Self {
        Self {
            unicode_string_factory: factory,
            opaque,
        }
    }
    pub(crate) unsafe fn from_ptr(
        factory: &'a UnicodeStringFactory,
        opaque: *mut nanoem_model_morph_impulse_t,
    ) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = nanoemMutableModelMorphImpulseCreateAsReference(opaque, status_ptr);
        status.closure(|| Self {
            unicode_string_factory: factory,
            opaque,
        })
    }
    pub fn as_origin(&self) -> MorphImpulse {
        MorphImpulse::new(&self.unicode_string_factory, unsafe {
            nanoemMutableModelMorphImpulseGetOriginObject(self.opaque)
        })
    }
    pub fn set_rigid_body(&self, value: Option<&MutableRigidBody>) {
        unsafe {
            nanoemMutableModelMorphImpulseSetRigidBodyObject(
                self.opaque,
                match value {
                    Some(value) => nanoemMutableModelRigidBodyGetOriginObject(value.opaque),
                    None => std::ptr::null_mut(),
                },
            )
        }
    }
    pub fn set_torque(&self, value: Vector4) {
        unsafe { nanoemMutableModelMorphImpulseSetTorque(self.opaque, value.as_ptr()) }
    }
    pub fn set_velocity(&self, value: Vector4) {
        unsafe { nanoemMutableModelMorphImpulseSetVelocity(self.opaque, value.as_ptr()) }
    }
    pub fn set_local(&self, value: bool) {
        unsafe { nanoemMutableModelMorphImpulseSetLocal(self.opaque, value as i32) }
    }
}

impl<'a> Drop for MutableMorphImpulse<'a> {
    fn drop(&mut self) {
        unsafe { nanoemMutableModelMorphImpulseDestroy(self.opaque) }
    }
}

pub struct MutableMorphMaterial<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *mut nanoem_mutable_model_morph_material_t,
}

impl<'a> MutableMorphMaterial<'a> {
    pub fn new(
        factory: &'a UnicodeStringFactory,
        opaque: *mut nanoem_mutable_model_morph_material_t,
    ) -> Self {
        Self {
            unicode_string_factory: factory,
            opaque,
        }
    }
    pub(crate) unsafe fn from_ptr(
        factory: &'a UnicodeStringFactory,
        opaque: *mut nanoem_model_morph_material_t,
    ) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = nanoemMutableModelMorphMaterialCreateAsReference(opaque, status_ptr);
        status.closure(|| Self {
            unicode_string_factory: factory,
            opaque,
        })
    }
    pub fn as_origin(&self) -> MorphMaterial {
        MorphMaterial::new(&self.unicode_string_factory, unsafe {
            nanoemMutableModelMorphMaterialGetOriginObject(self.opaque)
        })
    }
    pub fn set_material(&self, value: Option<&MutableMaterial>) {
        unsafe {
            nanoemMutableModelMorphMaterialSetMaterialObject(
                self.opaque,
                match value {
                    Some(value) => nanoemMutableModelMaterialGetOriginObject(value.opaque),
                    None => std::ptr::null_mut(),
                },
            )
        }
    }
    pub fn set_diffuse_texture_blend(&self, value: Vector4) {
        unsafe {
            nanoemMutableModelMorphMaterialSetDiffuseTextureBlend(self.opaque, value.as_ptr())
        }
    }
    pub fn set_sphere_map_texture_blend(&self, value: Vector4) {
        unsafe {
            nanoemMutableModelMorphMaterialSetSphereMapTextureBlend(self.opaque, value.as_ptr())
        }
    }
    pub fn set_toon_texture_blend(&self, value: Vector4) {
        unsafe { nanoemMutableModelMorphMaterialSetToonTextureBlend(self.opaque, value.as_ptr()) }
    }
    pub fn set_ambient_color(&self, value: Vector4) {
        unsafe { nanoemMutableModelMorphMaterialSetAmbientColor(self.opaque, value.as_ptr()) }
    }
    pub fn set_diffuse_color(&self, value: Vector4) {
        unsafe { nanoemMutableModelMorphMaterialSetDiffuseColor(self.opaque, value.as_ptr()) }
    }
    pub fn set_diffuse_opacity(&self, value: f32) {
        unsafe { nanoemMutableModelMorphMaterialSetDiffuseOpacity(self.opaque, value) }
    }
    pub fn set_specular_color(&self, value: Vector4) {
        unsafe { nanoemMutableModelMorphMaterialSetSpecularColor(self.opaque, value.as_ptr()) }
    }
    pub fn set_specular_power(&self, value: f32) {
        unsafe { nanoemMutableModelMorphMaterialSetSpecularPower(self.opaque, value) }
    }
    pub fn set_edge_color(&self, value: Vector4) {
        unsafe { nanoemMutableModelMorphMaterialSetEdgeColor(self.opaque, value.as_ptr()) }
    }
    pub fn set_edge_opacity(&self, value: f32) {
        unsafe { nanoemMutableModelMorphMaterialSetEdgeOpacity(self.opaque, value) }
    }
    pub fn set_edge_size(&self, value: f32) {
        unsafe { nanoemMutableModelMorphMaterialSetEdgeSize(self.opaque, value) }
    }
    pub fn set_operation_type(&self, value: MorphMaterialOperationType) {
        unsafe { nanoemMutableModelMorphMaterialSetOperationType(self.opaque, value.cast()) }
    }
}

impl<'a> Drop for MutableMorphMaterial<'a> {
    fn drop(&mut self) {
        unsafe { nanoemMutableModelMorphMaterialDestroy(self.opaque) }
    }
}

pub struct MutableMorphUV<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *mut nanoem_mutable_model_morph_uv_t,
}

impl<'a> MutableMorphUV<'a> {
    pub fn new(
        factory: &'a UnicodeStringFactory,
        opaque: *mut nanoem_mutable_model_morph_uv_t,
    ) -> Self {
        Self {
            unicode_string_factory: factory,
            opaque,
        }
    }
    pub(crate) unsafe fn from_ptr(
        factory: &'a UnicodeStringFactory,
        opaque: *mut nanoem_model_morph_uv_t,
    ) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = nanoemMutableModelMorphUVCreateAsReference(opaque, status_ptr);
        status.closure(|| Self {
            unicode_string_factory: factory,
            opaque,
        })
    }
    pub fn as_origin(&self) -> MorphUV {
        MorphUV::new(&self.unicode_string_factory, unsafe {
            nanoemMutableModelMorphUVGetOriginObject(self.opaque)
        })
    }
    pub fn set_vertex(&self, value: Option<&MutableVertex>) {
        unsafe {
            nanoemMutableModelMorphUVSetVertexObject(
                self.opaque,
                match value {
                    Some(value) => nanoemMutableModelVertexGetOriginObject(value.opaque),
                    None => std::ptr::null_mut(),
                },
            )
        }
    }
    pub fn set_position(&self, value: Vector4) {
        unsafe { nanoemMutableModelMorphUVSetPosition(self.opaque, value.as_ptr()) }
    }
}

impl<'a> Drop for MutableMorphUV<'a> {
    fn drop(&mut self) {
        unsafe { nanoemMutableModelMorphUVDestroy(self.opaque) }
    }
}

pub struct MutableMorphVertex<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *mut nanoem_mutable_model_morph_vertex_t,
}

impl<'a> MutableMorphVertex<'a> {
    pub fn new(
        factory: &'a UnicodeStringFactory,
        opaque: *mut nanoem_mutable_model_morph_vertex_t,
    ) -> Self {
        Self {
            unicode_string_factory: factory,
            opaque,
        }
    }
    pub(crate) unsafe fn from_ptr(
        factory: &'a UnicodeStringFactory,
        opaque: *mut nanoem_model_morph_vertex_t,
    ) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = nanoemMutableModelMorphVertexCreateAsReference(opaque, status_ptr);
        status.closure(|| Self {
            unicode_string_factory: factory,
            opaque,
        })
    }
    pub fn as_origin(&self) -> MorphVertex {
        MorphVertex::new(&self.unicode_string_factory, unsafe {
            nanoemMutableModelMorphVertexGetOriginObject(self.opaque)
        })
    }
    pub fn set_vertex(&self, value: Option<&MutableVertex>) {
        unsafe {
            nanoemMutableModelMorphVertexSetVertexObject(
                self.opaque,
                match value {
                    Some(value) => nanoemMutableModelVertexGetOriginObject(value.opaque),
                    None => std::ptr::null_mut(),
                },
            )
        }
    }
    pub fn set_position(&self, value: Vector4) {
        unsafe { nanoemMutableModelMorphVertexSetPosition(self.opaque, value.as_ptr()) }
    }
}

impl<'a> Drop for MutableMorphVertex<'a> {
    fn drop(&mut self) {
        unsafe { nanoemMutableModelMorphVertexDestroy(self.opaque) }
    }
}

pub struct MutableMorph<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *mut nanoem_mutable_model_morph_t,
}

impl<'a> MutableMorph<'a> {
    pub fn new(model: &'a MutableModel) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe {
            nanoemMutableModelMorphCreate(
                nanoemMutableModelGetOriginObject(model.opaque),
                status_ptr,
            )
        };
        status.closure(|| Self {
            unicode_string_factory: &model.unicode_string_factory,
            opaque,
        })
    }
    pub(crate) unsafe fn from_ptr(
        factory: &'a UnicodeStringFactory,
        opaque: *mut nanoem_model_morph_t,
    ) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = nanoemMutableModelMorphCreateAsReference(opaque, status_ptr);
        status.closure(|| Self {
            unicode_string_factory: factory,
            opaque,
        })
    }
    pub fn as_origin(&self) -> Morph {
        Morph::new(&self.unicode_string_factory, unsafe {
            nanoemMutableModelMorphGetOriginObject(self.opaque)
        })
    }
    pub fn set_name(&self, value: &str, language: Language) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let name = self.unicode_string_factory.to_unicode_string(value)?;
        unsafe {
            nanoemMutableModelMorphSetName(self.opaque, name.value, language.cast(), status_ptr);
        };
        status.unit()
    }
    pub fn set_category(&self, value: MorphCategory) {
        unsafe { nanoemMutableModelMorphSetCategory(self.opaque, value.cast()) }
    }
    pub fn set_morph_type(&self, value: MorphType) {
        unsafe { nanoemMutableModelMorphSetType(self.opaque, value.cast()) }
    }
    pub fn find_bone_morph(&self, value: i32) -> Result<MutableMorphBone> {
        let mut num_objects = 0usize;
        let ptr = unsafe {
            nanoemModelMorphGetAllBoneMorphObjects(
                nanoemMutableModelMorphGetOriginObject(self.opaque),
                &mut num_objects as *mut usize,
            )
        };
        let offset = value as usize;
        if offset < num_objects {
            Ok(unsafe {
                MutableMorphBone::from_ptr(&self.unicode_string_factory, *(ptr.add(offset)))?
            })
        } else {
            Err(Status::null_object())
        }
    }
    pub fn create_bone_morph(&self) -> Result<MutableMorphBone> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe { nanoemMutableModelMorphBoneCreate(self.opaque, status_ptr) };
        status.closure(|| MutableMorphBone::new(&self.unicode_string_factory, opaque))
    }
    pub fn insert_bone_morph(&self, value: MutableMorphBone, index: i32) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableModelMorphInsertBoneMorphObject(
                self.opaque,
                value.opaque,
                index,
                status_ptr,
            )
        }
        status.unit()
    }
    pub fn remove_bone_morph(&self, value: MutableMorphBone) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableModelMorphRemoveBoneMorphObject(self.opaque, value.opaque, status_ptr)
        }
        status.unit()
    }
    pub fn find_flip_morph(&self, value: i32) -> Result<MutableMorphFlip> {
        let mut num_objects = 0usize;
        let ptr = unsafe {
            nanoemModelMorphGetAllFlipMorphObjects(
                nanoemMutableModelMorphGetOriginObject(self.opaque),
                &mut num_objects as *mut usize,
            )
        };
        let offset = value as usize;
        if offset < num_objects {
            Ok(unsafe {
                MutableMorphFlip::from_ptr(&self.unicode_string_factory, *(ptr.add(offset)))?
            })
        } else {
            Err(Status::null_object())
        }
    }
    pub fn create_flip_morph(&self) -> Result<MutableMorphFlip> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe { nanoemMutableModelMorphFlipCreate(self.opaque, status_ptr) };
        status.closure(|| MutableMorphFlip::new(&self.unicode_string_factory, opaque))
    }
    pub fn insert_flip_morph(&self, value: MutableMorphFlip, index: i32) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableModelMorphInsertFlipMorphObject(
                self.opaque,
                value.opaque,
                index,
                status_ptr,
            )
        }
        status.unit()
    }
    pub fn remove_flip_morph(&self, value: MutableMorphFlip) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableModelMorphRemoveFlipMorphObject(self.opaque, value.opaque, status_ptr)
        }
        status.unit()
    }
    pub fn find_group_morph(&self, value: i32) -> Result<MutableMorphGroup> {
        let mut num_objects = 0usize;
        let ptr = unsafe {
            nanoemModelMorphGetAllGroupMorphObjects(
                nanoemMutableModelMorphGetOriginObject(self.opaque),
                &mut num_objects as *mut usize,
            )
        };
        let offset = value as usize;
        if offset < num_objects {
            Ok(unsafe {
                MutableMorphGroup::from_ptr(&self.unicode_string_factory, *(ptr.add(offset)))?
            })
        } else {
            Err(Status::null_object())
        }
    }
    pub fn create_group_morph(&self) -> Result<MutableMorphGroup> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe { nanoemMutableModelMorphGroupCreate(self.opaque, status_ptr) };
        status.closure(|| MutableMorphGroup::new(&self.unicode_string_factory, opaque))
    }
    pub fn insert_group_morph(&self, value: MutableMorphGroup, index: i32) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableModelMorphInsertGroupMorphObject(
                self.opaque,
                value.opaque,
                index,
                status_ptr,
            )
        }
        status.unit()
    }
    pub fn remove_group_morph(&self, value: MutableMorphGroup) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableModelMorphRemoveGroupMorphObject(self.opaque, value.opaque, status_ptr)
        }
        status.unit()
    }
    pub fn find_impulse_morph(&self, value: i32) -> Result<MutableMorphImpulse> {
        let mut num_objects = 0usize;
        let ptr = unsafe {
            nanoemModelMorphGetAllImpulseMorphObjects(
                nanoemMutableModelMorphGetOriginObject(self.opaque),
                &mut num_objects as *mut usize,
            )
        };
        let offset = value as usize;
        if offset < num_objects {
            Ok(unsafe {
                MutableMorphImpulse::from_ptr(&self.unicode_string_factory, *(ptr.add(offset)))?
            })
        } else {
            Err(Status::null_object())
        }
    }
    pub fn create_impulse_morph(&self) -> Result<MutableMorphImpulse> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe { nanoemMutableModelMorphImpulseCreate(self.opaque, status_ptr) };
        status.closure(|| MutableMorphImpulse::new(&self.unicode_string_factory, opaque))
    }
    pub fn insert_impulse_morph(&self, value: MutableMorphImpulse, index: i32) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableModelMorphInsertImpulseMorphObject(
                self.opaque,
                value.opaque,
                index,
                status_ptr,
            )
        }
        status.unit()
    }
    pub fn remove_impulse_morph(&self, value: MutableMorphImpulse) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableModelMorphRemoveImpulseMorphObject(self.opaque, value.opaque, status_ptr)
        }
        status.unit()
    }
    pub fn find_material_morph(&self, value: i32) -> Result<MutableMorphMaterial> {
        let mut num_objects = 0usize;
        let ptr = unsafe {
            nanoemModelMorphGetAllMaterialMorphObjects(
                nanoemMutableModelMorphGetOriginObject(self.opaque),
                &mut num_objects as *mut usize,
            )
        };
        let offset = value as usize;
        if offset < num_objects {
            Ok(unsafe {
                MutableMorphMaterial::from_ptr(&self.unicode_string_factory, *(ptr.add(offset)))?
            })
        } else {
            Err(Status::null_object())
        }
    }
    pub fn create_material_morph(&self) -> Result<MutableMorphMaterial> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe { nanoemMutableModelMorphMaterialCreate(self.opaque, status_ptr) };
        status.closure(|| MutableMorphMaterial::new(&self.unicode_string_factory, opaque))
    }
    pub fn insert_material_morph(&self, value: MutableMorphMaterial, index: i32) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableModelMorphInsertMaterialMorphObject(
                self.opaque,
                value.opaque,
                index,
                status_ptr,
            )
        }
        status.unit()
    }
    pub fn remove_material_morph(&self, value: MutableMorphMaterial) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableModelMorphRemoveMaterialMorphObject(self.opaque, value.opaque, status_ptr)
        }
        status.unit()
    }
    pub fn find_uv_morph(&self, value: i32) -> Result<MutableMorphUV> {
        let mut num_objects = 0usize;
        let ptr = unsafe {
            nanoemModelMorphGetAllUVMorphObjects(
                nanoemMutableModelMorphGetOriginObject(self.opaque),
                &mut num_objects as *mut usize,
            )
        };
        let offset = value as usize;
        if offset < num_objects {
            Ok(unsafe {
                MutableMorphUV::from_ptr(&self.unicode_string_factory, *(ptr.add(offset)))?
            })
        } else {
            Err(Status::null_object())
        }
    }
    pub fn create_uv_morph(&self) -> Result<MutableMorphUV> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe { nanoemMutableModelMorphUVCreate(self.opaque, status_ptr) };
        status.closure(|| MutableMorphUV::new(&self.unicode_string_factory, opaque))
    }
    pub fn insert_uv_morph(&self, value: MutableMorphUV, index: i32) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableModelMorphInsertUVMorphObject(self.opaque, value.opaque, index, status_ptr)
        }
        status.unit()
    }
    pub fn remove_uv_morph(&self, value: MutableMorphUV) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe { nanoemMutableModelMorphRemoveUVMorphObject(self.opaque, value.opaque, status_ptr) }
        status.unit()
    }
    pub fn find_vertex_morph(&self, value: i32) -> Result<MutableMorphVertex> {
        let mut num_objects = 0usize;
        let ptr = unsafe {
            nanoemModelMorphGetAllVertexMorphObjects(
                nanoemMutableModelMorphGetOriginObject(self.opaque),
                &mut num_objects as *mut usize,
            )
        };
        let offset = value as usize;
        if offset < num_objects {
            Ok(unsafe {
                MutableMorphVertex::from_ptr(&self.unicode_string_factory, *(ptr.add(offset)))?
            })
        } else {
            Err(Status::null_object())
        }
    }
    pub fn create_vertex_morph(&self) -> Result<MutableMorphVertex> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe { nanoemMutableModelMorphVertexCreate(self.opaque, status_ptr) };
        status.closure(|| MutableMorphVertex::new(&self.unicode_string_factory, opaque))
    }
    pub fn insert_vertex_morph(&self, value: MutableMorphVertex, index: i32) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableModelMorphInsertVertexMorphObject(
                self.opaque,
                value.opaque,
                index,
                status_ptr,
            )
        }
        status.unit()
    }
    pub fn remove_vertex_morph(&self, value: MutableMorphVertex) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableModelMorphRemoveVertexMorphObject(self.opaque, value.opaque, status_ptr)
        }
        status.unit()
    }
}

impl<'a> Drop for MutableMorph<'a> {
    fn drop(&mut self) {
        unsafe { nanoemMutableModelMorphDestroy(self.opaque) }
    }
}

pub struct MutableLabelItem<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *mut nanoem_mutable_model_label_item_t,
}

impl<'a> MutableLabelItem<'a> {
    pub fn from_bone(parent: &'a MutableLabel, value: MutableBone) -> Result<MutableLabelItem<'a>> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe {
            nanoemMutableModelLabelItemCreateFromBoneObject(
                parent.opaque,
                nanoemMutableModelBoneGetOriginObject(value.opaque),
                status_ptr,
            )
        };
        status.closure(|| MutableLabelItem {
            unicode_string_factory: parent.unicode_string_factory,
            opaque,
        })
    }
    pub fn from_morph(
        parent: &'a MutableLabel,
        value: MutableMorph,
    ) -> Result<MutableLabelItem<'a>> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe {
            nanoemMutableModelLabelItemCreateFromMorphObject(
                parent.opaque,
                nanoemMutableModelMorphGetOriginObject(value.opaque),
                status_ptr,
            )
        };
        status.closure(|| MutableLabelItem {
            unicode_string_factory: parent.unicode_string_factory,
            opaque,
        })
    }
    pub(crate) unsafe fn from_ptr(
        factory: &'a UnicodeStringFactory,
        opaque: *mut nanoem_model_label_item_t,
    ) -> Result<MutableLabelItem<'a>> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = nanoemMutableModelLabelItemCreateAsReference(opaque, status_ptr);
        status.closure(|| MutableLabelItem {
            unicode_string_factory: factory,
            opaque,
        })
    }
    pub fn as_origin(&self) -> LabelItem {
        LabelItem::new(&self.unicode_string_factory, unsafe {
            nanoemMutableModelLabelItemGetOriginObject(self.opaque)
        })
    }
}

impl<'a> Drop for MutableLabelItem<'a> {
    fn drop(&mut self) {
        unsafe { nanoemMutableModelLabelItemDestroy(self.opaque) }
    }
}

pub struct MutableLabel<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *mut nanoem_mutable_model_label_t,
}

impl<'a> MutableLabel<'a> {
    pub fn new(model: &'a MutableModel) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe {
            nanoemMutableModelLabelCreate(
                nanoemMutableModelGetOriginObject(model.opaque),
                status_ptr,
            )
        };
        status.closure(|| Self {
            unicode_string_factory: &model.unicode_string_factory,
            opaque,
        })
    }
    pub(crate) unsafe fn from_ptr(
        factory: &'a UnicodeStringFactory,
        opaque: *mut nanoem_model_label_t,
    ) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = nanoemMutableModelLabelCreateAsReference(opaque, status_ptr);
        status.closure(|| Self {
            unicode_string_factory: factory,
            opaque,
        })
    }
    pub fn as_origin(&self) -> Label {
        Label::new(&self.unicode_string_factory, unsafe {
            nanoemMutableModelLabelGetOriginObject(self.opaque)
        })
    }
    pub fn set_name(&self, value: &str, language: Language) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let name = self.unicode_string_factory.to_unicode_string(value)?;
        unsafe {
            nanoemMutableModelLabelSetName(self.opaque, name.value, language.cast(), status_ptr);
        };
        status.unit()
    }
    pub fn set_special(&self, value: bool) {
        unsafe { nanoemMutableModelLabelSetSpecial(self.opaque, value as i32) }
    }
    pub fn find_item(&self, value: i32) -> Result<MutableLabelItem> {
        let mut num_objects = 0usize;
        let ptr = unsafe {
            nanoemModelLabelGetAllItemObjects(
                nanoemMutableModelLabelGetOriginObject(self.opaque),
                &mut num_objects as *mut usize,
            )
        };
        let offset = value as usize;
        if offset < num_objects {
            Ok(unsafe {
                MutableLabelItem::from_ptr(&self.unicode_string_factory, *(ptr.add(offset)))?
            })
        } else {
            Err(Status::null_object())
        }
    }
    pub fn create_item_from_bone(&self, value: MutableBone) -> Result<MutableLabelItem> {
        MutableLabelItem::from_bone(self, value)
    }
    pub fn create_item_from_morph(&self, value: MutableMorph) -> Result<MutableLabelItem> {
        MutableLabelItem::from_morph(self, value)
    }
    pub fn insert_item(&self, value: MutableLabelItem, index: i32) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe {
            nanoemMutableModelLabelInsertItemObject(self.opaque, value.opaque, index, status_ptr)
        }
        status.unit()
    }
    pub fn remove_item(&self, value: MutableLabelItem) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        unsafe { nanoemMutableModelLabelRemoveItemObject(self.opaque, value.opaque, status_ptr) }
        status.unit()
    }
}

impl<'a> Drop for MutableLabel<'a> {
    fn drop(&mut self) {
        unsafe { nanoemMutableModelLabelDestroy(self.opaque) }
    }
}

pub struct MutableRigidBody<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *mut nanoem_mutable_model_rigid_body_t,
}

impl<'a> MutableRigidBody<'a> {
    pub fn new(model: &'a MutableModel) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe {
            nanoemMutableModelRigidBodyCreate(
                nanoemMutableModelGetOriginObject(model.opaque),
                status_ptr,
            )
        };
        status.closure(|| Self {
            unicode_string_factory: &model.unicode_string_factory,
            opaque,
        })
    }
    pub(crate) unsafe fn from_ptr(
        factory: &'a UnicodeStringFactory,
        opaque: *mut nanoem_model_rigid_body_t,
    ) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = nanoemMutableModelRigidBodyCreateAsReference(opaque, status_ptr);
        status.closure(|| Self {
            unicode_string_factory: factory,
            opaque,
        })
    }
    pub fn as_origin(&self) -> RigidBody {
        RigidBody::new(&self.unicode_string_factory, unsafe {
            nanoemMutableModelRigidBodyGetOriginObject(self.opaque)
        })
    }
    pub fn set_name(&self, value: &str, language: Language) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let name = self.unicode_string_factory.to_unicode_string(value)?;
        unsafe {
            nanoemMutableModelRigidBodySetName(
                self.opaque,
                name.value,
                language.cast(),
                status_ptr,
            );
        };
        status.unit()
    }
    pub fn set_bone(&self, value: Option<&MutableBone>) {
        unsafe {
            nanoemMutableModelRigidBodySetBoneObject(
                self.opaque,
                match value {
                    Some(value) => nanoemMutableModelBoneGetOriginObject(value.opaque),
                    None => std::ptr::null(),
                },
            )
        }
    }
    pub fn set_origin(&self, value: Vector4) {
        unsafe { nanoemMutableModelRigidBodySetOrigin(self.opaque, value.as_ptr()) }
    }
    pub fn set_orientation(&self, value: Vector4) {
        unsafe { nanoemMutableModelRigidBodySetOrientation(self.opaque, value.as_ptr()) }
    }
    pub fn set_shape_size(&self, value: Vector4) {
        unsafe { nanoemMutableModelRigidBodySetShapeSize(self.opaque, value.as_ptr()) }
    }
    pub fn set_transform_type(&self, value: RigidBodyTransformType) {
        unsafe { nanoemMutableModelRigidBodySetTransformType(self.opaque, value.cast()) }
    }
    pub fn set_shape_type(&self, value: RigidBodyShapeType) {
        unsafe { nanoemMutableModelRigidBodySetShapeType(self.opaque, value.cast()) }
    }
    pub fn set_mass(&self, value: f32) {
        unsafe { nanoemMutableModelRigidBodySetMass(self.opaque, value) }
    }
    pub fn set_linear_damping(&self, value: f32) {
        unsafe { nanoemMutableModelRigidBodySetLinearDamping(self.opaque, value) }
    }
    pub fn set_angular_damping(&self, value: f32) {
        unsafe { nanoemMutableModelRigidBodySetAngularDamping(self.opaque, value) }
    }
    pub fn set_friction(&self, value: f32) {
        unsafe { nanoemMutableModelRigidBodySetFriction(self.opaque, value) }
    }
    pub fn set_restitution(&self, value: f32) {
        unsafe { nanoemMutableModelRigidBodySetRestitution(self.opaque, value) }
    }
    pub fn set_collision_group_id(&self, value: i32) {
        unsafe { nanoemMutableModelRigidBodySetCollisionGroupId(self.opaque, value) }
    }
    pub fn set_collision_mask(&self, value: i32) {
        unsafe { nanoemMutableModelRigidBodySetCollisionMask(self.opaque, value) }
    }
}

impl<'a> Drop for MutableRigidBody<'a> {
    fn drop(&mut self) {
        unsafe { nanoemMutableModelRigidBodyDestroy(self.opaque) }
    }
}

pub struct MutableJoint<'a> {
    unicode_string_factory: &'a UnicodeStringFactory,
    opaque: *mut nanoem_mutable_model_joint_t,
}

impl<'a> MutableJoint<'a> {
    pub fn new(model: &'a MutableModel) -> Result<Self> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = unsafe {
            nanoemMutableModelJointCreate(
                nanoemMutableModelGetOriginObject(model.opaque),
                status_ptr,
            )
        };
        status.closure(|| Self {
            unicode_string_factory: &model.unicode_string_factory,
            opaque,
        })
    }
    pub(crate) unsafe fn from_ptr(
        factory: &'a UnicodeStringFactory,
        opaque: *mut nanoem_model_joint_t,
    ) -> Result<MutableJoint<'a>> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let opaque = nanoemMutableModelJointCreateAsReference(opaque, status_ptr);
        status.closure(|| MutableJoint {
            unicode_string_factory: factory,
            opaque,
        })
    }
    pub fn as_origin(&self) -> Joint {
        Joint::new(&self.unicode_string_factory, unsafe {
            nanoemMutableModelJointGetOriginObject(self.opaque)
        })
    }
    pub fn set_name(&self, value: &str, language: Language) -> Result<()> {
        let mut status = nanoem_status_t::SUCCESS;
        let status_ptr = &mut status as *mut nanoem_status_t;
        let name = self.unicode_string_factory.to_unicode_string(value)?;
        unsafe {
            nanoemMutableModelJointSetName(self.opaque, name.value, language.cast(), status_ptr);
        };
        status.unit()
    }
    pub fn set_rigid_body_a(&self, value: Option<&MutableRigidBody>) {
        unsafe {
            nanoemMutableModelJointSetRigidBodyAObject(
                self.opaque,
                match value {
                    Some(value) => nanoemMutableModelRigidBodyGetOriginObject(value.opaque),
                    None => std::ptr::null(),
                },
            )
        }
    }
    pub fn set_rigid_body_b(&self, value: Option<&MutableRigidBody>) {
        unsafe {
            nanoemMutableModelJointSetRigidBodyBObject(
                self.opaque,
                match value {
                    Some(value) => nanoemMutableModelRigidBodyGetOriginObject(value.opaque),
                    None => std::ptr::null(),
                },
            )
        }
    }
    pub fn set_joint_type(&self, value: JointType) {
        unsafe { nanoemMutableModelJointSetType(self.opaque, value.cast()) }
    }
    pub fn set_origin(&self, value: Vector4) {
        unsafe { nanoemMutableModelJointSetOrigin(self.opaque, value.as_ptr()) }
    }
    pub fn set_orientation(&self, value: Vector4) {
        unsafe { nanoemMutableModelJointSetOrientation(self.opaque, value.as_ptr()) }
    }
    pub fn set_angular_lower_limit(&self, value: Vector4) {
        unsafe { nanoemMutableModelJointSetAngularLowerLimit(self.opaque, value.as_ptr()) }
    }
    pub fn set_angular_upper_limit(&self, value: Vector4) {
        unsafe { nanoemMutableModelJointSetAngularUpperLimit(self.opaque, value.as_ptr()) }
    }
    pub fn set_angular_stiffness(&self, value: Vector4) {
        unsafe { nanoemMutableModelJointSetAngularStiffness(self.opaque, value.as_ptr()) }
    }
    pub fn set_linear_lower_limit(&self, value: Vector4) {
        unsafe { nanoemMutableModelJointSetLinearLowerLimit(self.opaque, value.as_ptr()) }
    }
    pub fn set_linear_upper_limit(&self, value: Vector4) {
        unsafe { nanoemMutableModelJointSetLinearUpperLimit(self.opaque, value.as_ptr()) }
    }
    pub fn set_linear_stiffness(&self, value: Vector4) {
        unsafe { nanoemMutableModelJointSetLinearStiffness(self.opaque, value.as_ptr()) }
    }
}

impl<'a> Drop for MutableJoint<'a> {
    fn drop(&mut self) {
        unsafe { nanoemMutableModelJointDestroy(self.opaque) }
    }
}
